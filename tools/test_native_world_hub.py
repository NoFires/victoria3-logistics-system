"""Exact 1.15 native hub getter in a synthetic process-local world.

No executable loading, game process, attach or game memory writes. DA2500,
D9F9C0 and both typed lookups execute original bytes; only distance scoring is
an explicit fixture service. This does not prove the actual game's lease or
its distance/basin/cold scoring.
"""
import argparse
import ctypes as c
import hashlib
from pathlib import Path
import re
import struct


def run(exe):
    data = exe.read_bytes()
    profile = (Path(__file__).resolve().parents[1] / 'native/src/offsets.h').read_text('utf-8')
    expected = re.search(r'kExpectedExeSha256\s*=\s*"([0-9a-f]{64})"', profile).group(1)
    assert hashlib.sha256(data).hexdigest() == expected, 'unsupported executable; no native bytes executed'
    getter = int(re.search(r'kRvaWorldMarketHub\s*=\s*(0x[0-9a-fA-F]+)', profile).group(1), 16)
    assert getter == 0xda2500
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    machine, sections = struct.unpack_from('<HH', data, pe + 4)
    assert machine == 0x8664
    table = pe + 24 + struct.unpack_from('<H', data, pe + 20)[0]

    def code(rva, size):
        for i in range(sections):
            va, raw_size, raw = struct.unpack_from('<III', data, table + i * 40 + 12)
            if va <= rva and rva + size <= va + raw_size:
                return data[raw + rva - va:raw + rva - va + size]
        raise AssertionError('native code outside file sections')

    assert code(getter, 8) == bytes.fromhex('48895c241048896c')
    assert code(0x67b920, 3) == bytes.fromhex('b001c3')
    assert code(0x6745c0, 3) == bytes.fromhex('32c0c3')
    kernel = c.WinDLL('kernel32', use_last_error=True)
    kernel.VirtualAlloc.argtypes = [c.c_void_p, c.c_size_t, c.c_uint32, c.c_uint32]
    kernel.VirtualAlloc.restype = c.c_void_p
    kernel.VirtualProtect.argtypes = [c.c_void_p, c.c_size_t, c.c_uint32, c.POINTER(c.c_uint32)]
    kernel.VirtualFree.argtypes = [c.c_void_p, c.c_size_t, c.c_uint32]
    kernel.GetCurrentProcess.restype = c.c_void_p
    kernel.FlushInstructionCache.argtypes = [c.c_void_p, c.c_void_p, c.c_size_t]
    base = kernel.VirtualAlloc(None, 0x6000000, 0x2000, 1)
    assert base
    pages = set()

    def commit(rva, size):
        for page in range(rva & ~4095, (rva + size + 4095) & ~4095, 4096):
            if page not in pages:
                assert kernel.VirtualAlloc(base + page, 4096, 0x1000, 4) == base + page
                pages.add(page)

    def put(rva, fmt, *values):
        raw = struct.pack(fmt, *values)
        commit(rva, len(raw))
        c.memmove(base + rva, raw, len(raw))

    functions = [(getter, 0xd0), (0xd9f9c0, 0xa9), (0xd16d80, 0xb2),
                 (0x72ca60, 0xb2), (0x67b920, 3), (0x6745c0, 3)]
    code_pages = set()
    try:
        for rva, size in functions:
            commit(rva, size)
            c.memmove(base + rva, code(rva, size), size)
            code_pages.update(range(rva & ~4095, (rva + size + 4095) & ~4095, 4096))
        fixture = 0x5e00000
        commit(fixture, 0x40000)
        area_db, area_entries, area = fixture, fixture + 0x100, fixture + 0x200
        state_db, state_entries = fixture + 0x400, fixture + 0x500
        true_vt, false_vt = fixture + 0x600, fixture + 0x620
        null_area, null_state, hubs = fixture + 0x700, fixture + 0x800, fixture + 0x900
        objects = {0: fixture + 0x1000, 0xab000002: fixture + 0x3000, 0x7f000003: fixture + 0x5000}
        source = objects[0]
        put(0x5c44e00, '<Q', base + area_db)
        put(0x5bd69c8, '<Q', base + state_db)
        put(0x5bd6158, '<Q', base + null_area)
        put(0x5c3fdf8, '<Q', base + null_state)
        put(area_db + 0x20, '<Q', base + area_entries)
        put(area_db + 0x2c, '<I', 1)
        put(area_entries + 4, '<IQ', 0, base + area)
        put(state_db + 0x20, '<Q', base + state_entries)
        put(state_db + 0x2c, '<I', 4)
        put(true_vt + 8, '<Q', base + 0x67b920)
        put(false_vt + 8, '<Q', base + 0x6745c0)
        put(null_state, '<Q', base + false_vt)
        put(null_state + 8, '<I', 0xffffffff)
        for identity, obj in objects.items():
            put(state_entries + (identity & 0xffffff) * 16 + 4, '<IQ', identity & 0xff000000, base + obj)
            put(obj, '<Q', base + true_vt)
            put(obj + 8, '<I', identity)
            put(obj + 0xb78, '<I', 0)
        put(area + 0x60, '<QII', base + hubs, 16, 0)
        metric_calls, costs = [], {}
        Metric = c.WINFUNCTYPE(c.c_void_p, c.c_void_p, c.c_void_p, c.c_void_p)

        @Metric
        def metric(out, current_source, candidate):
            assert current_source == base + source
            metric_calls.append(candidate - base)
            c.memmove(out, struct.pack('<q', costs[candidate - base]), 8)
            return out

        stub = b'\x48\xb8' + struct.pack('<Q', c.cast(metric, c.c_void_p).value) + b'\xff\xe0'
        commit(0x10219a0, len(stub))
        c.memmove(base + 0x10219a0, stub, len(stub))
        code_pages.add(0x1021000)
        for page in code_pages:
            old = c.c_uint32()
            assert kernel.VirtualProtect(base + page, 4096, 0x20, c.byref(old))
        assert kernel.FlushInstructionCache(kernel.GetCurrentProcess(), base, 0x6000000)
        Native = c.WINFUNCTYPE(c.c_void_p, c.c_void_p, c.c_void_p)
        native = Native(base + getter)
        count = 0

        def check(active, selected, nearest, distances, expected_hub, expected_calls, *, area_id=0, stale_generation=False):
            nonlocal count
            metric_calls.clear()
            costs.clear()
            costs.update({objects[k]: value for k, value in distances.items()})
            put(area + 0x6c, '<I', len(active))
            if active:
                put(hubs, '<' + 'I' * len(active), *active)
            put(source + 0xb78, '<I', area_id)
            put(source + 0xf48, '<I', selected)
            put(source + 0xb7c, '<I', nearest)
            put(state_entries + 2 * 16 + 4, '<I', 0xac000000 if stale_generation else 0xab000000)
            before = c.string_at(base + fixture, 0x40000)
            result = c.c_uint32(0xdeadbeef)
            assert native(base + source, c.byref(result)) == c.addressof(result), 'native output ownership'
            assert result.value == expected_hub, (count, hex(result.value), hex(expected_hub))
            assert metric_calls == [objects[k] for k in expected_calls], 'native candidate order or cache fast branch'
            assert c.string_at(base + fixture, 0x40000) == before, 'native getter modified borrowed inputs'
            count += 1

        a, b, d, missing = 0, 0xab000002, 0x7f000003, 0xffffffff
        check([a, b, d], b, a, {}, b, [])
        check([a, b], missing, b, {}, b, [])
        check([a, b], 0xac000002, a, {}, a, [])
        check([a, b, d], missing, missing, {a: 300, b: 100, d: 200}, b, [a, b, d])
        check([a, b, d], missing, missing, {a: 300, b: 200, d: 100}, d, [a, b, d])
        check([b, a], missing, missing, {a: 100, b: 100}, b, [b, a])
        check([a, b], missing, missing, {a: 100, b: 100}, a, [a, b])
        check([a], missing, missing, {a: 0x7fffffffffffffff}, a, [a])
        check([], a, b, {}, missing, [])
        check([b], missing, missing, {}, missing, [], stale_generation=True)
        check([b, d], missing, missing, {d: 200}, d, [d], stale_generation=True)
        check([b, d], missing, missing, {}, missing, [], area_id=missing)
        print(f'PASS: original current world hub getter ABI, {count} cases, active cache precedence, native fallback/ties/full handles and read-only inputs (distance fixture); sha256={expected}')
    finally:
        assert kernel.VirtualFree(base, 0, 0x8000)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--exe', type=Path, default=Path(r'C:\Program Files (x86)\Steam\steamapps\common\Victoria 3\binaries\victoria3.exe'))
    run(parser.parse_args().exe)
