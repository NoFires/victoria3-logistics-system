# Name-anchored discovery logic adapted from Ironroad. No historical locations or signatures.
import re
import struct

class AnchorBroken(Exception):

    def __init__(self, step, detail):
        super().__init__('%s: %s' % (step, detail))
        self.step = step
        self.detail = detail

def hexs(v):
    return '0x%x' % v

class Rtti:

    def __init__(self, pe):
        self.pe = pe
        self.td = self._type_descriptors()
        self.cols = self._cols()

    def _type_descriptors(self):
        pe, out = (self.pe, {})
        for needle in (b'.?AV', b'.?AU'):
            for off in pe.find_bytes(needle, limit=1 << 24):
                end = pe.data.find(b'\x00', off, off + 1024)
                if end < 0:
                    continue
                r = pe.off2rva(off)
                if r is not None:
                    out[r - 16] = pe.data[off:end].decode('latin1')
        return out

    def _cols(self):
        pe = self.pe
        _nm, va, _vs, po, rs = [s for s in pe.secs if s[0] == '.rdata'][0]
        buf, out = (pe.data[po:po + rs], {})
        for i in range(0, len(buf) - 24, 4):
            sig, offs, cdoff, ptd, pchd, pself = struct.unpack('<6I', buf[i:i + 24])
            if sig != 1:
                continue
            col = va + i
            if pself == col and ptd in self.td:
                out[col] = dict(td=ptd, name=self.td[ptd], off=offs, cdoff=cdoff, chd=pchd)
        return out

    def unique_col(self, mangled, step):
        hits = [(c, i) for c, i in self.cols.items() if i['name'] == mangled]
        if len(hits) != 1:
            raise AnchorBroken(step, 'expected 1 complete object locator for %r, observed %d' % (mangled, len(hits)))
        return hits[0]

    def unique_vtable(self, col_rva, step):
        pe, out = (self.pe, [])
        for off in pe.find_bytes(struct.pack('<Q', pe.base + col_rva), limit=256):
            r = pe.off2rva(off)
            if r is not None and pe.sec_of(r) == '.rdata':
                out.append(r + 8)
        if len(out) != 1:
            raise AnchorBroken(step, 'expected 1 vtable behind COL %s, observed %d' % (hexs(col_rva), len(out)))
        return out[0]

    def slots(self, vt_rva, maxn=64):
        pe, out = (self.pe, [])
        for i in range(maxn):
            q = pe.u64(vt_rva + 8 * i)
            if q is None or q < pe.base:
                break
            r = q - pe.base
            if not pe.in_text(r):
                break
            out.append(r)
        return out

