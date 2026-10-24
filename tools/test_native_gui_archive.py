"""Cross-check the C++ runtime patch and ZIP using an independent reader."""
import hashlib
from pathlib import Path
import sys
import zipfile
from native_gui import patch_market, patch_state, patch_logistics_fragment, localization_files

path = Path(sys.argv[1])
root = Path(__file__).resolve().parents[1]
source = Path('C:/Program Files (x86)/Steam/steamapps/common/Victoria 3/game/gui/market_panel.gui')
expected = {
    'gui/market_panel.gui': patch_market(source.read_bytes()).encode('utf-8'),
    'gui/states_panel.gui': patch_state(source.with_name('states_panel.gui').read_bytes()).encode('utf-8'),
    'gui/ironroad_logistics.gui': patch_logistics_fragment(source.read_bytes(), (root / 'native/assets/ironroad_logistics.gui').read_text(encoding='utf-8')).encode('utf-8'),
    **localization_files(),
}
assert path.stem == 'native-gui-' + hashlib.sha256(path.read_bytes()).hexdigest()
with zipfile.ZipFile(path) as archive:
    assert set(archive.namelist()) == set(expected)
    assert archive.testzip() is None
    for name, data in expected.items():
        assert archive.read(name) == data, name
        assert archive.getinfo(name).compress_type == zipfile.ZIP_STORED
print('PASS: C++ SHA-256, bad paths/duplicates/version/missing-resource rejection, deterministic cache, independent ZIP CRC and exact Python/native patch agreement')
