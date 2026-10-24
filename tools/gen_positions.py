"""Generate Ironroad's offline map data from the installed Victoria 3 files.

Outputs (into <repo>/data):
  positions.txt : state id, key, display name, world x/y/z (heightmap pixel space)
  heights.bin   : downsampled height field (IRHM v1)

World-space convention: the engine's map extent equals the original height map
(16384 x 7232). Province pixels (8192 x 3616) are scaled by 2 and z maps to the
row axis. Y is the raw 16-bit height sample; the native overlay scales it at
runtime (height_scale), which is calibrated in the ImGui panel.
"""
import argparse
import csv
import math
import re
import struct
from pathlib import Path

import numpy as np
from PIL import Image

Image.MAX_IMAGE_PIXELS = None

WORLD_W = 8192.0
WORLD_H = 3616.0
PROV_TO_WORLD = 1.0


def load_palette(province_png: Path):
    im = Image.open(province_png).convert("RGB")
    w, h = im.size
    arr = np.asarray(im, dtype=np.uint32)
    keys = (arr[:, :, 0] << 16) | (arr[:, :, 1] << 8) | arr[:, :, 2]
    flat = keys.reshape(-1)
    uniq, inverse, counts = np.unique(flat, return_inverse=True, return_counts=True)
    ys = np.repeat(np.arange(h, dtype=np.float64), w)
    xs = np.tile(np.arange(w, dtype=np.float64), h)
    sum_x = np.bincount(inverse, weights=xs, minlength=len(uniq))
    sum_y = np.bincount(inverse, weights=ys, minlength=len(uniq))
    centroids = {}
    for i, key in enumerate(uniq.tolist()):
        c = int(counts[i])
        if c <= 0:
            continue
        centroids[key] = (sum_x[i] / c, sum_y[i] / c, c)
    return w, h, centroids


STATE_RE = re.compile(r"^\s*([A-Z][A-Z0-9_]+)\s*=\s*\{", re.M)


def parse_state_regions(folder: Path):
    states = {}
    for path in sorted(folder.glob("*.txt")):
        text = path.read_text(encoding="utf-8-sig", errors="replace")
        for m in STATE_RE.finditer(text):
            key = m.group(1)
            # Find the matching closing brace (simple brace counter; region files
            # do not contain braces inside strings).
            i = text.find("{", m.start())
            depth = 0
            end = None
            for j in range(i, len(text)):
                ch = text[j]
                if ch == "{":
                    depth += 1
                elif ch == "}":
                    depth -= 1
                    if depth == 0:
                        end = j
                        break
            if end is None:
                continue
            body = text[i + 1:end]
            st = {"key": key, "provinces": [], "city": None, "port": None}
            pm = re.search(r"provinces\s*=\s*\{([^}]*)\}", body)
            if pm:
                st["provinces"] = re.findall(r'"x([0-9A-Fa-f]{6})"', pm.group(1))
            for field in ("city", "port"):
                fm = re.search(field + r'\s*=\s*"x([0-9A-Fa-f]{6})"', body)
                if fm:
                    st[field] = fm.group(1)
            idm = re.search(r"id\s*=\s*(\d+)", body)
            st["id"] = int(idm.group(1)) if idm else 0
            states[key] = st
    return states


def parse_localization(path: Path):
    names = {}
    if not path.is_file():
        return names
    text = path.read_text(encoding="utf-8-sig", errors="replace")
    for line in text.splitlines():
        m = re.match(r'\s*([A-Z0-9_]+):\d*\s+"(.*)"\s*$', line)
        if m:
            names[m.group(1)] = m.group(2).replace('\\"', '"')
    return names


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--game-root", required=True, type=Path)
    ap.add_argument("--out", required=True, type=Path)
    args = ap.parse_args()

    game = args.game_root / "game"
    provinces_png = game / "map_data" / "provinces.png"
    height_png = game / "map_data" / "heightmap.png"
    regions = game / "map_data" / "state_regions"
    loc = game / "localization" / "english" / "map" / "states_l_english.yml"
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)

    print("reading province map", provinces_png)
    pw, ph, centroids = load_palette(provinces_png)
    print(f"  {pw}x{ph}, {len(centroids)} colours")
    print("parsing state regions", regions)
    states = parse_state_regions(regions)
    names = parse_localization(loc)
    print(f"  {len(states)} states, {len(names)} localised names")

    print("reading height map", height_png)
    him = Image.open(height_png)
    hw, hh = him.size
    heights = np.asarray(him, dtype=np.uint16)

    def province_center(hex_rgb):
        key = int(hex_rgb, 16)
        c = centroids.get(key)
        return c

    rows = []
    missing_city = 0
    for key, st in sorted(states.items(), key=lambda kv: kv[1]["id"]):
        city = st["city"]
        center = province_center(city) if city else None
        if center is None:
            pts = [province_center(p) for p in st["provinces"]]
            pts = [p for p in pts if p]
            if not pts:
                continue
            cx = sum(p[0] for p in pts) / len(pts)
            cy = sum(p[1] for p in pts) / len(pts)
            missing_city += 1
        else:
            cx, cy = center[0], center[1]
        wx = cx * PROV_TO_WORLD
        wz = cy * PROV_TO_WORLD
        hx = min(int(round(wx * 2.0)), hw - 1)
        hz = min(int(round(wz * 2.0)), hh - 1)
        wy = float(heights[hz, hx])
        display = names.get(key) or key.replace("STATE_", "").replace("_", " ").title()
        city_hex = city.upper() if (city and center is not None) else ""
        rows.append((st["id"], key, display, wx, wy, wz, city_hex))

    # Height field downsample (box average, 16x).
    step = 16
    fw = hw // step
    fh = hh // step
    hs = heights[:fh * step, :fw * step].astype(np.float32)
    hs = hs.reshape(fh, step, fw, step).mean(axis=(1, 3))
    hs = np.clip(hs, 0, 65535).astype(np.uint16)

    with (out / "heights.bin").open("wb") as f:
        f.write(struct.pack("<IIII", 0x4D485249, 1, fw, fh))
        f.write(struct.pack("<ffff", 0.0, 0.0, WORLD_W, WORLD_H))
        f.write(struct.pack("<II", 0, 0))
        f.write(hs.tobytes())

    lines = ["IRONROAD-POSITIONS 1",
             "# id\tkey\tname\tx\ty\tz\tcity  (world = original heightmap pixels)"]
    for rid, key, display, x, y, z, city_hex in rows:
        lines.append(f"{rid}\t{key}\t{display}\t{x:.2f}\t{y:.2f}\t{z:.2f}\t{city_hex}")
    (out / "positions.txt").write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(f"wrote {len(rows)} states ({missing_city} without a city province)")
    print(f"wrote height field {fw}x{fh} ({(out / 'heights.bin').stat().st_size} bytes)")
    print(f"output: {out}")


if __name__ == "__main__":
    main()

