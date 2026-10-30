"""Version-gated native GUI patch prototype; never edits installed game files.

The DLL runtime derives the overridden GUI from the user's own base
file before the already-verified early VFS mount. This tool previews that patch
and its native widget/data bindings.
"""
import argparse
import hashlib
from pathlib import Path
import re

BASE_SHA256 = '1f682222b689666731e755bc52b412b3126f4afff40cde05b13a0757b027ec3e'
STATE_SHA256 = 'ee259534f21785d962672ba25ebb66e6f3c881ffc2f74fdc44caaf989ab76be6'
TAB = 'ironroad_logistics'
TAB_OVERRIDE = '''
                blockoverride "fifth_button" { text = "IRONROAD_LOGISTICS" }
                blockoverride "fifth_button_selected" { text = "IRONROAD_LOGISTICS" }
                blockoverride "fifth_button_tooltip" { tooltip = "IRONROAD_LOGISTICS" }
                blockoverride "fifth_button_click" {
                    onclick = "[InformationPanel.SelectTab('ironroad_logistics')]"
                }
                blockoverride "fifth_button_visibility" {
                    visible = "[InformationPanel.IsTabSelected('ironroad_logistics')]"
                }
                blockoverride "fifth_button_visibility_checked" {
                    visible = "[Not(InformationPanel.IsTabSelected('ironroad_logistics'))]"
                }
'''


def braces(text):
    """Match structural braces while preserving strings, escapes and # comments."""
    pairs, stack = {}, []
    quote = comment = escaped = False
    for i, c in enumerate(text):
        if comment:
            if c == '\n':
                comment = False
        elif quote:
            if escaped:
                escaped = False
            elif c == '\\':
                escaped = True
            elif c == '"':
                quote = False
        elif c == '#':
            comment = True
        elif c == '"':
            quote = True
        elif c == '{':
            stack.append(i)
        elif c == '}':
            if not stack:
                raise ValueError('unbalanced GUI closing brace')
            pairs[stack.pop()] = i
    if stack or quote:
        raise ValueError('unbalanced GUI block/string')
    return pairs


def block(text, pattern, start=0, end=None):
    matches = list(re.finditer(pattern, text[start:end]))
    if len(matches) != 1:
        raise ValueError(f'expected one GUI anchor, got {len(matches)}: {pattern}')
    opening = start + matches[0].end() - 1
    return opening, braces(text)[opening]


def patch_market(raw):
    if hashlib.sha256(raw).hexdigest() != BASE_SHA256:
        raise ValueError('market_panel.gui differs from verified 1.13.11; native UI patch disabled')
    text = raw.decode('utf-8-sig')
    top, top_end = block(text, r'blockoverride\s+"fixed_top"\s*\{')
    _, tabs_end = block(text, r'\btab_buttons\s*=\s*\{', top, top_end)
    text = text[:tabs_end] + TAB_OVERRIDE + text[tabs_end:]
    old = "[Or(InformationPanel.IsTabSelected('default'), InformationPanel.IsTabSelected('world_market_trade'))]"
    if text.count(old) != 1:
        raise ValueError('ambiguous native market filter visibility')
    text = text.replace(old, f"[Or(InformationPanel.IsTabSelected('{TAB}'), {old[1:-1]})]")
    anchor = '\t\t\t\tmarket_panel_details_content = {'
    if text.count(anchor) != 1:
        raise ValueError('ambiguous native market scroll content')
    addition = ('                container = {\n'
                "                    visible = \"[InformationPanel.IsTabSelected('ironroad_logistics')]\"\n"
                '                    using = default_fade_in_out\n'
                '                    ironroad_market_logistics_content = {}\n'
                '                }\n')
    text = text.replace(anchor, addition + anchor)
    # The logistics tab keeps only the balance column of the row's trailing
    # columns: the two dependence columns remain in the vanilla tabs but are
    # hidden while our tab is selected, which also gives our two extra columns
    # the width they need.
    for key in ('MARKET_GOODS_BALANCE_TOOLTIP', 'MARKET_GOODS_IMPORT_DEPENDENCE', 'MARKET_GOODS_EXPORT_DEPENDENCE'):
        pos = text.find('tooltip = "' + key + '"')
        if pos < 0:
            raise ValueError('missing native dependence column: ' + key)
        container = text.rfind('\n\t\t\t\tflowcontainer = {', 0, pos)
        if container < 0:
            raise ValueError('ambiguous native dependence container: ' + key)
        insert_at = text.index('{', container) + 1
        text = (text[:insert_at] + '\n\t\t\t\t\t' +
                "visible = \"[Not(InformationPanel.IsTabSelected('ironroad_logistics'))]\"" +
                text[insert_at:])
    braces(text)
    return text


