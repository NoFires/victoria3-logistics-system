"""Derive the world height scale from captured native arrow paths.

The SetPath hook logs lines like:

  CAPTURE setpath obj=... type=... n=... ... first=(x y z) last=(x y z) heightmap=(h0 h1) ...

The game's `y` for an arrow point is the engine's world height; `heightmap` is our
downsampled height sample at the same x/z. `height_scale = median(y / h)` for samples
away from sea level. `coord_scale` stays 1 unless the x/z span is implausible.
"""
import argparse
import os
import re
import statistics
from pathlib import Path

PAT = re.compile(
    r"CAPTURE setpath .*?first=\(([-\d.]+) ([-\d.]+) ([-\d.]+)\) "
    r"last=\(([-\d.]+) ([-\d.]+) ([-\d.]+)\) heightmap=\(([-\d.]+) ([-\d.]+)\)")

WORLD_W, WORLD_H = 16384.0, 7232.0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--log", type=Path,
                    default=Path(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)")) / "Steam/steamapps/common/Victoria 3/binaries/freight/freight.log")
    ap.add_argument("--ini", type=Path, help="rewrite height_scale in this freight.ini")
    ap.add_argument("--min-height", type=float, default=300.0,
                    help="ignore samples below this height map value (sea level noise)")
    args = ap.parse_args()
    if not args.log.is_file():
        print("no log at", args.log)
        return 2

    ratios = []
    spans = []
    samples = 0
    for line in args.log.read_text(encoding="utf-8", errors="replace").splitlines():
        m = PAT.search(line)
        if not m:
            continue
        vals = [float(x) for x in m.groups()]
        ax, ay, az, bx, by, bz, ha, hb = vals
        samples += 1
        spans.append((abs(bx - ax), abs(bz - az)))
        for y, h in ((ay, ha), (by, hb)):
            if h >= args.min_height and abs(y) > 1.0:
                ratios.append(y / h)

    print(f"captures: {samples}")
    if not ratios:
        print("no usable samples (move an army or a supply route in-game first)")
        return 1
    ratios.sort()
    med = statistics.median(ratios)
    p10 = ratios[int(len(ratios) * 0.1)]
    p90 = ratios[int(len(ratios) * 0.9)]
    print(f"height samples: {len(ratios)}")
    print(f"y/heightmap: median={med:.5f}  p10={p10:.5f}  p90={p90:.5f}")
    if p10 > 0 and p90 / p10 > 3.0:
        print("warning: wide spread; captures may mix coordinate systems or sea level")
    print(f"suggested height_scale={med:.5f}")
    if spans:
        mx = max(s[0] for s in spans)
        mz = max(s[1] for s in spans)
        print(f"capture xz span: {mx:.0f} x {mz:.0f} (world {WORLD_W:.0f} x {WORLD_H:.0f})")
        if mx > WORLD_W or mz > WORLD_H:
            print("warning: coordinates exceed the map extent; coord_scale may be wrong")

    if args.ini:
        if not args.ini.is_file():
            print("ini not found:", args.ini)
            return 2
        lines = args.ini.read_text(encoding="utf-8").splitlines()
        out, replaced = [], False
        for line in lines:
            if line.startswith("height_scale="):
                out.append(f"height_scale={med:.5f}")
                replaced = True
            else:
                out.append(line)
        if not replaced:
            out.append(f"height_scale={med:.5f}")
        args.ini.write_text("\n".join(out) + "\n", encoding="utf-8")
        print("updated", args.ini)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
