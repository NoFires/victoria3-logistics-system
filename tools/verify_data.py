"""Validate the generated Ironroad map data (offline, stdlib only)."""
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / "data"
WORLD_W = 8192.0
WORLD_H = 3616.0


def fail(msg):
    print("FAIL:", msg)
    return 1


def main():
    pos = DATA / "positions.txt"
    heights = DATA / "heights.bin"
    if not pos.is_file():
        return fail("data/positions.txt missing (run tools/gen_positions.py)")
    if not heights.is_file():
        return fail("data/heights.bin missing (run tools/gen_positions.py)")

    lines = pos.read_text(encoding="utf-8").splitlines()
    if not lines or not lines[0].startswith("IRONROAD-POSITIONS"):
        return fail("positions.txt header")
    rows = []
    for line in lines:
        if not line or line[0] == "#":
            continue
        parts = line.split("\t")
        if len(parts) < 6:
            continue
        rows.append((int(parts[0]), parts[1], parts[2], float(parts[3]), float(parts[4]),
                     float(parts[5]), parts[6] if len(parts) > 6 else ""))
    if len(rows) < 600:
        return fail(f"only {len(rows)} states")
    ids = [r[0] for r in rows]
    keys = [r[1] for r in rows]
    if len(set(ids)) != len(ids):
        return fail("duplicate state ids")
    if len(set(keys)) != len(keys):
        return fail("duplicate state keys")
    for rid, key, name, x, y, z, city in rows:
        if not key.startswith("STATE_"):
            return fail(f"unexpected key {key}")
        if not (0.0 <= x <= WORLD_W and 0.0 <= z <= WORLD_H):
            return fail(f"{key} outside world bounds ({x},{z})")
        if not (0.0 <= y <= 65535.0):
            return fail(f"{key} bad height {y}")
        if not name:
            return fail(f"{key} has no name")

    raw = heights.read_bytes()
    magic, version, w, h = struct.unpack_from("<IIII", raw, 0)
    x0, z0, x1, z1 = struct.unpack_from("<ffff", raw, 16)
    if magic != 0x4D485249 or version != 1:
        return fail("heights.bin header")
    if (x0, z0, x1, z1) != (0.0, 0.0, WORLD_W, WORLD_H):
        return fail("heights.bin world bounds")
    if len(raw) != 40 + w * h * 2:
        return fail("heights.bin payload size")
    if w < 256 or h < 128:
        return fail("height field too small")

    prov = DATA / "provinces.bin"
    if not prov.is_file():
        return fail("data/provinces.bin missing (run tools/gen_provinces.py)")
    p = prov.read_bytes()
    pmagic, pver, pn, pe = struct.unpack_from("<IIII", p, 0)
    if pmagic != 0x47505249 or pver != 1:
        return fail("provinces.bin header")
    if len(p) != 16 + pn * 32 + pe * 4:
        return fail("provinces.bin size")
    if pn < 1000 or pe < pn:
        return fail("provinces.bin implausible counts")
    first_edges_ok = True
    land = 0
    for i in range(0, pn, max(1, pn // 1000)):
        off = 16 + i * 32
        color, x, z, hh, first, cnt, tid, flags = struct.unpack_from("<IfffIIII", p, off)
        if not (0 <= x <= WORLD_W and 0 <= z <= WORLD_H):
            return fail(f"province node {i} outside world")
        if flags & 1:
            land += 1
        if first + cnt > pe:
            first_edges_ok = False
    if not first_edges_ok:
        return fail("province edge table out of range")
    if land == 0:
        return fail("province graph has no land nodes")
    if not (DATA / "terrain_names.txt").is_file():
        return fail("terrain_names.txt missing")

    states_with_city = sum(1 for r in rows if len(r) > 6 and r[6])
    print(f"OK: {len(rows)} states, {len(keys)} unique keys, {states_with_city} with a city colour")
    print(f"OK: x[{min(r[3] for r in rows):.0f}..{max(r[3] for r in rows):.0f}] "
          f"z[{min(r[5] for r in rows):.0f}..{max(r[5] for r in rows):.0f}]")
    print(f"OK: height field {w}x{h}, {len(raw)} bytes")
    print(f"OK: province graph {pn} nodes, {pe} edges, {len(p)} bytes")
    return 0


if __name__ == "__main__":
    sys.exit(main())
