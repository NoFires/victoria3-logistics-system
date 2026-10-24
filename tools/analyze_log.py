"""Summarise an Ironroad log without printing raw pointers/addresses.

Usage:
  python tools/analyze_log.py                 # default installed freight log
  python tools/analyze_log.py --log path.log
"""
import argparse
import os
import re
from collections import Counter
from pathlib import Path


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--log", type=Path,
                    default=Path(os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)")) / "Steam/steamapps/common/Victoria 3/binaries/freight/freight.log")
    ap.add_argument("--all-runs", action="store_true", help="include earlier processes in an appended log")
    ap.add_argument("--game-errors", type=Path,
                    default=Path(os.environ.get("USERPROFILE", "")) / "Documents/Paradox Interactive/Victoria 3/logs/error.log",
                    help="game shader/resource diagnostics (separate from the native loader log)")
    args = ap.parse_args()
    if not args.log.is_file():
        print("no log at", args.log)
        return 2
    lines = args.log.read_text(encoding="utf-8", errors="replace").splitlines()
    starts = [i for i, line in enumerate(lines) if line.startswith('=== Ironroad')]
    if starts and not args.all_runs:
        lines = lines[starts[-1]:]
    print(f"log: {args.log} ({len(lines)} lines)")
    print('scope:', 'all recorded processes' if args.all_runs else 'latest process only')

    def safe(text):
        return re.sub(r'\b[0-9a-fA-F]{16}\b', '<pointer>', text)

    counters = Counter()
    warnings, errors = [], []
    type_lines, pass_lines, route_lines, captures, submits = [], [], [], [], []
    for line in lines:
        m = re.match(r"\[(\d+)\] (\w+) (.*)", line)
        level = m.group(2) if m else "?"
        text = m.group(3) if m else line
        counters[level] += 1
        if level == "ERR":
            errors.append(text)
        elif level == "WARN":
            warnings.append(text)
        if "resolved arrow type" in text:
            type_lines.append(text)
        if "arrow pass #" in text:
            pass_lines.append(text)
        if "route " in text and "built" in text:
            route_lines.append(text)
        if text.startswith("CAPTURE setpath"):
            captures.append(text)
        if text.startswith('SUBMIT scope='):
            submits.append(text)

    print("\ncounts by level:", dict(counters))
    print("arrow type resolutions:", len(type_lines))
    for t in type_lines[:3]:
        print("  ", safe(t))
    print("arrow pass entries:", len(pass_lines))
    for t in pass_lines[:12]:
        print("  ", safe(t))
    print('eligible native scopes:', sorted(set(re.findall(r'scope=([\d/-]+).*eligible=1', '\n'.join(pass_lines)))))
    print('submission summaries:', len(submits))
    if submits:
        print('  latest:', submits[-1])
    print("route builds:", len(route_lines))
    for t in route_lines[:5]:
        print("  ", safe(t))
    print("native SetPath captures:", len(captures))
    if warnings:
        print("\nwarnings:")
        for t in warnings[:12]:
            print("  ", safe(t))
    if errors:
        print("\nerrors:")
        for t in errors[:12]:
            print("  ", safe(t))
    unresolved = Counter(re.findall(r"arrow type '([^']+)' unresolved", '\n'.join(lines)))
    if unresolved:
        print('\nunresolved resource types:', dict(unresolved))
    if args.game_errors.is_file():
        game_lines = args.game_errors.read_text(encoding='utf-8', errors='replace').splitlines()
        matches = [i for i, line in enumerate(game_lines) if 'ironroad' in line.lower()]
        print(f'\ngame resource/shader log: {len(matches)} Ironroad references ({args.game_errors})')
        included = set()
        for index in matches[:12]:
            for i in range(max(0, index-1), min(len(game_lines), index+4)):
                if i not in included:
                    print('  ', safe(game_lines[i]))
                    included.add(i)
        print('  Separate game log; absence of errors alone does not prove visible rendering.')
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