def patch_logistics_fragment(raw, fragment):
    if hashlib.sha256(raw).hexdigest() != BASE_SHA256:
        raise ValueError('market_panel.gui differs from verified 1.13.11; native UI patch disabled')
    original = raw.decode('utf-8-sig')
    start, end = block(original, r'type\s+goods_entry_button\s*=\s*flowcontainer\s*\{')
    price, price_end = block(original, r'block\s+"last_column"\s*\{', start, end)
    marker = '__IRONROAD_NATIVE_MARKET_PRICE__'
    if fragment.count(marker) != 1:
        raise ValueError('missing/ambiguous original market price placeholder')
    result = fragment.replace(marker, original[price+1:price_end])
    braces(result)
    return result


def patch_state(raw):
    if hashlib.sha256(raw).hexdigest() != STATE_SHA256:
        raise ValueError('states_panel.gui differs from verified 1.13.11; native state UI patch disabled')
    text = raw.decode('utf-8-sig')
    marker = 'visible =  "[InformationPanel.IsTabSelected(\'local_goods\')]"'
    if text.count(marker) != 1:
        raise ValueError('ambiguous state local goods tab')
    start = text.rfind('flowcontainer = {', 0, text.index(marker))
    opening = text.index('{', start)
    pairs = braces(text)
    end = pairs[opening]
    section = text[start:end+1]
    header_start = section.index('flowcontainer = {', section.index(marker))
    header_open = section.index('{', header_start)
    header_end = braces(section)[header_open]
    model = section.index('datamodel = "[StatesPanel.AccessLocalGoods]"')
    row_start = section.index('button = {', model)
    row_open = section.index('{', row_start)
    row_end = braces(section)[row_open]
    row = section[row_start:row_end+1]
    content = row.index('flowcontainer = {')
    content_end = braces(row)[row.index('{', content)]
    row = row[:content_end] + 'ironroad_state_freight_cell = {}\n' + row[content_end:]
    # Include the inherited 55px goods icon. Bound price widgets instead of
    # auto-sized containers; their minimum sizes used to push Freight offscreen.
    # 55 + 50 + 50 + 65 + 90 + 90 + 80 + 60 = the native 540px row.
    for old, new in [('60 23', '50 23'), ('90 52', '65 50'), ('70 40', '80 40'),
                     ('margin_right = 3', 'margin_right = 0'),
                     ('margin_left = 3', 'margin_left = 0')]:
        row = row.replace(old, new)
    balance_at = row.index('tooltip = "STATE_GOODS_BALANCE_TOOLTIP"')
    balance_start = row.rfind('flowcontainer = {', 0, balance_at)
    balance_end = braces(row)[row.index('{', balance_start)]
    balance = row[balance_start:balance_end+1]
    for old, new in [('autoresize = yes', 'size = { 65 23 }'),
                     ('margin_right = 15', 'margin_right = 0'),
                     ('margin_left = 15', 'margin_left = 0'),
                     ('align = right|nobaseline', 'align = hcenter|nobaseline'),
                     ('align = left|nobaseline', 'align = hcenter|nobaseline')]:
        balance = balance.replace(old, new)
    row = row[:balance_start] + balance + row[balance_end+1:]
    for anchor in ('tooltip = "[Goods.GetStatePriceDesc]"', 'using = market_price_tooltip_with_graph'):
        price_start = row.rfind('\tcontainer = {', 0, row.index(anchor)) + 1
        price_end = braces(row)[row.index('{', price_start)]
        price = row[price_start:price_end+1].replace('container = {', 'widget = {', 1)
        price = price.replace('minimumsize = { 100 -1 }', 'size = { 90 50 }')
        price = price.replace('autoresize = yes', 'size = { 90 40 }\nusing = elide_fontsize_min')
        row = row[:price_start] + price + row[price_end+1:]
    diff_start = row.rfind('widget = {', 0, row.index('text = "[Goods.GetStateToMarketPriceDiff|%0=-]"'))
    diff_end = braces(row)[row.index('{', diff_start)]
    diff = row[diff_start:diff_end+1].replace('align = right|nobaseline', 'align = center')
    diff = diff.replace('margin_right = 10', 'margin_right = 0')
    row = row[:diff_start] + diff + row[diff_end+1:]
    header = section[header_start:header_end]
    widths = iter((55, 50, 50, 65, 90, 90, 80))
    header = re.sub(r'size = \{ (55|60|90|100|80) 20 \}',
                    lambda m: f'size = {{ {next(widths)} 20 }}', header)
    header = header.replace('margin_right = 7', '')
    header = header.replace('sort_button = {', 'sort_button = {\nmargin_left = 0\nmargin_right = 0')
    header = header.replace('text = "concept_local_price"', 'text = "IRONROAD_LOCAL_PRICE"\ntooltip = "concept_local_price"')
    header = header.replace('text = "concept_market_price"', 'text = "IRONROAD_MARKET_PRICE"')
    header = header.replace('text = "PRICE_DIFFERENCE"', 'text = "IRONROAD_PRICE_DIFFERENCE"')
    header += ('sort_button = { align = hcenter|nobaseline text = "IRONROAD_FREIGHT_COLUMN" '
               'size = { 60 20 } margin_left = 0 margin_right = 0 tooltip = "IRONROAD_STATE_WORK_TOOLTIP" }\n')
    section = section[:row_start] + row + section[row_end+1:]
    section = section[:header_start] + header + section[header_end:]
    section = section[:header_start] + 'ironroad_state_filters = {}\n' + section[header_start:]
    text = text[:start] + section + text[end+1:]
    braces(text)
    return text