class Recover:
    ARROW_OBJECT = '.?AVCArrowObject@@'
    MILITARY_ARROW = '.?AVCMilitaryPathArrowObject@@'
    SUPPLY_ARROW = '.?AVCSupplyRouteArrowObject@@'
    CTOR_LAMBDA = '.?AV?$_Func_impl_no_alloc@V<lambda_1>@?1???0CArrowObject@@QEAA@PEBVCPdxArrowType@@@Z@X$$V@std@@'
    ARROW_VERTEX_BUFFER = '.?AV?$CPdxHybridInlineBufferAllocator@USArrowVertex@@$0DOI@H@@'
    NEW_DELETE_ALLOCATOR = '.?AVCPdxNewDeleteAllocator@@'

    def __init__(self, pe):
        self.pe = pe
        self.rtti = Rtti(pe)

    def _unique_referrer(self, target_rva, step, what):
        fns = self.pe.xref_funcs(target_rva)
        if len(fns) != 1:
            raise AnchorBroken(step, 'expected exactly 1 function referencing %s (%s), observed %d' % (hexs(target_rva), what, len(fns)))
        return fns[0]

    def _callees(self, fn_rva):
        ex = self.pe.extent(fn_rva)
        if ex is None:
            raise AnchorBroken('callees of %s' % hexs(fn_rva), 'no .pdata extent')
        return sorted(set(self.pe.calls_from(*ex)))

    def _require_function_start(self, rva, what):
        if not self.pe.is_function_start(rva):
            raise AnchorBroken('%s: .pdata entry' % what, '%s is not a true function start' % hexs(rva))

    def _class_vtable(self, mangled, label, steps):
        col, info = self.rtti.unique_col(mangled, '%s: complete object locator' % label)
        vt = self.rtti.unique_vtable(col, '%s: vtable' % label)
        steps.append(dict(step='%s: RTTI type descriptor %s -> complete object locator -> vtable' % (label, mangled), asserts='exactly one COL for the descriptor and exactly one vtable behind it', expected_matches=1, observed_matches=1, result_rva=hexs(vt)))
        return (col, info, vt)

    def arrow_object_ctor(self):
        steps = []
        _col, _info, vt = self._class_vtable(self.ARROW_OBJECT, 'CArrowObject', steps)
        fn = self._unique_referrer(vt, 'CArrowObject: vtable referrer', 'a base vtable is installed only by its constructor')
        steps.append(dict(step='the single .text function referencing that vtable', asserts='exactly one, and it is the constructor', expected_matches=1, observed_matches=1, result_rva=hexs(fn)))
        self._require_function_start(fn, 'CArrowObject::ctor')
        return (fn, vt, steps)

    def arrow_object_ctor_cross(self):
        steps = []
        _col, _info, vt = self._class_vtable(self.CTOR_LAMBDA, 'the constructor lambda', steps)
        holder = self._unique_referrer(vt, 'constructor lambda: vtable referrer', 'the std::function constructor for that lambda')
        steps.append(dict(step='the single .text function referencing the _Func_impl vtable', expected_matches=1, observed_matches=1, result_rva=hexs(holder)))
        fn = self._unique_referrer(holder, 'constructor lambda: holder caller', 'a lambda declared inside one function is constructed only there')
        steps.append(dict(step='the single caller of that std::function constructor', expected_matches=1, observed_matches=1, result_rva=hexs(fn)))
        return (fn, steps)

    def constant_buffer_slot(self):
        steps = []
        _c, _i, base_vt = self._class_vtable(self.ARROW_OBJECT, 'CArrowObject', steps)
        _c, _i, mil_vt = self._class_vtable(self.MILITARY_ARROW, 'CMilitaryPathArrowObject', steps)
        _c, _i, sup_vt = self._class_vtable(self.SUPPLY_ARROW, 'CSupplyRouteArrowObject', steps)
        base, mil, sup = (self.rtti.slots(v) for v in (base_vt, mil_vt, sup_vt))
        if not len(base) == len(mil) == len(sup):
            raise AnchorBroken('constant-buffer slot: vtable lengths', 'base %d, military %d, supply %d' % (len(base), len(mil), len(sup)))
        diff = [i for i in range(len(base)) if base[i] != mil[i] and mil[i] == sup[i]]
        if len(diff) != 1:
            raise AnchorBroken('constant-buffer slot: unique divergent index', 'expected exactly 1 slot where the base differs from both subclasses and the subclasses agree, observed %d: %s' % (len(diff), diff))
        idx = diff[0]
        steps.append(dict(step='the unique slot index at which CArrowObject differs from both shipped subclasses, which agree with each other', asserts='three vtables of equal length; exactly one divergent index', expected_matches=1, observed_matches=1, result_rva=hexs(base_vt + 8 * idx)))
        return (dict(vtable_rva=base_vt, military_vtable_rva=mil_vt, supply_vtable_rva=sup_vt, slot_index=idx, slot_count=len(base), base_target=base[idx], subclass_target=mil[idx], slots=base), steps)

    def _subclass_path_updater(self, mangled, label, steps):
        _c, _i, vt = self._class_vtable(mangled, label, steps)
        ctor = self._unique_referrer(vt, '%s: vtable referrer' % label, 'the subclass constructor')
        steps.append(dict(step='%s: the single .text function installing that vtable' % label, expected_matches=1, observed_matches=1, result_rva=hexs(ctor)))
        getter = self._unique_referrer(ctor, '%s: constructor caller' % label, 'the typed get-or-create helper')
        steps.append(dict(step='%s: the single caller of that constructor' % label, expected_matches=1, observed_matches=1, result_rva=hexs(getter)))
        callers = self.pe.xref_funcs(getter)
        if len(callers) != 1:
            raise AnchorBroken('%s: helper callers' % label, 'expected exactly 1 function calling %s, observed %d' % (hexs(getter), len(callers)))
        steps.append(dict(step='%s: the single caller of that helper - the path updater' % label, expected_matches=1, observed_matches=1, result_rva=hexs(callers[0])))
        return (callers[0], getter)

    def _updater_pair(self, steps):
        sup, sup_getter = self._subclass_path_updater(self.SUPPLY_ARROW, 'CSupplyRouteArrowObject', steps)
        mil, mil_getter = self._subclass_path_updater(self.MILITARY_ARROW, 'CMilitaryPathArrowObject', steps)
        common = sorted(set(self._callees(sup)) & set(self._callees(mil)))
        steps.append(dict(step='callees common to both path updaters', asserts='two independent RTTI roots that must meet', observed_matches=len(common)))
        return (sup, mil, common, sup_getter, mil_getter)

    def arrow_object_set_path(self):
        steps = []
        sup, mil, common, _sg, _mg = self._updater_pair(steps)
        _c, _i, vbuf_vt = self._class_vtable(self.ARROW_VERTEX_BUFFER, 'the SArrowVertex buffer', steps)
        direct = set(self.pe.xref_funcs(vbuf_vt))
        reaching = [f for f in common if f in direct or set(self._callees(f)) & direct]
        if len(reaching) != 1:
            raise AnchorBroken('set_path: vertex-buffer intersection', 'expected exactly 1 shared callee reaching the SArrowVertex buffer allocator, observed %d: %s' % (len(reaching), [hexs(x) for x in reaching]))
        fn = reaching[0]
        steps.append(dict(step='the unique shared callee that reaches the SArrowVertex buffer allocator within one further call', asserts='exactly one of the shared callees builds arrow geometry', expected_matches=1, observed_matches=1, result_rva=hexs(fn)))
        self._require_function_start(fn, 'CArrowObject::SetPath')
        return (fn, steps, dict(supply_updater=sup, military_updater=mil, common=common, vertex_buffer_vtable=vbuf_vt, vertex_buffer_builder=sorted(direct)[0] if direct else None))

    def ensure_constant_buffer(self):
        steps = []
        _sup, _mil, common, _sg, _mg = self._updater_pair(steps)
        slot, _s = self.constant_buffer_slot()
        field = self._slot_read_offset(slot['subclass_target'])
        steps.append(dict(step='the object field the divergent slot uploads from, read off its own instruction stream rather than assumed', asserts='the slot body loads one qword field of `this` into rdx', expected_matches=1, observed_matches=1, result_rva=hexs(field)))
        writers = [f for f in common if self._writes_qword_field(f, field)]
        if len(writers) != 1:
            raise AnchorBroken('ensure_constant_buffer: field writers', 'expected exactly 1 shared callee storing into this+%s, observed %d: %s' % (hexs(field), len(writers), [hexs(x) for x in writers]))
        fn = writers[0]
        steps.append(dict(step='the unique callee shared by both updaters that stores into that field', expected_matches=1, observed_matches=1, result_rva=hexs(fn)))
        self._require_function_start(fn, 'CArrowObject::EnsureConstantBuffer')
        return (fn, field, steps)

    def _slot_read_offset(self, slot_target):
        for ins in self.pe.insns(slot_target, slot_target + 64):
            if ins.mnemonic == 'mov' and ins.op_str.startswith('rdx, qword ptr ['):
                m = re.search('\\+ (0x[0-9a-f]+)\\]', ins.op_str)
                if m:
                    return int(m.group(1), 16)
            if ins.mnemonic in ('jmp', 'ret'):
                break
        raise AnchorBroken('constant-buffer slot: field read', 'the slot body does not load a qword field of `this` into rdx')

    def _writes_qword_field(self, fn_rva, field):
        ex = self.pe.extent(fn_rva)
        if ex is None:
            return False
        want = '+ %s], r' % hexs(field)
        for ins in self.pe.insns(*ex):
            if ins.mnemonic == 'mov' and 'qword ptr [' in ins.op_str and (want in ins.op_str):
                return True
        return False

    def bucket_append(self, slot_info, type_field, bucket_field):
        steps = []
        cands = [(i, t) for i, t in enumerate(slot_info['slots']) if self._is_bucket_append(t, type_field, bucket_field)]
        if len(cands) != 1:
            raise AnchorBroken('bucket_append: slot scan', 'expected exactly 1 slot whose body reads the arrow type at this+%s and the word at type+%s, observed %d: %s' % (hexs(type_field), hexs(bucket_field), len(cands), [(i, hexs(t)) for i, t in cands]))
        idx, fn = cands[0]
        steps.append(dict(step='the unique CArrowObject vtable slot whose body loads the arrow type from this+%s and sign-extends the word at type+%s' % (hexs(type_field), hexs(bucket_field)), asserts='exactly one slot appends by bucket id', expected_matches=1, observed_matches=1, result_rva=hexs(fn)))
        self._require_function_start(fn, 'the arrow bucket append')
        return (fn, idx, steps)

    def _is_bucket_append(self, fn_rva, type_field, bucket_field):
        ex = self.pe.extent(fn_rva)
        if ex is None or ex[1] - ex[0] > 1024:
            return False
        loads_type = False
        want_type = re.compile('^rdx, qword ptr \\[\\w+ \\+ %s\\]$' % hexs(type_field))
        want_bucket = 'word ptr [rdx + %s]' % hexs(bucket_field)
        for ins in self.pe.insns(*ex):
            if ins.mnemonic == 'mov' and want_type.match(ins.op_str):
                loads_type = True
            elif loads_type and ins.mnemonic == 'movsx' and (want_bucket in ins.op_str):
                return True
        return False

    def ctor_field_offsets(self, ctor_rva):
        ex = self.pe.extent(ctor_rva)
        type_field = flag_field = None
        zeroed = []
        for ins in self.pe.insns(*ex):
            if ins.mnemonic == 'or' and ins.op_str.startswith('byte ptr [rcx + '):
                m = re.search('\\+ (0x[0-9a-f]+)\\], (0x[0-9a-f]+|\\d+)', ins.op_str)
                if m:
                    flag_field = (int(m.group(1), 16), int(m.group(2), 0))
            if ins.mnemonic == 'mov' and ins.op_str.endswith(', rdx') and ins.op_str.startswith('qword ptr [rcx + '):
                m = re.search('\\+ (0x[0-9a-f]+)\\]', ins.op_str)
                if m:
                    type_field = int(m.group(1), 16)
            if ins.mnemonic == 'mov' and ins.op_str.startswith('qword ptr [rcx + ') and ins.op_str.endswith(', rbx'):
                m = re.search('\\+ (0x[0-9a-f]+)\\]', ins.op_str)
                if m:
                    zeroed.append(int(m.group(1), 16))
        if type_field is None or flag_field is None:
            raise AnchorBroken('ctor: field offsets', 'the constructor does not both OR a flag byte and store rdx into a qword field')
        return (type_field, flag_field, zeroed)

    def render_bucket_list(self, append_rva):
        steps = []
        callees = self.pe.calls_from(*self.pe.extent(append_rva))
        if not callees:
            raise AnchorBroken('bucket_list: append callees', 'the append makes no direct call')
        reader = callees[0]
        if not self._derefs_second_argument(reader):
            raise AnchorBroken('bucket_list: context reader', '%s does not open by dereferencing the render context in rdx' % hexs(reader))
        steps.append(dict(step="the append's first callee, which dereferences the render context in rdx before touching anything else", asserts='mov <reg>, rdx followed by mov edx, dword ptr [rdx]', expected_matches=1, observed_matches=1, result_rva=hexs(reader)))
        level1 = self.pe.calls_from(*self.pe.extent(reader))[0]
        steps.append(dict(step='its first callee - the first-level bucket accessor', expected_matches=1, observed_matches=1, result_rva=hexs(level1)))
        root = self._container_root(level1)
        steps.append(dict(step='the .data container that accessor indexes, taken as the lowest of the RIP-relative addresses it reads for data pointer, capacity and size', asserts='at least three of its .data reads land inside one 0x18-byte CPdxVector', expected_matches=1, observed_matches=1, result_rva=hexs(root)))
        return (root, reader, level1, steps)

    def _derefs_second_argument(self, fn_rva):
        moved = False
        for ins in self.pe.insns(fn_rva, fn_rva + 64):
            if ins.mnemonic == 'mov' and ins.op_str.endswith(', rdx') and (',' in ins.op_str) and ('[' not in ins.op_str):
                moved = True
            elif moved and ins.mnemonic == 'mov' and (ins.op_str == 'edx, dword ptr [rdx]'):
                return True
        return False

    def _container_root(self, fn_rva):
        hits = set()
        for ins in self.pe.insns(*self.pe.extent(fn_rva)):
            m = re.search('\\[rip \\+ (0x[0-9a-f]+)\\]', ins.op_str)
            if not m or ins.mnemonic not in ('mov', 'movsxd'):
                continue
            tgt = ins.address + ins.size + int(m.group(1), 16) - self.pe.base
            if self.pe.sec_of(tgt) == '.data':
                hits.add(tgt)
        if not hits:
            raise AnchorBroken('bucket_list: container reads', 'the accessor reads no .data address')
        root = min(hits)
        near = {h for h in hits if root <= h < root + 24}
        if len(near) < 3:
            raise AnchorBroken('bucket_list: container shape', 'expected at least 3 reads inside one 0x18-byte container at %s, observed %d' % (hexs(root), len(near)))
        return root

    def render_pass(self, bucket_list_rva):
        steps = []
        readers = sorted(set(self.pe.xref_funcs(bucket_list_rva)) | set(self.pe.xref_funcs(bucket_list_rva + 12)))
        steps.append(dict(step='functions reading the bucket list container', observed_matches=len(readers)))
        cands = [f for f in readers if self._reads_index_triple(f)]
        if len(cands) != 1:
            raise AnchorBroken('render_pass: triple indexing', 'expected exactly 1 reader indexing the list with a caller-supplied triple, observed %d: %s' % (len(cands), [hexs(x) for x in cands]))
        fn = cands[0]
        steps.append(dict(step='the unique reader that indexes the list with a caller-supplied triple at [rcx], [rcx+8] and [rcx+0x10]', expected_matches=1, observed_matches=1, result_rva=hexs(fn)))
        self._require_function_start(fn, 'the arrow render pass')
        callers = self.pe.xref_funcs(fn)
        return (fn, callers, steps)

    def _reads_index_triple(self, fn_rva):
        ex = self.pe.extent(fn_rva)
        if ex is None:
            return False
        want = {'rax, dword ptr [rcx]', 'rax, dword ptr [rcx + 8]', 'rdx, dword ptr [rcx + 0x10]'}
        got = {i.op_str for i in self.pe.insns(fn_rva, min(ex[1], fn_rva + 128)) if i.mnemonic == 'movsxd'}
        return want <= got

    def get_arrow_type(self):
        steps = []
        _c, _i, vt = self._class_vtable(self.MILITARY_ARROW, 'CMilitaryPathArrowObject', steps)
        ctor = self._unique_referrer(vt, 'military: vtable referrer', 'the subclass constructor')
        getter = self._unique_referrer(ctor, 'military: constructor caller', 'the typed get-or-create helper')
        steps.append(dict(step='CMilitaryPathArrowObject RTTI -> constructor -> typed helper', expected_matches=1, observed_matches=1, result_rva=hexs(getter)))
        calls = self.pe.calls_from(*self.pe.extent(getter))
        if len(calls) < 2:
            raise AnchorBroken('get_arrow_type: helper shape', 'the typed helper makes %d direct calls, expected at least 2' % len(calls))
        name_builder, resolver = (calls[0], calls[1])
        if not self._passes_result_as_second_argument(getter, name_builder, resolver):
            raise AnchorBroken('get_arrow_type: call shape', 'the helper does not hand the freshly built name straight to its second callee')
        steps.append(dict(step="the helper's second direct callee, given the freshly built name in rdx", asserts='call <name builder>; mov rdx, rax; call <resolver>', expected_matches=1, observed_matches=1, result_rva=hexs(resolver)))
        if not self._is_string_keyed_lookup(resolver):
            raise AnchorBroken('get_arrow_type: string contract', '%s does not read rdx as an MSVC std::string' % hexs(resolver))
        steps.append(dict(step='confirm it reads rdx as an MSVC std::string - capacity at +0x18 against 0xf selects SSO against pointer, size at +0x10 feeds the hash', asserts='cmp qword ptr [rdx + 0x18], 0xf and mov ebx, dword ptr [rdx + 0x10]', expected_matches=1, observed_matches=1))
        self._require_function_start(resolver, 'CAssetFactory::GetArrowType')
        return (resolver, name_builder, getter, steps)

    def _passes_result_as_second_argument(self, fn_rva, first, second):
        seq = []
        for ins in self.pe.insns(*self.pe.extent(fn_rva)):
            if ins.mnemonic == 'call' and ins.op_str.startswith('0x'):
                seq.append(('call', int(ins.op_str, 16) - self.pe.base))
            elif ins.mnemonic == 'mov' and ins.op_str == 'rdx, rax':
                seq.append(('mov_rdx_rax', None))
        return any((seq[i] == ('call', first) and seq[i + 1][0] == 'mov_rdx_rax' and (seq[i + 2] == ('call', second)) for i in range(len(seq) - 2)))

    def _is_string_keyed_lookup(self, fn_rva):
        ex = self.pe.extent(fn_rva)
        ops = {i.op_str for i in self.pe.insns(fn_rva, min(ex[1], fn_rva + 64))}
        return 'qword ptr [rdx + 0x18], 0xf' in ops and 'ebx, dword ptr [rdx + 0x10]' in ops
    SUBREGS = {'rcx': ('rcx', 'ecx', 'cx', 'cl', 'ch'), 'rdx': ('rdx', 'edx', 'dx', 'dl', 'dh'), 'r8': ('r8', 'r8d', 'r8w', 'r8b'), 'r9': ('r9', 'r9d', 'r9w', 'r9b')}

    def first_touch(self, fn_rva, reg):
        names = set(self.SUBREGS[reg])
        for ins in self.pe.insns(*self.pe.extent(fn_rva)):
            if ins.mnemonic in ('call', 'syscall'):
                return ('read', 'reaches %s %s untouched' % (ins.mnemonic, ins.op_str))
            rd, wr = ins.regs_access()
            if names & {ins.reg_name(r) for r in rd}:
                return ('read', '%s %s' % (ins.mnemonic, ins.op_str))
            if names & {ins.reg_name(r) for r in wr}:
                return ('write', '%s %s' % (ins.mnemonic, ins.op_str))
        return ('untouched', '')

    def dead_on_entry(self, fn_rva, regs):
        out = []
        for reg in regs:
            kind, _text = self.first_touch(fn_rva, reg)
            if kind in ('write', 'untouched'):
                out.append(reg)
        return out

    def caller_rcx_evidence(self, caller_rva, callee_rva):
        last = None
        for ins in self.pe.insns(*self.pe.extent(caller_rva)):
            _rd, wr = ins.regs_access()
            if {ins.reg_name(r) for r in wr} & set(self.SUBREGS['rcx']):
                last = '%s %s' % (ins.mnemonic, ins.op_str)
            if ins.mnemonic == 'call' and ins.op_str.startswith('0x') and (int(ins.op_str, 16) - self.pe.base == callee_rva):
                return last
        return None

    def render_device(self, ensure_cb_rva):
        steps = []
        hits = set()
        for ins in self.pe.insns(*self.pe.extent(ensure_cb_rva)):
            m = re.match('^rax, qword ptr \\[rip \\+ (0x[0-9a-f]+)\\]$', ins.op_str)
            if m and ins.mnemonic == 'mov':
                tgt = ins.address + ins.size + int(m.group(1), 16) - self.pe.base
                if self.pe.sec_of(tgt) == '.data':
                    hits.add(tgt)
        if len(hits) != 1:
            raise AnchorBroken('render_device: global read', 'expected exactly 1 .data global loaded into rax, observed %d: %s' % (len(hits), [hexs(x) for x in sorted(hits)]))
        g = hits.pop()
        steps.append(dict(step='the single .data global the constant-buffer creator loads and dereferences before calling through a vtable', expected_matches=1, observed_matches=1, result_rva=hexs(g)))
        return (g, steps)

    def new_delete_allocator(self):
        steps = []
        _c, _i, vt = self._class_vtable(self.NEW_DELETE_ALLOCATOR, 'CPdxNewDeleteAllocator', steps)
        hits = []
        for off in self.pe.find_bytes(struct.pack('<Q', self.pe.base + vt), limit=4096):
            r = self.pe.off2rva(off)
            if r is not None and self.pe.sec_of(r) == '.data':
                hits.append(r)
        if len(hits) != 1:
            raise AnchorBroken('allocator: singleton', 'expected exactly 1 .data object carrying that vtable, observed %d: %s' % (len(hits), [hexs(x) for x in hits]))
        steps.append(dict(step='the unique .data qword equal to that vtable address - the static allocator instance', asserts='exactly one instance image-wide', expected_matches=1, observed_matches=1, result_rva=hexs(hits[0])))
        return (hits[0], vt, steps)
