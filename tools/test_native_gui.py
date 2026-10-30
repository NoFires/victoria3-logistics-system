from pathlib import Path
import re
from native_gui import braces, block, patch_market, patch_state, patch_logistics_fragment, localization_files

GAME = Path('C:/Program Files (x86)/Steam/steamapps/common/Victoria 3/game')
raw = (GAME / 'gui/market_panel.gui').read_bytes()
patched = patch_market(raw)
assert patched.count("InformationPanel.SelectTab('ironroad_logistics')") == 1
assert patched.count('ironroad_market_logistics_content = {') == 1
original = raw.decode('utf-8-sig')
for tab in ('default','world_market_trade','food_security','states'):
    expression = f"InformationPanel.SelectTab('{tab}')"
    assert patched.count(expression) == original.count(expression), tab
assert patched.count('MarketPanel.GetSearchBar') == original.count('MarketPanel.GetSearchBar')
assert patched.count('MarketPanel.GetFilters') == original.count('MarketPanel.GetFilters')
# Keeping callback names is insufficient: the original controls must remain
# usable in Logistics, including the native expand/collapse condition.
control_starts = {
    'filter header': original.rfind('section_header_button = {', 0, original.index('"FILTERS"')),
    'search': original.index('search_bar = {'),
    'categories': original.rfind('flowcontainer = {', 0, original.index(
        'visible = "[Not(GetVariableSystem.Exists( \'market_filters\' ))]"', original.index('search_bar = {'))),
}
pairs = braces(original)
for name, start in control_starts.items():
    assert start >= 0, name
    opening = original.index('{', start)
    assert original[start:pairs[opening]+1] in patched, f'native {name} controls must be preserved'
assert "[Or(InformationPanel.IsTabSelected('ironroad_logistics'), Or(InformationPanel.IsTabSelected('default'), InformationPanel.IsTabSelected('world_market_trade')))]" in patched
braces('a = { text = "} # {" # ignored {\n child = {} }')
try:
    patch_market(raw + b'\n# changed installation')
except ValueError as error:
    assert 'differs from verified' in str(error)
else:
    raise AssertionError('unknown GUI version must fail closed')
fragment = (Path(__file__).resolve().parents[1] / 'native/assets/ironroad_logistics.gui').read_text()
braces(fragment)
assert 'on_start' not in fragment, '1.13.11 widget rejects on_start callback'
assert fragment.count('goods_entry_button = {') == 1
assert fragment.count('onmousehierarchyenter =') == 1
assert fragment.count('onmousehierarchyleave =') == 1
assert "GetVariableSystem.Get('ironroad_hover_context_v1')" in fragment
assert fragment.count("Concatenate('ironroad_market_hover_', Goods.GetKey)") == 2
assert "GetVariableSystem.Get('ironroad_demand_v1')" in fragment, 'domestic model must expose its Freight demand ledger'
assert 'text = "IRONROAD_ORDERS_LEDGER"' in fragment
assert 'text = "IRONROAD_FREIGHT_LEDGER"' in fragment
assert "GetVariableSystem.Get('ironroad_demand_v1')" in fragment
assert "EqualTo_string(GetVariableSystem.Get('ironroad_map_status_v1'), 'map-retired-state-v1')" in fragment
assert 'text = "IRONROAD_MAP_RETIRED"' in fragment
assert 'Target / allocated / unmet:' not in fragment
assert 'Freight demand / fulfilled / unmet (model units):' not in fragment
assert 'IRONROAD_FREIGHT_WORK' in fragment and 'IRONROAD_FREIGHT_SHARE' in fragment
assert "Concatenate('ironroad_good_work_', Goods.GetKey)" in fragment
assert "Concatenate('ironroad_good_share_', Goods.GetKey)" in fragment
assert fragment.count('__IRONROAD_NATIVE_MARKET_PRICE__') == 1
# Live 1.13.11 rejects `margin_right` on icon widgets:
#   pdx_gui_widget.cpp - Property 'margin_right'(815) not handled
#   pdx_gui_factory.cpp - Error setting properties for '' (icon)
# Insets must use position/size only, so keep the property out of the fragment.
for match in re.finditer(r'\bicon\s*=\s*\{', fragment):
    assert 'margin_right' not in fragment[match.end():braces(fragment)[match.end()-1]], 'unsupported icon margin'
resolved = patch_logistics_fragment(raw, fragment)
row_start, row_end = block(original, r'type\s+goods_entry_button\s*=\s*flowcontainer\s*\{')
price_start, price_end = block(original, r'block\s+"last_column"\s*\{', row_start, row_end)
assert original[price_start+1:price_end] in resolved
assert '__IRONROAD_NATIVE_MARKET_PRICE__' not in resolved
assert 'Goods.GetPriceTrend' in resolved and 'Goods.GetMarketPrice' in resolved
for source, template in [(raw+b'changed', fragment), (raw, fragment.replace('__IRONROAD_NATIVE_MARKET_PRICE__', '')),
                         (raw, fragment+'__IRONROAD_NATIVE_MARKET_PRICE__')]:
    try:
        patch_logistics_fragment(source, template)
    except ValueError:
        pass
    else:
        raise AssertionError('unknown source or missing/duplicate price placeholder must be rejected')
