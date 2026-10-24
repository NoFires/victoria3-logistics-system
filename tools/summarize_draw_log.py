"""Summarize DRAW PERF records; never echo raw log lines or binary locations."""
import argparse
import re
from pathlib import Path

PATTERN = re.compile(r'DRAW PERF paths=(\d+) uploads=(\d+) reused=(\d+) failed=(\d+) pool=(\d+) cpu_us=(\d+) repeats_skipped=(\d+)')


def summarize(text, pid=None):
    rows = []
    for line in text.splitlines():
        if pid is not None and not re.search(r'\bpid=%d\b' % pid, line):
            continue
        match = PATTERN.search(line)
        if match:
            rows.append(tuple(map(int, match.groups())))
    if not rows:
        return {'batches': 0}
    return {
        'batches': len(rows),
        'geometry_uploads': sum(row[1] for row in rows),
        'cache_reuses': sum(row[2] for row in rows),
        'zero_upload_batches': sum(row[1] == 0 for row in rows),
        'failed_paths': sum(row[3] for row in rows),
        'peak_pool_objects': max(row[4] for row in rows),
        'total_batch_cpu_ms': round(sum(row[5] for row in rows) / 1000, 3),
        'largest_batch_cpu_ms': round(max(row[5] for row in rows) / 1000, 3),
        'latest_repeat_counter': rows[-1][6],
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--log', type=Path, required=True)
    parser.add_argument('--pid', type=int)
    parser.add_argument('--tail-mib', type=int, default=16)
    args = parser.parse_args()
    if args.tail_mib < 1:
        parser.error('tail-mib must be positive')
    with args.log.open('rb') as stream:
        stream.seek(0, 2)
        stream.seek(max(0, stream.tell() - args.tail_mib * 1024 * 1024))
        text = stream.read().decode('utf-8', errors='replace')
    for name, value in summarize(text, args.pid).items():
        print('%s: %s' % (name, value))


if __name__ == '__main__':
    main()
