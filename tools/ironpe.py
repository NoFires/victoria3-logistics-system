# Generic PE parsing and cross-reference helpers; all locations are derived from the input.
import bisect
import hashlib
import os
import struct
import numpy as np
IMAGE_FILE_MACHINE_AMD64 = 34404

class PE:

    def __init__(self, path):
        self.path = os.path.abspath(path)
        with open(path, 'rb') as fh:
            self.data = fh.read()
        d = self.data
        if d[:2] != b'MZ':
            raise ValueError('not a PE: no MZ signature')
        pe_off = struct.unpack_from('<I', d, 60)[0]
        if d[pe_off:pe_off + 4] != b'PE\x00\x00':
            raise ValueError('not a PE: no PE signature')
        coff = pe_off + 4
        machine, nsec, _ts, _sym, _nsym, opt_size, _chars = struct.unpack_from('<HHIIIHH', d, coff)
        if machine != IMAGE_FILE_MACHINE_AMD64:
            raise ValueError('not an x64 image (machine=0x%04x)' % machine)
        opt = coff + 20
        magic = struct.unpack_from('<H', d, opt)[0]
        if magic != 523:
            raise ValueError('not PE32+ (magic=0x%04x)' % magic)
        self.base = struct.unpack_from('<Q', d, opt + 24)[0]
        nrva = struct.unpack_from('<I', d, opt + 108)[0]
        dd = opt + 112
        self.dirs = [struct.unpack_from('<II', d, dd + 8 * i) for i in range(nrva)]
        sec_off = opt + opt_size
        self.secs = []
        for i in range(nsec):
            o = sec_off + 40 * i
            name = d[o:o + 8].rstrip(b'\x00').decode('latin1')
            vsize, vaddr, rsize, raddr = struct.unpack_from('<IIII', d, o + 8)
            self.secs.append((name, vaddr, vsize, raddr, rsize))
        self.text = self._sec('.text')
        self._build_pdata()
        self._build_disp()

    @property
    def size(self):
        return len(self.data)

    @property
    def md5(self):
        if not hasattr(self, '_md5'):
            self._md5 = hashlib.md5(self.data).hexdigest()
        return self._md5

    def sibling_checksum_txt(self):
        p = os.path.join(os.path.dirname(self.path), 'checksum.txt')
        try:
            with open(p) as fh:
                return fh.read().strip()
        except OSError:
            return None

    def _sec(self, name):
        for s in self.secs:
            if s[0] == name:
                return s
        return None

    def rva2off(self, rva):
        for _nm, va, vs, po, rs in self.secs:
            if va <= rva < va + max(vs, rs):
                d = rva - va
                return po + d if d < rs else None
        return None

    def off2rva(self, off):
        for _nm, va, _vs, po, rs in self.secs:
            if po <= off < po + rs:
                return va + (off - po)
        return None

    def sec_of(self, rva):
        for nm, va, vs, _po, rs in self.secs:
            if va <= rva < va + max(vs, rs):
                return nm
        return None

    def va(self, rva):
        return self.base + rva

    def rva(self, va):
        return va - self.base

    def rd(self, rva, n):
        o = self.rva2off(rva)
        return self.data[o:o + n] if o is not None else None

    def u8(self, rva):
        b = self.rd(rva, 1)
        return b[0] if b else None

    def u32(self, rva):
        b = self.rd(rva, 4)
        return struct.unpack('<I', b)[0] if b and len(b) == 4 else None

    def u64(self, rva):
        b = self.rd(rva, 8)
        return struct.unpack('<Q', b)[0] if b and len(b) == 8 else None

    def in_text(self, rva):
        _nm, va, vs, _po, rs = self.text
        return va <= rva < va + max(vs, rs)

    def _build_pdata(self):
        sec = self._sec('.pdata')
        _nm, _va, vs, po, rs = sec
        n = min(vs, rs) // 12
        ent = []
        for i in range(n):
            b, e, u = struct.unpack_from('<III', self.data, po + i * 12)
            if b == 0 and e == 0:
                continue
            ent.append((b, e, u))
        ent.sort()
        self.pdata = ent
        self._pbeg = [e[0] for e in ent]

    def _unwind_flags(self, u):
        o = self.rva2off(u)
        if o is None:
            return None
        return self.data[o] >> 3

    def chunk_of(self, rva):
        i = bisect.bisect_right(self._pbeg, rva) - 1
        if i < 0:
            return None
        b, e, u = self.pdata[i]
        return (b, e, u) if b <= rva < e else None

    def chain_root(self, rva):
        c = self.chunk_of(rva)
        if c is None:
            return None
        b, e, u = c
        for _ in range(64):
            f = self._unwind_flags(u)
            if f is None or not f & 4:
                return (b, e)
            o = self.rva2off(u)
            cnt = self.data[o + 2]
            nxt = o + 4 + (cnt + 1 & ~1) * 2
            b, e, u = struct.unpack_from('<III', self.data, nxt)
        return (b, e)

    def extent(self, rva):
        r = self.chain_root(rva)
        if r is None:
            return None
        b, _ = r
        i = bisect.bisect_left(self._pbeg, b)
        if i >= len(self.pdata) or self.pdata[i][0] != b:
            return None
        end = self.pdata[i][1]
        j = i + 1
        while j < len(self.pdata):
            bb, ee, _uu = self.pdata[j]
            if bb != end:
                break
            cr = self.chain_root(bb)
            if cr is None or cr[0] != b:
                break
            end = ee
            j += 1
        return (b, end)

    def func_of(self, rva):
        ex = self.extent(rva)
        return ex[0] if ex else None

    def is_function_start(self, rva):
        c = self.chunk_of(rva)
        if c is None or c[0] != rva:
            return False
        return (self._unwind_flags(c[2]) or 0) & 4 == 0

    def _build_disp(self):
        _nm, va, _vs, po, rs = self.text
        n = rs
        buf = np.frombuffer(self.data[po:po + n], dtype=np.uint8)
        d = buf[:n - 4].astype(np.uint32)
        d |= buf[1:n - 3].astype(np.uint32) << 8
        d |= buf[2:n - 2].astype(np.uint32) << 16
        d |= buf[3:n - 1].astype(np.uint32) << 24
        idx = np.arange(n - 4, dtype=np.uint32)
        self._targets = (d + idx + np.uint32(4) + np.uint32(va)).astype(np.uint32)
        self._text_rva = va

    def xref_sites(self, target_rva):
        hits = np.nonzero(self._targets == np.uint32(target_rva))[0]
        return [self._text_rva + int(i) for i in hits]

    def xrefs(self, target_rva):
        return [(c, self.func_of(c)) for c in self.xref_sites(target_rva)]

    def xref_funcs(self, target_rva):
        return sorted({f for _c, f in self.xrefs(target_rva) if f is not None})

    def find_bytes(self, needle, limit=1000):
        out = []
        start = 0
        while len(out) < limit:
            i = self.data.find(needle, start)
            if i < 0:
                break
            out.append(i)
            start = i + 1
        return out

    def _literal_start(self, off):
        i = off
        while i > 0 and 32 <= self.data[i - 1] < 127:
            i -= 1
        return i

    def find_cstr(self, s, limit=1000, sections=None):
        needle = s.encode() if isinstance(s, str) else s
        out = []
        seen = set()
        for off in self.find_bytes(needle + b'\x00', limit=limit * 4):
            st = self._literal_start(off)
            if st != off:
                continue
            r = self.off2rva(st)
            if r is None or r in seen:
                continue
            if sections and self.sec_of(r) not in sections:
                continue
            seen.add(r)
            out.append(r)
            if len(out) >= limit:
                break
        return out

    def find_substr_literals(self, s, limit=1000, sections=None):
        needle = s.encode() if isinstance(s, str) else s
        out = []
        seen = set()
        for off in self.find_bytes(needle, limit=limit * 4):
            st = self._literal_start(off)
            r = self.off2rva(st)
            if r is None or r in seen:
                continue
            end = st
            while end < len(self.data) and 32 <= self.data[end] < 127:
                end += 1
            if end >= len(self.data) or self.data[end] != 0:
                continue
            if sections and self.sec_of(r) not in sections:
                continue
            seen.add(r)
            out.append((r, self.data[st:end].decode('latin1')))
            if len(out) >= limit:
                break
        return out

    def cstr_at(self, rva, maxlen=512):
        o = self.rva2off(rva)
        if o is None:
            return None
        end = self.data.find(b'\x00', o, o + maxlen)
        return self.data[o:end].decode('latin1') if end > 0 else None

    def md(self, detail=True):
        import capstone
        m = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
        m.detail = detail
        return m

    def insns(self, start_rva, end_rva=None, detail=True):
        if end_rva is None:
            ex = self.extent(start_rva)
            end_rva = ex[1] if ex else start_rva + 256
        o = self.rva2off(start_rva)
        code = self.data[o:o + (end_rva - start_rva)]
        return list(self.md(detail).disasm(code, self.base + start_rva))

    def calls_from(self, start_rva, end_rva=None):
        out = []
        for ins in self.insns(start_rva, end_rva, detail=False):
            if ins.mnemonic in ('call', 'jmp') and ins.op_str.startswith('0x'):
                out.append(int(ins.op_str, 16) - self.base)
        return out
