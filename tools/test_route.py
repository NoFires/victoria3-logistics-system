"""Offline check of the province graph router; draws routes on a downscaled map."""
import argparse
import heapq
import math
import re
import struct
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw

Image.MAX_IMAGE_PIXELS = None


def load_graph(path: Path):
    d = path.read_bytes()
    magic, ver, n, e = struct.unpack_from("<IIII", d, 0)
    assert magic == 0x47505249 and ver == 1, "bad provinces.bin"
    nodes = []
    off = 16
    for _ in range(n):
        color, x, z, h, first, cnt, tid, flags = struct.unpack_from("<IfffIIII", d, off)
        nodes.append((color, x, z, h, first, cnt, tid, flags))
        off += 32
    edges = np.frombuffer(d, dtype="<u4", offset=off, count=e)
    return nodes, edges


def load_states(root: Path):
    out = {}
    for p in (root / "game/map_data/state_regions").glob("*.txt"):
        text = p.read_text(encoding="utf-8-sig", errors="replace")
        for m in re.finditer(r"^([A-Z][A-Z0-9_]+)\s*=\s*\{", text, re.M):
            key = m.group(1)
            i = text.find("{", m.start())
            depth = 0
            end = None
            for j in range(i, len(text)):
                if text[j] == "{":
                    depth += 1
                elif text[j] == "}":
                    depth -= 1
                    if depth == 0:
                        end = j
                        break
            body = text[i + 1:end]
            cm = re.search(r'city\s*=\s*"x([0-9A-Fa-f]{6})"', body)
            out[key] = int(cm.group(1), 16) if cm else None
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--game-root", required=True, type=Path)
    ap.add_argument("--data", type=Path, default=Path(__file__).resolve().parents[1] / "data")
    ap.add_argument("--out", type=Path, default=Path(__file__).resolve().parents[1] / "native/build/diagnostics/route_test.png")
    ap.add_argument("--pairs", default="STATE_GOTLAND:STATE_SCANIA,STATE_SVEALAND:STATE_ILE_DE_FRANCE,STATE_BRANDENBURG:STATE_LOMBARDY")
    args = ap.parse_args()

    nodes, edges = load_graph(args.data / "provinces.bin")
    states = load_states(args.game_root)
    index = {nd[0]: i for i, nd in enumerate(nodes)}
    factors = {}
    for line in (args.data / "terrain_names.txt").read_text().splitlines():
        i, name, f = line.split("\t")
        factors[int(i)] = float(f)

    def land(i):
        return nodes[i][7] & 1

    def astar(s, t):
        def h(i):
            return math.hypot(nodes[i][1] - nodes[t][1], nodes[i][2] - nodes[t][2])
        came = {s: -1}
        gscore = {s: 0.0}
        closed = set()
        openq = [(h(s), 0.0, s)]
        while openq:
            _, g, u = heapq.heappop(openq)
            if u in closed:
                continue
            if u == t:
                path = []
                while u != -1:
                    path.append(u)
                    u = came[u]
                return path[::-1]
            closed.add(u)
            first, cnt = nodes[u][4], nodes[u][5]
            for k in range(first, first + cnt):
                v = int(edges[k])
                if v in closed or not land(v):
                    continue
                du = math.hypot(nodes[u][1] - nodes[v][1], nodes[u][2] - nodes[v][2])
                factor = 0.5 * (factors.get(nodes[u][6], 1.0) + factors.get(nodes[v][6], 1.0))
                ng = g + du * factor
                if ng < gscore.get(v, 1e30) - 1e-6:
                    gscore[v] = ng
                    came[v] = u
                    heapq.heappush(openq, (ng + h(v), ng, v))
        return None

    base = Image.open(args.game_root / "game/map_data/provinces.png").convert("RGB")
    scale = 1024 / base.width
    small = base.resize((1024, int(base.height * scale)))
    draw = ImageDraw.Draw(small)

    for pair in args.pairs.split(","):
        a, b = pair.split(":")
        ca, cb = states.get(a), states.get(b)
        if ca not in index or cb not in index:
            print(pair, "missing city", ca in index, cb in index)
            continue
        path = astar(index[ca], index[cb])
        if not path:
            print(pair, "NO PATH")
            continue
        pts = [(nodes[i][1] * scale, nodes[i][2] * scale) for i in path]
        draw.line(pts, fill=(0, 255, 0), width=2)
        print(pair, "nodes", len(path), "start", pts[0], "end", pts[-1])
    out = args.out
    out.parent.mkdir(parents=True, exist_ok=True)
    small.save(out)
    print("wrote", out)


if __name__ == "__main__":
    main()