assert 'Freight (order-cost)' not in fragment
assert 'Market.AccessMarketGoods( MarketPanel.GetFilters )' in fragment
assert 'InformationPanelBar.OpenGoodsPanel' in (GAME / 'gui/market_panel.gui').read_text()
print('PASS: native GUI patch anchors, balanced blocks, original tabs/filter bindings, built-in goods navigation and unknown-version rejection')
state_raw = (GAME / 'gui/states_panel.gui').read_bytes()
state_original = state_raw.decode('utf-8-sig')
state_patched = patch_state(state_raw)
braces(state_patched)
assert state_patched.count('ironroad_state_freight_cell = {}') == 1
assert state_patched.count('ironroad_state_filters = {}') == 1
assert state_patched.count('IRONROAD_FREIGHT_COLUMN') == 1
# Preserve every vanilla script expression and its multiplicity, including all
# sorts, original prices/graphs, navigation, menus and native window lifecycle.
for expression in set(re.findall(r'"(\[[^"\n]+\])"', state_original)):
    assert state_patched.count(expression) == state_original.count(expression), expression
start = state_original.rfind('flowcontainer = {', 0, state_original.index(
    'visible =  "[InformationPanel.IsTabSelected(\'local_goods\')]"'))
end = braces(state_original)[state_original.index('{', start)]
assert state_patched.startswith(state_original[:start]) and state_patched.endswith(state_original[end+1:])
nonlocal_start = state_original.rfind('flowcontainer = {', 0, state_original.index(
    'visible = "[Not(IsDataModelEmpty(StatesPanel.AccessNonLocalGoods))]"'))
nonlocal_end = braces(state_original)[state_original.index('{', nonlocal_start)]
assert state_original[nonlocal_start:nonlocal_end+1] in state_patched
for bad in (state_raw+b'changed', raw):
    try:
        patch_state(bad)
    except ValueError:
        pass
    else:
        raise AssertionError('unknown state GUI or market GUI must be rejected')
assert 'State.GetIDString' in fragment and 'ironroad_state_rows_id_v1' in fragment
assert "Concatenate('ironroad_state_work_', Concatenate(State.GetIDString, Concatenate('_', Goods.GetKey)))" in fragment
assert 'IRONROAD_STATE_WORK_TOOLTIP' in state_patched
assert "GetVariableSystem.Get('ironroad_state_context_v1')" in fragment
for name in ('good', 'direction', 'transport'):
    assert f"ironroad_state_{name}_v1" in fragment
for value in ('imports', 'exports', 'land', 'sea'):
    assert f"'), ':{value}'))]" in fragment
assert 'IRONROAD_STATE_FILTER_TOOLTIP' in fragment and 'IRONROAD_STATE_GOOD_TOOLTIP' in fragment
labels = []
for match in re.finditer(r'\btextbox\s*=\s*\{', fragment):
    body = fragment[match.start():braces(fragment)[match.end()-1]+1]
    if 'parentanchor = center' in body and 'text = "IRONROAD_' in body:
        labels.append(body)
        assert 'size = { 100% 100% }' in body and 'align = center' in body, 'button labels need a bounded centered text box'
        assert 'alwaystransparent = yes' in body, 'full button label cannot capture its parent click'
assert len(labels) == 16
for name in ('state', 'market'):
    assert f"ironroad_{name}_color_v1" in fragment
assert fragment.count('ironroad_flow_legend = {}') == 2
assert "GetVariableSystem.Toggle('ironroad_legend_open_v1')" in fragment
assert "Concatenate(':', Goods.GetKey)" in fragment, 'selection indicator uses the native good key, never row order'
print('PASS: state Freight column, original lifecycle/sort/price/action bindings, untouched other panels and unknown-version rejection')

# Screenshot regressions: inherited goods icons consume 55px; child bars and
# auto-sized containers must not silently make the state row wider than 540px.
def children(text):
    pairs = braces(text)
    cursor = 0
    while match := re.search(r'\b(\w+)\s*=\s*\{', text[cursor:]):
        start = cursor + match.start()
        opening = cursor + match.end() - 1
        end = pairs[opening]
        yield match[1], text[start:end+1]
        cursor = end + 1


def direct(text):
    opening = text.index('{')
    body = text[opening+1:braces(text)[opening]]
    for kind, node in children(body):
        if kind not in ('size', 'minimumsize', 'maximumsize', 'position', 'margin'):
            body = body.replace(node, '', 1)
    return body


def width(text):
    match = re.search(r'\b(?:minimumsize|size)\s*=\s*\{\s*(\d+)\s', direct(text))
    if match and 'minimumsize' not in match[0]:
        return int(match[1])
    nested = [width(node) for kind, node in children(text[text.index('{')+1:-1])
              if kind in ('widget', 'textbox', 'flowcontainer', 'double_direction_progressbar_gold')]
    return max([int(match[1]) if match else 0, *nested])


