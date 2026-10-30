from frame_timing import read_intervals, summarize

def row(begin=1000, ticks=None, world=1, frequency=1000):
    ticks = ticks or list(range(1, 65))
    return f'FRAME PERF thread=7 world={world} swap=00000001 frequency={frequency} begin={begin} end={begin+sum(ticks)} count=64 ticks='+','.join(map(str,ticks))

def rejected(action):
    try:
        action()
    except ValueError:
        return
    raise AssertionError('Invalid frame sample accepted')

first = row()
values = read_intervals(first)
report = summarize(values)
assert report['frames'] == 64 and report['seconds'] == 2.080
assert report['p50_ms'] == 32 and report['p95_ms'] == 61 and report['p99_ms'] == 64
assert summarize(values, start=1001, end=3080)['frames'] == 63
second = row(begin=3080)
assert summarize(read_intervals(first+'\n'+second))['frames'] == 128
for invalid in [first[:-1], first.replace('count=64','count=63'), first.replace('end=3080','end=3081'),
                first.replace('frequency=1000','frequency=0'), first.replace('thread=7','thread=0'),
                first.replace('ticks=1,','ticks=0,')]:
    rejected(lambda: read_intervals(invalid))
rejected(lambda: summarize([]))
rejected(lambda: summarize(values, minimum_seconds=60))
rejected(lambda: summarize(read_intervals(first+'\n'+row(begin=3081))))
rejected(lambda: summarize(read_intervals(first+'\n'+first)))
rejected(lambda: summarize(read_intervals(first+'\n'+row(begin=3080,world=2))))
rejected(lambda: summarize(read_intervals(first+'\n'+row(begin=3080,frequency=2000))))
assert not read_intervals('FRAME PERF enabled frequency=1000 batch=64 metric=Present-entry-interval')
print('PASS: frame timing parser, exact quantiles, complete ranges, duration/identity/gap/truncation rejection')
