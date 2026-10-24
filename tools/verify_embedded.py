"""Check the actual DLL's Windows resources, ZIP entries and embedded map data."""
import argparse
import hashlib
import io
import re
from pathlib import Path
import struct
import zipfile
from ironpe import PE
from make_arrow_colors import asset_text, STYLES, WIDTHS
from package_runtime import DATA_FILES
from native_gui import TAB_OVERRIDE, localization_files


def resources(pe):
    base = pe.dirs[2][0]
    def children(offset):
        raw = pe.rd(base + offset, 16)
        named, count = struct.unpack_from('<HH', raw, 12)
        for i in range(named + count):
            identity, pointer = struct.unpack('<II', pe.rd(base + offset + 16 + i*8, 8))
            yield identity, pointer
    found = {}
    for kind, type_pointer in children(0):
        if kind != 10:
            continue
        for identity, name_pointer in children(type_pointer & 0x7fffffff):
            for language, leaf in children(name_pointer & 0x7fffffff):
                address, size = struct.unpack_from('<II', pe.rd(base + leaf, 16))
                assert identity not in found, 'Ambiguous resource language'
                found[identity] = pe.rd(address, size)
    return found


def verify(dll, root):
    data = resources(PE(str(dll)))
    assert set(data) == {151,201,202,203,204,205,301,302,303,304}, f'Unexpected resources: {set(data)}'
    assert data[301] == (root / 'native/assets/ironroad_logistics.gui').read_bytes()
    for identity, expected in enumerate(localization_files().values(), start=302):
        assert data[identity] == expected
    assert data[304] == TAB_OVERRIDE.encode('utf-8')
    with zipfile.ZipFile(io.BytesIO(data[151])) as archive:
        assert set(archive.namelist()) == {'gfx/models/military/ironroad_arrows.asset', 'gfx/FX/ironroad_building_flow.shader'}
        assert archive.read('gfx/models/military/ironroad_arrows.asset') == asset_text().encode()
        assert archive.read('gfx/FX/ironroad_building_flow.shader') == (root / 'native/assets/building_flow.shader').read_bytes()
        assert not any('.metadata' in name or name.endswith('.mod') or name.endswith('.dds') for name in archive.namelist())
        header = (root / 'native/src/flow_width.h').read_text(encoding='utf-8')
        widths = re.search(r'kFlowWidths\{\{(.*?)\}\}', header).group(1)
        assert tuple(float(x.strip().removesuffix('f')) for x in widths.split(',')) == WIDTHS, 'C++/resource width mismatch'
        shader = (root / 'native/assets/building_flow.shader').read_text(encoding='utf-8')
        for effect in set(STYLES.values()) - {'Tan'}:
            assert f'Effect IronroadBuilding{effect}' in shader, effect
    for i, name in enumerate(DATA_FILES):
        assert data[201+i] == (root / 'data' / name).read_bytes(), name
    print(f'PASS: standalone DLL contains adapters, five exact maps and own native GUI resources; no playset metadata, copied base GUI or game textures. SHA256={hashlib.sha256(dll.read_bytes()).hexdigest()}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--dll', type=Path, default=Path('dist/freight/freight.dll'))
    args = parser.parse_args()
    verify(args.dll, Path(__file__).resolve().parents[1])
