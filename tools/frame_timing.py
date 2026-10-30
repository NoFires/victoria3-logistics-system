"""Analyze opt-in FRAME PERF raw QPC intervals (Present entry, not GPU time)."""
import argparse
import json
import math
from pathlib import Path
import re

ROW = re.compile(r'FRAME PERF thread=(\d+) world=(\d+) swap=(\S+) frequency=(\d+) begin=(\d+) end=(\d+) count=(\d+) ticks=([0-9,]+)$')

def read_intervals(text):
    intervals = []
    for line in text.splitlines():
        if 'FRAME PERF thread=' not in line:
            continue
        match = ROW.search(line)
        if not match:
            raise ValueError('Malformed or truncated frame timing batch')
        thread, world, swap, frequency, begin, end, count, raw = match.groups()
        thread, world, frequency, begin, end, count = map(int, (thread, world, frequency, begin, end, count))
        ticks = [int(n) for n in raw.split(',')]
        if not thread or frequency <= 0 or begin <= 0 or count != 64 or len(ticks) != count or any(n <= 0 for n in ticks) or sum(ticks) != end - begin:
            raise ValueError('Invalid frame timing identity, count, clock or duration')
        cursor = begin
        for tick in ticks:
            intervals.append((cursor, cursor + tick, frequency, thread, world, swap))
            cursor += tick
    return intervals

def summarize(intervals, start=None, end=None, minimum_seconds=0):
    selected = [r for r in intervals if (start is None or r[0] >= start) and (end is None or r[1] <= end)]
    if len(selected) < 2:
        raise ValueError('Insufficient complete frame intervals')
    identity = selected[0][2:]
    if any(r[2:] != identity for r in selected):
        raise ValueError('Mixed frequency/thread/world/swap identities in sample')
    if any(a[1] != b[0] for a, b in zip(selected, selected[1:])):
        raise ValueError('Frame sample has missing, duplicate or reordered intervals')
    frequency = selected[0][2]
    seconds = (selected[-1][1] - selected[0][0]) / frequency
    if seconds < minimum_seconds:
        raise ValueError('Complete contiguous frame sample is shorter than required')
    values = sorted((r[1] - r[0]) * 1000 / frequency for r in selected)
    def percentile(p):
        return values[math.ceil(len(values) * p) - 1]
    return dict(metric='Present-entry interval; not GPU execution or display latency', percentile='nearest rank',
                frames=len(values), seconds=seconds, begin_qpc=selected[0][0], end_qpc=selected[-1][1],
                frequency=frequency, thread=identity[1], world=identity[2], swap=identity[3],
                mean_ms=sum(values) / len(values), p50_ms=percentile(.5), p95_ms=percentile(.95),
                p99_ms=percentile(.99), max_ms=values[-1])

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--log', type=Path, required=True)
    parser.add_argument('--start-qpc', type=int)
    parser.add_argument('--end-qpc', type=int)
    parser.add_argument('--require-seconds', type=float, default=0)
    args = parser.parse_args()
    print(json.dumps(summarize(read_intervals(args.log.read_text(encoding='utf-8')), args.start_qpc, args.end_qpc, args.require_seconds), indent=2))

if __name__ == '__main__':
    main()
