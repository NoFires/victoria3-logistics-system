"""Embed all required runtime data and original adapters in the proxy DLL."""
import argparse
import hashlib
from pathlib import Path
import zipfile
from make_arrow_colors import asset_text
from native_gui import TAB_OVERRIDE, localization_files

DATA_FILES = ('positions.txt', 'heights.bin', 'provinces.bin', 'terrain_names.txt', 'splines.bin')


def package(root, output):
    output.mkdir(parents=True, exist_ok=True)
    archive = output / 'ironroad-assets.zip'
    with zipfile.ZipFile(archive, 'w', compression=zipfile.ZIP_STORED) as z:
        for name, data in (
            ('gfx/models/military/ironroad_arrows.asset', asset_text().encode()),
            ('gfx/FX/ironroad_building_flow.shader', (root / 'native/assets/building_flow.shader').read_bytes()),
        ):
            info = zipfile.ZipInfo(name, date_time=(2026, 1, 1, 0, 0, 0))
            z.writestr(info, data)
    resources = [(151, archive)] + [(201+i, root / 'data' / name) for i, name in enumerate(DATA_FILES)]
    resources.append((301, root / 'native/assets/ironroad_logistics.gui'))
    for identity, (name, data) in enumerate(localization_files().items(), start=302):
        path = output / Path(name).name
        path.write_bytes(data)
        resources.append((identity, path))
    tab = output / 'native-tab.txt'
    tab.write_bytes(TAB_OVERRIDE.encode('utf-8'))
    resources.append((304, tab))
    text = '\n'.join(f'{identity} RCDATA "{path.resolve().as_posix()}"' for identity, path in resources) + '\n'
    (output / 'embedded.rc').write_text(text, encoding='utf-8')
    total = sum(path.stat().st_size for _, path in resources)
    print(f'Packaged {len(resources)} embedded resources ({total} bytes); assets sha256={hashlib.sha256(archive.read_bytes()).hexdigest()}')


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    package(Path(__file__).resolve().parents[1], args.output)