header_start = state_patched.index('flowcontainer = {', state_patched.index(
    'visible =  "[InformationPanel.IsTabSelected(\'local_goods\')]"'))
header_end = braces(state_patched)[state_patched.index('{', header_start)]
header = state_patched[header_start:header_end+1]
headers = [node for kind, node in children(header[header.index('{')+1:-1]) if kind == 'sort_button']
model = state_patched.index('datamodel = "[StatesPanel.AccessLocalGoods]"')
row_start = state_patched.index('button = {', model)
row_end = braces(state_patched)[state_patched.index('{', row_start)]
row = state_patched[row_start:row_end+1]
flow_start = row.index('flowcontainer = {')
flow_end = braces(row)[row.index('{', flow_start)]
columns = [node for kind, node in children(row[row.index('{', flow_start)+1:flow_end])
           if kind in ('flowcontainer', 'widget', 'container', 'ironroad_state_freight_cell')]
state_cell_start, state_cell_end = block(fragment, r'type\s+ironroad_state_freight_cell\s*=\s*widget\s*\{')
state_cell = fragment[state_cell_start:state_cell_end+1]
state_widths = [55, width(columns[0]), width(columns[1]), width(columns[2]),
                width(columns[3]), width(columns[4]), width(columns[5]), width(state_cell)]
assert sum(state_widths) <= 540, ('state row overflows', state_widths)
assert [width(node) for node in headers] == state_widths, 'state headers must follow actual cells including goods icon'
assert 'minimumsize' not in direct(columns[3]) and 'minimumsize' not in direct(columns[4]), 'price cells must have fixed bounds'
assert 'sort_icon_good.dds' in fragment and "Market.Sort('name')" in fragment, 'missing market goods header'
market_header_start = fragment.rfind('flowcontainer = {', 0, fragment.index('tooltip = "SORT_BY_MARKET_GOODS"'))
market_header_end = braces(fragment)[fragment.index('{', market_header_start)]
market_headers = [node for kind, node in children(fragment[fragment.index('{', market_header_start)+1:market_header_end])
                  if kind == 'sort_button']
market_row_start, market_row_end = block(resolved, r'blockoverride\s+"last_column"\s*\{')
market_columns = [node for kind, node in children(resolved[market_row_start+1:market_row_end]) if kind == 'widget']
assert sum(width(node) for node in market_headers) == 540
assert [width(node) for node in market_headers[:3]] == [55, width(market_columns[0]), width(market_columns[1])]
assert sum(width(node) for node in market_headers[3:]) == width(market_columns[2])
assert 55 + sum(width(node) for node in market_columns) == 540, 'market content must fit inherited row'
for key in ('MARKET_PANEL_BALANCE_GOLD', 'MARKET_PANEL_BALANCE', 'MARKET_PANEL_BALANCE_BLUE'):
    start = fragment.rfind('textbox = {', 0, fragment.index('text = "' + key + '"'))
    end = braces(fragment)[fragment.index('{', start)]
    cell = fragment[start:end+1]
    assert 'autoresize' not in cell and re.search(r'size\s*=\s*\{\s*\d+\s+55\s*\}', cell), 'balance floats at row top'
print('PASS: screenshot layout bounds, state header/cell alignment, market goods header and centered balance cells')

# The formal legend must describe the exact production width bands, not example
# screenshot values. Visual samples are four times the world-style width ratio.
width_source = (Path(__file__).resolve().parents[1] / 'native/src/flow_width.h').read_text(encoding='utf-8')
thresholds = [float(n) for n in re.search(r'kFlowWidthThresholds\{\{([^}]+)', width_source)[1].split(',')]
widths = [float(n.strip().rstrip('f')) for n in re.search(r'kFlowWidths\{\{([^}]+)', width_source)[1].split(',')]
legend_start, legend_end = block(fragment, r'type\s+ironroad_flow_legend\s*=\s*flowcontainer\s*\{')
legend = fragment[legend_start:legend_end+1]
bars = re.findall(r'size = \{ 40 (\d+) \}', legend)
assert [int(n) for n in bars] == [4*w for w in widths]
assert 8*64+7*2 <= 530
for content in localization_files().values():
    text = content.decode('utf-8-sig')
    for band in range(8):
        line = next(line for line in text.splitlines() if line.startswith(f' IRONROAD_WIDTH_{band}:'))
        expected = ([0, thresholds[0]] if band == 0 else [thresholds[-1]] if band == 7 else [thresholds[band-1], thresholds[band]])
        for threshold in expected:
            assert re.search(rf'(?<!\d){threshold:g}(?!\d)', line), (band, line)
    for key in ('LEGEND_STATE', 'LEGEND_MARKET', 'LEGEND_GOODS', 'LEGEND_NATIVE', 'WIDTH_TOOLTIP'):
        assert f' IRONROAD_{key}:' in text
print('PASS: formal colour modes, selected-good indicator and bilingual eight-band legend match production width thresholds/ratios')
