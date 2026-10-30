"""Exercise exact-build native selection bytes in an isolated synthetic world.

Does not load/launch/attach to Victoria 3. Only the three read-only functions and
their valid-handle lookup are executable; unrelated reserved pages stay absent.
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
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    machine, sections = struct.unpack_from('<HH', data, pe + 4)
    optional_size = struct.unpack_from('<H', data, pe + 20)[0]
    assert machine == 0x8664
    table = pe + 24 + optional_size

    def code(rva, size):
        for i in range(sections):
            at = table + i * 40
            va, raw_size, raw = struct.unpack_from('<III', data, at + 12)
            if va <= rva and rva + size <= va + raw_size:
                return data[raw + rva - va:raw + rva - va + size]
        raise AssertionError('native code outside file sections')

    kernel = c.WinDLL('kernel32', use_last_error=True)
    kernel.VirtualAlloc.argtypes = [c.c_void_p, c.c_size_t, c.c_uint32, c.c_uint32]
    kernel.VirtualAlloc.restype = c.c_void_p
    kernel.VirtualProtect.argtypes = [c.c_void_p, c.c_size_t, c.c_uint32, c.POINTER(c.c_uint32)]
    kernel.VirtualProtect.restype = c.c_int
    kernel.VirtualFree.argtypes = [c.c_void_p, c.c_size_t, c.c_uint32]
    kernel.FlushInstructionCache.argtypes = [c.c_void_p, c.c_void_p, c.c_size_t]
    kernel.GetCurrentProcess.restype = c.c_void_p
    base = kernel.VirtualAlloc(None, 0x6000000, 0x2000, 1)
    assert base, 'reserve native address range'

    def commit(rva, size):
        assert kernel.VirtualAlloc(base + rva, size, 0x1000, 4) == base + rva

    def put(address, fmt, *values):
        raw = struct.pack('<' + fmt, *values)
        c.memmove(address, raw, len(raw))

    try:
        for rva, start, size in [(0xcf6000, 0xcf6b70, 0xa8), (0x13bd000, 0x13bd840, 0x42a)]:
            commit(rva, 0x1000)
            c.memmove(base + start, code(start, size), size)
            previous = c.c_uint32()
            assert kernel.VirtualProtect(base + rva, 0x1000, 0x20, c.byref(previous))
            assert kernel.FlushInstructionCache(kernel.GetCurrentProcess(), base + rva, 0x1000)
        for rva in [0x58ff000, 0x5966000]:
            commit(rva, 0x1000)
        constants = {0x58ffb68: 100000000, 0x58ffbb0: 25000, 0x58ffb70: 5000,
                     0x58ff580: 0, 0x58ff5b0: 5000, 0x58ff5a8: 10000, 0x58ff5a0: 20000}
        for rva, value in constants.items():
            put(base + rva, 'q', value)
        database = c.create_string_buffer(0x40)
        entries = c.create_string_buffer(32)
        objects = [c.create_string_buffer(0xb0) for _ in range(2)]
        put(base + 0x59667f8, 'Q', c.addressof(database))
        put(c.addressof(database) + 0x20, 'Q', c.addressof(entries))
        put(c.addressof(database) + 0x2c, 'I', 2)
        ids = [0, 0x7f000001]
        for i, obj in enumerate(objects):
            put(c.addressof(entries) + i * 16 + 8, 'Q', c.addressof(obj))
            put(c.addressof(obj) + 8, 'I', ids[i])
        select = c.WINFUNCTYPE(c.c_void_p, c.c_void_p)(base + 0x13bdbe0)
        cases = 0

        def exercise(distances, passages, modes, expected_index):
            nonlocal cases
            paths = c.create_string_buffer(len(distances) * 0x38)
            vector = c.create_string_buffer(0x18)
            lists = []
            for obj, tariff in zip(objects, modes):
                put(c.addressof(obj) + 0xac, 'BB', *tariff)
            for i, (distance, route) in enumerate(zip(distances, passages)):
                item = c.addressof(paths) + i * 0x38
                put(item + 0x30, 'q', distance)
                if route:
                    handles = (c.c_uint32 * len(route))(*(ids[n] for n in route))
                    lists.append(handles)
                    put(item + 0x18, 'QII', c.addressof(handles), len(route), len(route))
            put(c.addressof(vector), 'QII', c.addressof(paths), len(distances), len(distances))
            watched = [database, entries, *objects, paths, vector, *lists]
            before = [bytes(value) for value in watched]
            constant_before = [c.string_at(base + rva, 8) for rva in constants]
            chosen = select(c.addressof(vector))
            assert chosen == c.addressof(paths) + expected_index * 0x38, (distances, passages, modes, expected_index)
            assert [bytes(value) for value in watched] == before, 'selector modified borrowed input'
            assert [c.string_at(base + rva, 8) for rva in constants] == constant_before, 'selector modified costs'
            cases += 1

        # Direct return, last/first winners and native stable tie order.
        exercise([100000000], [[1]], [(3, 3), (3, 3)], 0)
        exercise([400000000, 100000000, 200000000], [[], [], []], [(0, 0), (0, 0)], 1)
        exercise([100000000, 100000000, 100000000], [[], [], []], [(0, 0), (0, 0)], 0)
        # Sweep both directions of both straits. Uses original instructions,
        # with an independent small-integer oracle only for these test inputs.
        tariffs = [0, 5000, 10000, 20000]
        for a in range(4):
            for b in range(4):
                modes = [(a, b), (b, a)]
                costs = [100000 + (tariffs[a] + tariffs[b]) * 5,
                         175000, 125000 + (tariffs[a] + tariffs[b]) * 5]
                expected_index = min(range(3), key=lambda i: costs[i])
                exercise([100000000, 400000000, 200000000], [[0], [], [1]], modes, expected_index)
        # Changing the same current strait can reverse the selected native path.
        exercise([100000000, 400000000], [[1], []], [(0, 0), (0, 0)], 0)
        exercise([100000000, 400000000], [[1], []], [(0, 0), (3, 3)], 1)
        print(f'PASS: native selector ABI, {cases} cases, current tariffs, full handles, stable ties and read-only inputs; sha256={expected}')
    finally:
        kernel.VirtualFree(base, 0, 0x8000)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe', type=Path, default=Path(r'C:\Program Files (x86)\Steam\steamapps\common\Victoria 3\binaries\victoria3.exe'))
    run(parser.parse_args().exe)
