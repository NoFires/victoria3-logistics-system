"""Generate an original arrowType definition referencing recipient-owned assets."""
import argparse
import re
from pathlib import Path


def build_text(name):
    if not re.fullmatch(r'[A-Za-z][A-Za-z0-9_]*', name):
        raise ValueError('Use an identifier containing letters, digits and underscores.')
    return ('arrowType = {\n'
            '    name = "%s"\n'
            '    texture = "gfx/map/movement_arrows/movearrow_move.dds"\n'
            '    overlay = "gfx/map/movement_arrows/movearrow_move.dds"\n'
            '    effect = "gfx/FX/arrow.shader"\n'
            '    effectname = "ArrowTextureColorEffect"\n'
            '    size = 1.0\n'
            '    subdivisions = 5\n'
            '    line_smoothing = yes\n'
            '}\n') % name


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game-root', type=Path, required=True, help='Folder containing the game subfolder.')
    parser.add_argument('--out', type=Path, required=True, help='A NEW standalone asset file in your mod.')
    parser.add_argument('--name', default='localFreightPreview')
    args = parser.parse_args()
    root = args.game_root.resolve()
    if args.out.resolve().is_relative_to(root):
        parser.error('Output must be in your separate mod, outside the installation.')
    for relative in ('game/gfx/map/movement_arrows/movearrow_move.dds', 'game/gfx/FX/arrow.shader'):
        if not (root / relative).is_file():
            parser.error('Expected local asset missing: %s. Inspect this build before proceeding.' % relative)
    text = build_text(args.name)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    with args.out.open('x', encoding='utf-8') as stream:
        stream.write(text)
    print('Created custom arrowType definition. Enable the mod and restart before resolving it.')


if __name__ == '__main__':
    main()
