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
    text = text[:start] + section + text[end+1:]
    braces(text)
    return text


def localization_files():
    files = {}
    for language, translations in {
        'english': ['Logistics', 'Show', 'Hide', 'Goods', 'Freight: --', 'Laid track: --', 'Live freight snapshot is not connected yet.'],
        'simp_chinese': ['物流', '显示', '隐藏', '货物', '货运量：--', '已铺铁路：--', '实时货运数据尚未接通。'],
    }.items():
        keys = ['LOGISTICS','SHOW','HIDE','GOODS','FREIGHT_PENDING','TRACK_PENDING','SNAPSHOT_PENDING']
        keys += ['FREIGHT_WORK', 'LAID_TRACK', 'FREIGHT_SHARE', 'WORK_TOOLTIP', 'SHARE_TOOLTIP', 'FREIGHT_FLOWS']
        translations += (['Freight', 'Laid track', '% of freight',
                          'Model allocation: orders x goods weight x route distance (map units). Current search/category selection; hover changes only the map.',
                          'This good\'s work divided by all-market work. Filters and hover keep that denominator fixed.', 'Freight flows']
                         if language == 'english' else
                         ['工作量', '已铺铁路', '占比',
                          '模型分配：订单数 × 商品权重 × 路程（地图单位）。卡片汇总当前搜索／分类；悬停仅改变地图。',
                          '该商品工作量除以全市场工作量。筛选和悬停不会改变分母。', '货运线路'])
        keys += ['FREIGHT_COLUMN', 'STATE_WORK_TOOLTIP']
        translations += (['Freight', 'Model allocation: work from final incoming and outgoing shipments for this state, counted once per shipment. Orders x goods weight x route distance (map units). -- means unavailable.']
                         if language == 'english' else
                         ['货运', '模型分配：当前地区最终输入与输出货流的工作量，每笔仅计一次。订单数 × 商品权重 × 路程（地图单位）。-- 表示数据不可用。'])
        keys += ['LOCAL_PRICE', 'MARKET_PRICE', 'PRICE_DIFFERENCE']
        translations += (['Local price', 'Market price', 'Difference'] if language == 'english'
                         else ['本地价', '市场价', '价差'])
        keys += ['ORDERS_LEDGER', 'FREIGHT_LEDGER', 'MODEL_ALLOCATION', 'MAP_RETIRED']
        translations += (['Goods orders: target / allocated / unmet',
                          'Freight units: target / fulfilled / unmet', 'Model allocation',
                          'Native map resources are not ready. Model figures update independently.'] if language == 'english'
                         else ['原商品订单：目标／已分配／未满足',
                               '物流需求账：目标／满足／未满足（模型单位）', '模型分配',
                               '原生地图资源尚未就绪；模型数值按当前存档独立更新。'])
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
