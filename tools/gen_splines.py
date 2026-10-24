"""Decode the installed game's version-4 spline network (no runtime Python).

The file is a token stream, not map_data/nodes.dat. Unknown schema/tokens fail
closed. Output uses province-image X/Z to match positions.txt; the native upload
converts Z back to the engine's bottom-origin coordinates exactly once.
"""
import argparse
import collections
import hashlib
import math
import struct
import re
from pathlib import Path


def decode(data):
    pos = 0

    def token():
        nonlocal pos
        if pos + 2 > len(data):
            raise ValueError('truncated token')
        t, = struct.unpack_from('<H', data, pos)
        pos += 2
        formats = {0x0C: 'i', 0x14: 'I', 0x0D: 'f', 0x029C: 'Q'}
        if t in formats:
            fmt = '<' + formats[t]
            size = struct.calcsize(fmt)
            if pos + size > len(data):
                raise ValueError('truncated scalar')
            value, = struct.unpack_from(fmt, data, pos)
            pos += size
            return ('value', value)
        if t not in (1, 3, 4, 11, 76, 238, 1114, 1524, 1525, 1526, 1527):
            raise ValueError(f'unknown spline token {t:#x} at {pos - 2:#x}')
        return ('token', t)

    def block(nested=False):
        out = []
        while pos < len(data):
            item = token()
            if item == ('token', 4):
                if not nested:
                    raise ValueError('unexpected closing brace')
                return out
            out.append(block(True) if item == ('token', 3) else item)
        if nested:
            raise ValueError('unclosed block')
        return out

    tree = block()
    expected = [(0, 238), (1, 1), (3, 1114), (4, 1), (6, 1524),
                (7, 1), (9, 1525), (10, 1), (12, 1526), (13, 1)]
    if len(tree) != 15 or any(tree[i] != ('token', t) for i, t in expected):
        raise ValueError('unsupported spline schema')
    if tree[2] != ('value', 4):
        raise ValueError('unsupported spline version')

    def records(items, key):
        out = {}
        for r in items:
            if (len(r) != 6 or r[0] != ('token', 11) or r[1] != ('token', 1)
                    or r[3] != ('token', key) or r[4] != ('token', 1)
                    or r[2][0] != 'value' or any(v[0] != 'value' for v in r[5])):
                raise ValueError('invalid spline record')
            rid = r[2][1]
            if rid in out:
                raise ValueError('duplicate spline record')
            out[rid] = [v[1] for v in r[5]]
        return out

    points = records(tree[8], 76)
    strips = records(tree[11], 1527)
    links = records(tree[14], 1525)
    if [v[1] for v in tree[5]] != [len(points), len(strips), len(links)]:
        raise ValueError('spline counts disagree')
    for p in points.values():
        if len(p) != 2 or not all(math.isfinite(v) for v in p):
            raise ValueError('invalid spline point')
    for sid, refs in strips.items():
        if len(refs) < 2 or any(r not in points for r in refs) or sid & 255 not in range(4):
            raise ValueError('invalid spline references/type')
    used = set()
    for key, refs in links.items():
        current, target, kind = (key >> 6) & 0x1FFFFFFF, key >> 35, key & 63
        if not refs or kind > 3:
            raise ValueError('invalid spline link')
        for sid in refs:
            if sid not in strips or sid in used or sid & 255 != kind:
                raise ValueError('invalid spline link membership')
            used.add(sid)
            a, b = strips[sid][0], strips[sid][-1]
            if current not in (a, b):
                raise ValueError('disconnected spline link')
            current = b if current == a else a
        if current != target:
            raise ValueError('invalid spline link endpoints')
    if used != strips.keys():
        raise ValueError('unbound spline strip')
    return points, strips, links


def generate(source, output, width=8192.0, height=3616.0):
    raw = source.read_bytes()
    points, strips, links = decode(raw)
    endpoints = sorted({r for refs in strips.values() for r in (refs[0], refs[-1])})
    # Land hub +3 is the port; naval anchors encode the same state region with
    # bit 23 set. Verify against the installed port locator, rather than joining
    # geographically nearest cities or bridging arbitrary disconnected islands.
    locator_path = source.parent.parent / 'map_object_data/generated_map_object_locators_port.txt'
    locators = {}
    for rid, x, _y, z in re.findall(
            r'\bid=(\d+)\s+position=\{\s*([-\d.]+)\s+([-\d.]+)\s+([-\d.]+)',
            locator_path.read_text(encoding='utf-8-sig')):
        locators[int(rid)] = (float(x), float(z))
    transfers, skipped = [], []
    for naval in endpoints:
        if naval >= 0x10000000 or not naval & 0x800000:
            continue
        anchor = naval & 0x7FFFFF
        region, suffix = divmod(anchor, 100)
        land = anchor + 3
        if suffix or land not in points or region not in locators:
            continue
        gap = math.dist(points[naval], points[land])
        if math.dist(points[land], locators[region]) > 3 or not 0.001 < gap <= 10:
            skipped.append(region)
            continue
        transfers.append((region, land, naval, gap))
    endpoints = sorted(set(endpoints) | {land for _, land, _, _ in transfers})
    index = {rid: i for i, rid in enumerate(endpoints)}
    point_rows, edges = [], []
    for sid, refs in sorted(strips.items()):
        coords = [points[r] for r in refs]
        length = sum(math.hypot(b[0] - a[0], b[1] - a[1])
                     for a, b in zip(coords, coords[1:]))
        if length <= 0 or not math.isfinite(length):
            raise ValueError('degenerate spline')
        edges.append((sid, index[refs[0]], index[refs[-1]], sid & 255,
                      len(point_rows), len(coords), length))
        point_rows.extend((x, height - z) for x, z in coords)
    for region, land, naval, gap in transfers:
        edges.append((0xF0000000 | region, index[land], index[naval], 4,
                      len(point_rows), 2, gap))
        point_rows.extend((x, height - z) for x, z in (points[land], points[naval]))
    output.parent.mkdir(parents=True, exist_ok=True)
    with output.open('wb') as f:
        f.write(struct.pack('<IIffIII', 0x47535249, 3, width, height,
                            len(endpoints), len(edges), len(point_rows)))
        for rid in endpoints:
            x, z = points[rid]
            f.write(struct.pack('<Iff', rid, x, height - z))
        for e in edges:
            f.write(struct.pack('<IIIIIIf', *e))
        for p in point_rows:
            f.write(struct.pack('<ff', *p))
        f.write(struct.pack('<I', len(links)))
        for key, refs in sorted(links.items()):
            f.write(struct.pack('<QI', key, len(refs)))
            f.write(struct.pack('<' + 'I' * len(refs), *refs))
    print(f'OK: {len(endpoints)} junctions, {len(strips)} authored splines + {len(transfers)} transfers, '
          f'{len(point_rows)} points; types={dict(collections.Counter(e[3] for e in edges))}')
    print(f'source SHA-256: {hashlib.sha256(raw).hexdigest()}')
    print(f'port transfers: {len(transfers)} verified short shore links; '
          f'{len(skipped)} excluded locator/gap mismatches: {skipped}')


if __name__ == '__main__':
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--game-root', type=Path, required=True)
    ap.add_argument('--out', type=Path, default=Path('data/splines.bin'))
    args = ap.parse_args()
    generate(args.game_root / 'game/gfx/map/spline_network/spline_network.splnet', args.out)
