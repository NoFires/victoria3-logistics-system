"""Execute current native comparator/registration/sort bytes outside the game.

Allocation, map insertion, hashing and numeric getters are explicit fixtures;
this verifies ABI/ownership and native ordering, not a live GUI access window.
"""
import argparse
import ctypes as c
import hashlib
from pathlib import Path
import re
import struct
from ironpe import PE


def run(exe):
    root = Path(__file__).resolve().parents[1]
    expected = re.search(r'kExpectedExeSha256\s*=\s*"([0-9a-f]{64})"',
                         (root / 'native/src/offsets.h').read_text('utf-8')).group(1)
    assert hashlib.sha256(exe.read_bytes()).hexdigest() == expected, 'unsupported native sort executable'
    pe = PE(exe)
    kernel = c.WinDLL('kernel32', use_last_error=True)
    kernel.VirtualAlloc.argtypes = [c.c_void_p, c.c_size_t, c.c_uint32, c.c_uint32]
    kernel.VirtualAlloc.restype = c.c_void_p
    kernel.VirtualProtect.argtypes = [c.c_void_p, c.c_size_t, c.c_uint32, c.POINTER(c.c_uint32)]
    kernel.VirtualProtect.restype = c.c_int
    kernel.VirtualFree.argtypes = [c.c_void_p, c.c_size_t, c.c_uint32]
    kernel.FlushInstructionCache.argtypes = [c.c_void_p, c.c_void_p, c.c_size_t]
    kernel.GetCurrentProcess.restype = c.c_void_p
    base = kernel.VirtualAlloc(None, 0x4900000, 0x2000, 1)
    assert base
    pages = set()
    callbacks, allocations, freed, errors, registry = [], {}, set(), [], {}
    quantities = {}
    direction = c.c_ubyte()
    refresh_events = []

    def put(address, fmt, *values):
        data = struct.pack('<' + fmt, *values)
        c.memmove(address, data, len(data))

    def get(address, fmt='Q'):
        return struct.unpack('<' + fmt, c.string_at(address, struct.calcsize('<' + fmt)))[0]

    def commit(rva, size):
        for page in range(rva & ~0xfff, (rva + size + 0xfff) & ~0xfff, 0x1000):
            if page not in pages:
                assert kernel.VirtualAlloc(base + page, 0x1000, 0x1000, 4) == base + page
                pages.add(page)

    def native(rva, size=None):
        if size is None:
            chunk = pe.chunk_of(rva)
            assert chunk and chunk[0] == rva
            size = chunk[1] - rva
        commit(rva, size)
        c.memmove(base + rva, pe.rd(rva, size), size)

    def stub(rva, prototype, callback):
        wrapped = prototype(callback)
        callbacks.append(wrapped)
        raw = b'\x48\xb8' + struct.pack('<Q', c.cast(wrapped, c.c_void_p).value) + b'\xff\xe0'
        commit(rva, len(raw))
        c.memmove(base + rva, raw, len(raw))

    def allocate(size):
        if size != 0x50:
            errors.append(('unexpected allocation', size))
        buffer = c.create_string_buffer(max(size, 0x50))
        address = c.addressof(buffer)
        allocations[address] = buffer
        return address

    def release(address):
        if address not in allocations or address in freed:
            errors.append(('invalid release', address))
        freed.add(address)

    clone_type = c.WINFUNCTYPE(c.c_void_p, c.c_void_p, c.c_void_p)
    destroy_type = c.WINFUNCTYPE(None, c.c_void_p, c.c_ubyte)
    clone = clone_type(base + 0x1a0c5b0)
    destroy = destroy_type(base + 0x9ea3d0)

    def insert(table, result, hashed, key, unused, function):
        if table != 0x12345050 or hashed != 0x13579:
            errors.append(('registration parameters', table, hashed))
        text = c.string_at(get(key) if get(key + 24) > 15 else key, get(key + 16))
        object_address = get(function + 56)
        destination = (c.c_uint64 * 8)()
        registry[text] = clone(object_address, c.addressof(destination))
        return result

    def numeric(row, result):
        if row not in quantities:
            errors.append(('unknown numeric row', row))
        put(result, 'q', quantities.get(row, 0))
        return result

    def refresh_direction(table, result, hashed, key, unused, value):
        direction.value = get(value, 'B')
        return result

    def refresh_select(sorter, key):
        refresh_events.append(('select', sorter, key, direction.value))
        direction.value ^= 1

    def refresh_notify(sorter):
        refresh_events.append(('notify', sorter))

    try:
        for rva in [0x19d4ef0, 0x19d8490, 0x1a0c5b0, 0x1a0c4f0, 0x9ea3d0, 0x1a05570, 0x2333460]:
            native(rva)
        for rva, size in [(0x20ed7a0, 14), (0x20ed780, 26), (0x26dabc0, 14),
                          (0x7b7ff0, 32), (0x1a0ff50, 10), (0x1a0ff80, 10)]:
            native(rva, size)
        for table in [0x47b6938, 0x480c328, 0x485da18, 0x47b7828, 0x47b49a8]:
            commit(table, 40)
            for index in range(5):
                put(base + table + index * 8, 'Q', base + pe.u64(table + index * 8) - pe.base)
        stub(0x43b2210, c.WINFUNCTYPE(c.c_void_p, c.c_size_t), allocate)
        stub(0x43b2100, c.WINFUNCTYPE(None, c.c_void_p), release)
        stub(0x3e59340, c.WINFUNCTYPE(c.c_uint32, c.c_void_p, c.c_void_p, c.c_uint32),
             lambda unused, text, size: 0x13579)
        stub(0x1a0edd0, c.WINFUNCTYPE(c.c_void_p, c.c_void_p, c.c_void_p, c.c_uint32,
                                     c.c_void_p, c.c_void_p, c.c_void_p), insert)
        for rva in [0x200f110, 0x200dd50]:
            stub(rva, c.WINFUNCTYPE(c.c_void_p, c.c_void_p, c.c_void_p), numeric)
        stub(0x1a02440, c.WINFUNCTYPE(c.c_void_p, c.c_void_p, c.c_void_p),
             lambda table, key: c.addressof(direction))
        stub(0x1a0e740, c.WINFUNCTYPE(c.c_void_p, c.c_void_p, c.c_void_p, c.c_uint32,
                                     c.c_void_p, c.c_void_p, c.c_void_p), refresh_direction)
        stub(0x19d4c60, c.WINFUNCTYPE(None, c.c_void_p, c.c_void_p), refresh_select)
        stub(0x19e2730, c.WINFUNCTYPE(None, c.c_void_p), refresh_notify)
        for page in pages:
            previous = c.c_uint32()
            assert kernel.VirtualProtect(base + page, 0x1000, 0x20, c.byref(previous))
        assert kernel.FlushInstructionCache(kernel.GetCurrentProcess(), base, 0x4900000)
        register = c.WINFUNCTYPE(None, c.c_void_p, c.c_void_p, c.c_void_p)(base + 0x19d8490)
        sort = c.WINFUNCTYPE(c.c_void_p, c.c_void_p, c.c_void_p, c.c_void_p)(base + 0x1a05570)
        cases = 0
        for getter_table in [0x480c328, 0x485da18]:
            source = c.create_string_buffer(80)
            owner = c.addressof(source)
            put(owner, 'Q', base + 0x47b6938)
            put(owner + 8, 'Q', base + getter_table)
            put(owner + 64, 'Q', owner + 8)
            put(owner + 72, 'I', 1)
            for key, heap in [(key, heap) for key in [b'ironroad_work', b'ironroad_share'] for heap in [False, True]]:
                text = c.create_string_buffer(32)
                key_address = c.addressof(text)
                heap_text = c.create_string_buffer(key)
                if heap:
                    put(key_address, 'Q', c.addressof(heap_text))
                else:
                    c.memmove(key_address, key, len(key))
                put(key_address + 16, 'QQ', len(key), 31 if heap else 15)
                temporary = (c.c_uint64 * 8)()
                temporary[7] = clone(owner, c.addressof(temporary))
                temporary_object = temporary[7]
                register(0x12345000, key_address, c.addressof(temporary))
                retained = registry[key]
                assert temporary[7] == 0 and temporary_object in freed and retained not in freed
                assert get(retained + 64) == retained + 8, 'getter clone must not retain temporary storage'
                for count in [0, 1, 5, 32]:
                    rows = [c.create_string_buffer(16) for _ in range(count)]
                    values = [(0, 100000000, 1000000000, 900000000, (1 << 54) + 1)[i % 5]
                              for i in range(count)]
                    original = [c.addressof(row) for row in rows]
                    for row, value in zip(original, values):
                        quantities[row] = value
                        put(row + 8, 'i', 100)
                    for ascending in [0, 1]:
                        array = (c.c_uint64 * count)(*original)
                        argument = c.create_string_buffer(72)
                        put(c.addressof(argument) + 56, 'Q', retained)
                        put(c.addressof(argument) + 64, 'B', ascending)
                        sort(c.addressof(array), c.addressof(array) + count * 8, c.addressof(argument))
                        expected_rows = sorted(original, key=quantities.get, reverse=not ascending)
                        assert list(array) == expected_rows, (getter_table, count, ascending)
                        assert all(get(row + 8, 'i') == 100 for row in original)
                        cases += 1
                destroy(retained, 1)
                assert retained in freed
                del registry[key]
        refresh = c.WINFUNCTYPE(None, c.c_void_p)(base + 0x19d4ef0)
        for heap in [False, True]:
            sorter = c.create_string_buffer(80)
            address = c.addressof(sorter)
            text = c.create_string_buffer(b'ironroad_work')
            key = address + 8
            if heap:
                put(key, 'Q', c.addressof(text))
            else:
                c.memmove(key, b'ironroad_work', 13)
            put(key + 16, 'QQ', 13, 31 if heap else 15)
            for flag in [0, 1]:
                direction.value = flag
                refresh_events.clear()
                before = c.string_at(address, 80)
                refresh(address)
                assert direction.value == flag and c.string_at(address, 80) == before
                assert refresh_events == [('select', address, key, 1-flag), ('notify', address)]
        assert not errors, errors
        assert set(allocations) == freed and not registry, 'native temporary/retained payload leak'
        print(f'PASS native sort ABI: {cases} original ordering cases, exact integer getters, stable ties, '
              'native clone/registration/release and 4 direction-preserving refresh cases; allocation/map/getter fixture services')
    finally:
        kernel.VirtualFree(base, 0, 0x8000)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe', type=Path, default=Path(
        r'C:\Program Files (x86)\Steam\steamapps\common\Victoria 3\binaries\victoria3.exe'))
    run(parser.parse_args().exe)
