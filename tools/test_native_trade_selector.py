"""Exercise exact-build native selection bytes in an isolated synthetic world.

Does not load/launch/attach to Victoria 3. The current route getter, cost and seeded-choice functions
and valid-handle lookup execute unchanged. Province/owner services use explicit
fixture stubs; this does not prove a game access lease. Other unrelated reserved pages stay absent.
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
        for rva, start, size in [(0xb04000, 0xb04970, 0xb2), (0xcfb000, 0xcfb310, 0x8fb),
                                 (0xd03000, 0xd039c0, 0x2a8)]:
            commit(rva, 0x1000)
            c.memmove(base + start, code(start, size), size)
            previous = c.c_uint32()
            assert kernel.VirtualProtect(base + rva, 0x1000, 0x20, c.byref(previous))
            assert kernel.FlushInstructionCache(kernel.GetCurrentProcess(), base + rva, 0x1000)
        # Fixture services return the passed province token and a valid owner.
        for rva, stub in [(0xc80000, bytes.fromhex('4889c8c3')),
                          (0xf1e000, bytes.fromhex('c701000000004889c8c3'))]:
            commit(rva, 0x1000)
            target = rva + (0xcc0 if rva == 0xc80000 else 0x630)
            c.memmove(base + target, stub, len(stub))
            previous = c.c_uint32()
            assert kernel.VirtualProtect(base + rva, 0x1000, 0x20, c.byref(previous))
            assert kernel.FlushInstructionCache(kernel.GetCurrentProcess(), base + rva, 0x1000)
        for rva in [0x5bd7000, 0x5bdc000, 0x5c43000]:
            commit(rva, 0x1000)
        constants = {0x5bdc788: 100000000, 0x5bdc7b0: 25000, 0x5bdc780: 5000,
                     0x5bdc7a0: 0, 0x5bd7910: 0, 0x5bd7920: 5000,
                     0x5bd7938: 10000, 0x5bd7940: 20000}
        for rva, value in constants.items():
            put(base + rva, 'q', value)
        database = c.create_string_buffer(0x40)
        entries = c.create_string_buffer(32)
        objects = [c.create_string_buffer(0x130) for _ in range(2)]
        put(base + 0x5c43138, 'Q', c.addressof(database))
        put(c.addressof(database) + 0x20, 'Q', c.addressof(entries))
        put(c.addressof(database) + 0x2c, 'I', 2)
        ids = [0, 0x7f000001]
        for i, obj in enumerate(objects):
            put(c.addressof(entries) + i * 16 + 8, 'Q', c.addressof(obj))
            put(c.addressof(obj) + 8, 'I', ids[i])
            put(c.addressof(entries) + i * 16 + 4, 'I', ids[i] & 0xff000000)
        definitions = [c.create_string_buffer(0x60) for _ in objects]
        for obj, definition in zip(objects, definitions):
            put(c.addressof(obj) + 0x120, 'Q', c.addressof(definition))
            put(c.addressof(definition) + 0x40, 'HH', 1, 2)
        select = c.WINFUNCTYPE(c.c_void_p, c.c_void_p, c.c_uint32, c.c_uint32)(base + 0xd039c0)

        def hash_id(value):
            result = 0x811c9dc5
            for byte in struct.pack('<I', value):
                result = ((result ^ byte) * 0x1000193) & 0xffffffff
            return result

        cases = 0

        def exercise(distances, passages, modes, expected_index, endpoints=(0, 0xab000801), jitter=0):
            nonlocal cases
            paths = c.create_string_buffer(len(distances) * 0x38)
            vector = c.create_string_buffer(0x18)
            put(base + 0x5bdc7a0, 'q', jitter)
            market = c.create_string_buffer(0x320)
            groups = c.create_string_buffer(4 * 0x18)
            group = c.create_string_buffer(0x48)
            destinations = c.create_string_buffer(4 * 0x28)
            source, destination = endpoints
            put(c.addressof(market) + 0x300, 'Q', c.addressof(groups))
            put(c.addressof(market) + 0x30c, 'I', 1)
            at = c.addressof(groups) + (hash_id(source) & 1) * 0x18
            put(at + 4, 'B', 1)
            put(at + 8, 'I', source)
            put(at + 0x10, 'Q', c.addressof(group))
            put(c.addressof(group) + 0x28, 'Q', c.addressof(destinations))
            put(c.addressof(group) + 0x34, 'I', 1)
            at = c.addressof(destinations) + (hash_id(destination) & 1) * 0x28
            put(at + 4, 'B', 1)
            put(at + 8, 'I', destination)
            lists = []
            for obj, tariff in zip(objects, modes):
                put(c.addressof(obj) + 0x12c, 'BB', *tariff)
            for i, (distance, route) in enumerate(zip(distances, passages)):
                item = c.addressof(paths) + i * 0x38
                put(item + 0x30, 'q', distance)
                if route:
                    handles = (c.c_uint32 * len(route))(*(ids[n] for n in route))
                    lists.append(handles)
                    put(item + 0x18, 'QII', c.addressof(handles), len(route), len(route))
            put(c.addressof(vector), 'QII', c.addressof(paths), len(distances), len(distances))
            put(at + 0x10, 'QII', c.addressof(paths), len(distances), len(distances))
            watched = [database, entries, *objects, *definitions, market, groups, group, destinations, paths, vector, *lists]
            before = [bytes(value) for value in watched]
            constant_before = [c.string_at(base + rva, 8) for rva in constants]
            chosen = select(c.addressof(market), source, destination)
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
        # Independent oracle for the newly added endpoint-seeded tie jitter.
        for source, destination in [(0, 1), (0xab000801, 0x12000002), (1, 0)]:
            a, b = hash_id(source), hash_id(destination)
            seed = a ^ ((b + 0x9e3779b9 + (a << 6) + (a >> 2)) & 0xffffffff)
            routes = [[0], [1], []]
            factors = []
            for route in routes:
                value = seed
                for index in route:
                    a, b = hash_id(value), hash_id(ids[index])
                    value = a ^ ((b + 0x9e3779b9 + (a << 6) + (a >> 2)) & 0xffffffff)
                factors.append(100000 + (value % 1001) * 200 - 100000)
            exercise([100000000] * 3, routes, [(0, 0)] * 2,
                     min(range(3), key=lambda i: factors[i]), (source, destination), 100000)
        print(f'PASS: native selector ABI, {cases} cases, current tariffs, full endpoint handles, seeded choice and read-only inputs (province/owner fixture services); sha256={expected}')
    finally:
        kernel.VirtualFree(base, 0, 0x8000)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe', type=Path, default=Path(r'C:\Program Files (x86)\Steam\steamapps\common\Victoria 3\binaries\victoria3.exe'))
    run(parser.parse_args().exe)
