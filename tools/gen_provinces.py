"""Generate the province adjacency graph used for approximate land routing.

Inputs: game/map_data/provinces.png, province_terrains.txt, heightmap.png
Output: data/provinces.bin

Format (little endian):
  u32 magic 'IRPG' (0x47505249), u32 version=1, u32 nodeCount, u32 edgeCount
  node[0..N): u32 color, f32 x, f32 z, f32 height, u32 firstEdge, u32 edgeCount,
              u32 terrainId, u32 flags        (32 bytes)
  edge[0..E): u32 targetNode               (4 bytes)

The graph is built from 4-neighbour pixel borders (plus the x wrap) between distinct
province colours. Ocean and lake provinces are present but flagged; the router only
walks land. This is an approximation of the authored spline network, not a replacement.
"""
import argparse
import re
import struct
from pathlib import Path

import numpy as np
from PIL import Image

Image.MAX_IMAGE_PIXELS = None

TERRAIN_FACTOR = {
    "plains": 1.0, "savanna": 1.1, "desert": 1.35, "forest": 1.25,
    "jungle": 1.6, "hills": 1.6, "wetland": 1.8, "tundra": 1.6,
    "mountain": 2.4, "snow": 2.2, "ocean": 64.0, "lakes": 64.0,
}
LAND_FLAG = 1
SEA_FLAG = 2


def load_terrain(path: Path):
    terrain = {}
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        m = re.match(r'x([0-9A-Fa-f]{6})="([^"]+)"', line)
        if m:
            terrain[int(m.group(1), 16)] = m.group(2)
    return terrain


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--game-root", required=True, type=Path)
    ap.add_argument("--out", required=True, type=Path)
    args = ap.parse_args()
    game = args.game_root / "game"
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)

    im = Image.open(game / "map_data" / "provinces.png").convert("RGB")
    w, h = im.size
    rgb = np.asarray(im, dtype=np.uint32)
    keys = (rgb[:, :, 0] << 16) | (rgb[:, :, 1] << 8) | rgb[:, :, 2]
    del rgb
    print(f"province map {w}x{h}")

    terrain_name = load_terrain(game / "map_data" / "province_terrains.txt")

    # Borders: right, down, and the x wrap (default.map has wrap_x = yes).
    pairs = []
    m = keys[:, :-1] != keys[:, 1:]
    pairs.append(np.stack([keys[:, :-1][m], keys[:, 1:][m]], axis=1))
    del m
    m = keys[:-1, :] != keys[1:, :]
    pairs.append(np.stack([keys[:-1, :][m], keys[1:, :][m]], axis=1))
    del m
    m = keys[:, 0] != keys[:, -1]
    pairs.append(np.stack([keys[:, 0][m], keys[:, -1][m]], axis=1))
    del m
    pairs = np.concatenate(pairs, axis=0)
    print("border pixel pairs:", len(pairs))
    lo = np.minimum(pairs[:, 0], pairs[:, 1]).astype(np.uint64)
    hi = np.maximum(pairs[:, 0], pairs[:, 1]).astype(np.uint64)
    keys64 = np.unique((lo << np.uint64(32)) | hi)
    del pairs, lo, hi
    a = (keys64 >> np.uint64(32)).astype(np.uint32)
    b = (keys64 & np.uint64(0xFFFFFFFF)).astype(np.uint32)
    print("unique borders:", len(keys64))

    # Centroid per colour (weighted by pixel count).
    flat = keys.reshape(-1)
    uniq, inverse, counts = np.unique(flat, return_inverse=True, return_counts=True)
    index = {int(c): i for i, c in enumerate(uniq.tolist())}
    ys = np.repeat(np.arange(h, dtype=np.float64), w)
    xs = np.tile(np.arange(w, dtype=np.float64), h)
    sum_x = np.bincount(inverse, weights=xs, minlength=len(uniq))
    sum_y = np.bincount(inverse, weights=ys, minlength=len(uniq))
    del inverse, xs, ys, flat
    cx = sum_x / np.maximum(counts, 1)
    cy = sum_y / np.maximum(counts, 1)

    him = Image.open(game / "map_data" / "heightmap.png")
    heights = np.asarray(him, dtype=np.uint16)
    hh, hw = heights.shape

    terrain_types = sorted(set(terrain_name.values()))
    terrain_id = {t: i for i, t in enumerate(terrain_types)}

    n = len(uniq)
    first = np.zeros(n, dtype=np.uint32)
    ecount = np.zeros(n, dtype=np.uint32)
    src = np.concatenate([np.fromiter((index[int(x)] for x in a), dtype=np.uint32),
                          np.fromiter((index[int(x)] for x in b), dtype=np.uint32)])
    dst = np.concatenate([np.fromiter((index[int(x)] for x in b), dtype=np.uint32),
                          np.fromiter((index[int(x)] for x in a), dtype=np.uint32)])
    order = np.argsort(src, kind="stable")
    src = src[order]
    dst = dst[order]
    counts_per = np.bincount(src, minlength=n)
    first = np.concatenate([[0], np.cumsum(counts_per)[:-1]]).astype(np.uint32)
    ecount = counts_per.astype(np.uint32)
    edge_targets = dst

    nodes = bytearray()
    for i, color in enumerate(uniq.tolist()):
        x = float(cx[i])
        z = float(cy[i])
        hx = min(int(round(x * 2.0)), hw - 1)
        hz = min(int(round(z * 2.0)), hh - 1)
        height = float(heights[hz, hx])
        tname = terrain_name.get(color)
        tid = terrain_id.get(tname, 0) if tname else 0
        sea = tname in ("ocean", "lakes")
        flags = SEA_FLAG if sea else LAND_FLAG
        nodes += struct.pack("<IfffIIII", color, x, z, height, int(first[i]),
                             int(ecount[i]), tid, flags)

    with (out / "provinces.bin").open("wb") as f:
        f.write(struct.pack("<IIII", 0x47505249, 1, n, len(edge_targets)))
        f.write(nodes)
        f.write(edge_targets.astype("<u4").tobytes())

    # terrain name table for the C++ side
    with (out / "terrain_names.txt").open("w", encoding="utf-8") as f:
        for i, t in enumerate(terrain_types):
            f.write(f"{i}\t{t}\t{TERRAIN_FACTOR.get(t, 1.0)}\n")

    print(f"nodes={n} edges={len(edge_targets)} land={int(np.count_nonzero(np.array([terrain_name.get(int(c)) not in ('ocean','lakes') for c in uniq.tolist()])))}")
    print(f"wrote {out / 'provinces.bin'} ({(out / 'provinces.bin').stat().st_size} bytes)")


if __name__ == "__main__":
    main()