def localization_files():
    files = {}
    for language, translations in {
        'english': ['Logistics', 'Show', 'Hide', 'Goods', 'Trade quantity: --', 'Laid track: --', 'Waiting for original trade data.'],
        'simp_chinese': ['物流', '显示', '隐藏', '货物', '贸易量：--', '已铺铁路：--', '正在等待原版贸易数据。'],
    }.items():
        keys = ['LOGISTICS','SHOW','HIDE','GOODS','FREIGHT_PENDING','TRACK_PENDING','SNAPSHOT_PENDING']
        keys += ['FREIGHT_WORK', 'LAID_TRACK', 'FREIGHT_SHARE', 'WORK_TOOLTIP', 'SHARE_TOOLTIP', 'FREIGHT_FLOWS']
        translations += (['Trade qty', 'Laid track', 'Share',
                          'Original trade plus domestic model allocation, in goods units. Domestic logistics uses local supply/demand after original imports/exports and current original land/port links. Each shipment counts once; this card follows search/category, hover changes only the map. Not tonnes, convoy demand or distance work.',
                          'This good\'s original trade plus domestic model allocation divided by the all-market total. Filters and hover keep that denominator fixed.', 'Flows']
                         if language == 'english' else
                         ['贸易量', '已铺铁路', '占比',
                          '原版贸易量＋国内模型分配，使用商品数量单位。国内按原版进出口后的州净供需及当前陆路／港口连接分配；每笔货流只计一次。卡片跟随搜索／分类，悬停只改变地图。不是吨数、运输船需求或距离工作量。',
                          '该商品原版贸易量＋国内模型分配，除以全市场对应总量。筛选和悬停不改变分母。', '货流'])
        keys += ['FREIGHT_COLUMN', 'STATE_WORK_TOOLTIP']
        translations += (['Trade', 'Original trade plus domestic model allocation entering or leaving this state; each shipment counts once in goods units. Missing original geometry retains its known quantity. -- means data unavailable.']
                         if language == 'english' else
                         ['物流', '当前州原版贸易量＋国内模型分配，最终输入／输出每笔只计一次，单位为商品数量。缺少原版图形保留已知原版数量；-- 仅表示数据不可用。'])
        keys += ['LOCAL_PRICE', 'MARKET_PRICE', 'PRICE_DIFFERENCE']
        translations += (['Local price', 'Market price', 'Difference'] if language == 'english'
                         else ['本地价', '市场价', '价差'])
        keys += ['ORDERS_LEDGER', 'FREIGHT_LEDGER', 'MODEL_ALLOCATION', 'MAP_RETIRED']
        translations += (['Goods quantity: total / original trade / domestic model allocation',
                          'Domestic Freight demand: target / fulfilled / unmet', 'Original trade + domestic model allocation',
                          'Native map resources are not ready. Original quantities remain separate from route geometry.'] if language == 'english'
                         else ['商品量：总量／原版贸易／国内模型分配',
                               '国内 Freight 需求：目标／满足／未满足', '原版贸易＋国内模型分配',
                               '原生地图资源尚未就绪；原版数量与路线图形分开记录。'])
        keys += ['TRACK_TOOLTIP']
        translations += (['Current built railway strips with both endpoints in this market, each physical strip counted once, including within a state. MU means map units of the authored 2D polyline, not kilometres or track capacity. Independent of goods filters, hover and transported quantity. Zero means known no built railway; -- means unavailable.'] if language == 'english'
                         else ['两端均属当前市场的已建铁路，每个物理样条区段只计一次，含州内连接。MU 表示当前样条二维折线的地图单位，不是公里或运力。商品筛选、悬停及运量不改变此长度。0 表示已知没有已建铁路，-- 表示数据不可用。'])
        keys += ['MAP', 'BOTH_DIRECTIONS', 'IMPORTS', 'EXPORTS', 'ALL_GOODS', 'SEGMENTS',
                 'ALL_TRANSPORT', 'LAND', 'SEA', 'PICK_GOOD', 'STATE_FILTER_TOOLTIP', 'STATE_GOOD_TOOLTIP']
        translations += (['Map', 'Both', 'In', 'Out', 'All goods', 'Links',
                          'All', 'Land', 'Sea', 'Click Trade',
                          'Filters change only this state\'s map. Imports are green; exports are orange. Land or Sea shows only the matching original connection segments, including port approaches. Mixed routes retain their original path. The goods column always keeps complete state quantities; allocation, demand and market totals do not change. Switching states or closing this tab resets these filters.',
                          'Complete original trade plus domestic model quantity for this state and good. Click to show only this good on the map; All goods restores the map filter. Other goods rows keep their full quantities. -- means unavailable.']
                         if language == 'english' else
                         ['地图', '双向', '输入', '输出', '全部商品', '区段',
                          '全部', '陆路', '海路', '点物流选商品',
                          '筛选只改变当前州的地图。绿色为输入，橙色为输出。陆路或海路仅显示对应原生连接区段，海路包含进出港段；混合路线保持原路。商品列始终保留完整州运量，分配、需求账与市场总量不变。切州或关闭本页后筛选重置。',
                          '当前州该商品的完整原版贸易量＋国内模型分配。点击仅在地图显示该商品；点全部商品恢复。其他商品行仍显示完整数量，-- 表示不可用。'])
        keys += ['COLOR', 'COLOR_STATE', 'COLOR_MARKET', 'COLOR_GOODS', 'COLOR_NATIVE', 'COLOR_MODES',
                 'LEGEND', 'LEGEND_STATE', 'LEGEND_MARKET', 'LEGEND_GOODS', 'LEGEND_NATIVE',
                 'LEGEND_STATE_TOOLTIP', 'LEGEND_MARKET_TOOLTIP', 'LEGEND_GOODS_TOOLTIP', 'LEGEND_NATIVE_TOOLTIP',
                 'WIDTH_LEGEND', 'WIDTH_TOOLTIP', 'WIDTH_EXPLANATION']
        translations += (['Colour', 'Flow', 'Heat', 'Good', 'Native', 'Display only',
                          'Legend', 'Green imports / orange exports', 'Market heat: green → lime → yellow → orange → red',
                          'One good: hover a market row or click a state Trade cell', 'Native green: colour does not encode traffic',
                          'Direction relative to this state. Arrowheads follow the net original direction; opposing equal traffic has no one-way glyph. Width still follows final allocated goods quantity.',
                          'Each segment quantity uses the fixed maximum over the complete market. Bands: below 20%, 20–40%, 40–60%, 60–80%, and at least 80%. Filters and hover keep this scale. No quantity means no line; this is not capacity or congestion.',
                          'Only one selected good is highlighted, using its native goods ID for a stable palette. Hover a market row or click the state Trade column; without a good the map is empty. Leaving a market row returns to that empty selection. Cards, rows and allocation are unchanged.',
                          'Uniform colour from the verified native building-flow green style. Colour does not indicate imports, exports, heat or capacity. Original direction glyphs and quantity widths are retained.',
                          'Width: 8 fixed quantity bands',
                          'Final allocated goods quantity sets width, before distance or goods weights. Zero draws no line; missing data is --. Hover each sample for its exact interval. Samples show width ratios, not screen pixels.',
                          'Samples 1–8 show relative width. Hover for quantity thresholds; filters keep the absolute scale.']
                         if language == 'english' else
                         ['颜色', '流向', '热度', '单商品', '原生绿', '仅改变显示',
                          '图例', '绿色输入／橙色输出', '市场热度：绿→浅绿→黄→橙→红',
                          '单商品：悬停市场行，或点击地区物流列', '原生绿：颜色不表示流向或热度',
                          '相对当前州的流向。箭头沿原货流净方向，双向等量不显示单向箭头；线宽仍表示最终分配的商品数量。',
                          '区段数量以完整市场最大区段量为固定尺度。五档为低于20%、20–40%、40–60%、60–80%、至少80%。筛选和悬停不改变该尺度。没有数量不画线，不表示容量或拥堵。',
                          '仅高亮一个商品，按当前原生商品ID选择稳定配色。悬停市场商品行或点击地区物流列；未选商品时地图为空。移出市场行后恢复未选状态。卡片、商品列和分配结果不变。',
                          '统一使用已验证的原生建筑流线绿色样式；颜色不表示输入、输出、热度或容量，保留原有方向箭头与数量线宽。',
                          '线宽：按分配量分8档',
                          '线宽取最终分配商品数量，不乘距离或商品权重。零不画线，缺数据为--。悬停各示例查看精确数量区间；示例仅表示宽度比例，不是屏幕像素。',
                          '1–8档仅示宽度比例。悬停查看数量阈值；筛选保持绝对尺度。'])
        for band, interval in enumerate(['0 < q < 10', '10 ≤ q < 50', '50 ≤ q < 200', '200 ≤ q < 500',
                                         '500 ≤ q < 2000', '2000 ≤ q < 5000', '5000 ≤ q < 20000', 'q ≥ 20000']):
            keys += [f'WIDTH_{band}']
            translations += [f'Band {band+1}: {interval} goods units.' if language == 'english' else f'第{band+1}档：{interval}，单位为商品数量。']
        content = 'l_' + language + ':\n' + ''.join(f' IRONROAD_{key}:0 "{value}"\n' for key, value in zip(keys, translations))
        files[f'localization/{language}/ironroad_l_{language}.yml'] = content.encode('utf-8-sig')
    return files


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game-root', type=Path, default=Path('C:/Program Files (x86)/Steam/steamapps/common/Victoria 3'))
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    if args.out.resolve().is_relative_to(args.game_root.resolve()):
        parser.error('preview output must be outside the game installation')
    source = args.game_root / 'game/gui/market_panel.gui'
    target = args.out / 'gui'
    target.mkdir(parents=True, exist_ok=True)
    (target / 'market_panel.gui').write_text(patch_market(source.read_bytes()), encoding='utf-8')
    (target / 'states_panel.gui').write_text(patch_state((args.game_root / 'game/gui/states_panel.gui').read_bytes()), encoding='utf-8')
    root = Path(__file__).resolve().parents[1]
    own = (root / 'native/assets/ironroad_logistics.gui').read_text(encoding='utf-8')
    (target / 'ironroad_logistics.gui').write_text(patch_logistics_fragment(source.read_bytes(), own), encoding='utf-8')
    for name, data in localization_files().items():
        destination = args.out / name
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(data)
    print(f'Native GUI patch preview written outside game: {args.out}')
    print('Not deployed; native parser, VFS override precedence and DLL bindings still need verification.')


if __name__ == '__main__':
    main()
