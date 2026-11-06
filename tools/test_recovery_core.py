"""Reject unrelated stores while discovering changed native arrow uploaders."""
import unittest
from types import SimpleNamespace

from recovery_core import Recover, AnchorBroken


class FakePe:
    def __init__(self, bodies):
        self.bodies = bodies

    def extent(self, address):
        return (address, address + 64) if address in self.bodies else None

    def insns(self, start, end):
        for mnemonic, operands in self.bodies[start]:
            yield SimpleNamespace(mnemonic=mnemonic, op_str=operands)


class RecoveryTests(unittest.TestCase):
    def writer(self, body):
        rec = Recover.__new__(Recover)
        rec.pe = FakePe({1: body})
        return rec._writes_qword_field(1, 0x60)

    def test_this_survives_call_in_nonvolatile_register(self):
        self.assertTrue(self.writer([
            ('mov', 'rbx, rcx'), ('call', '0x1000'),
            ('mov', 'qword ptr [rbx + 0x60], rax')]))

    def test_new_allocation_is_not_this(self):
        self.assertFalse(self.writer([
            ('call', '0x1000'), ('mov', 'qword ptr [rax + 0x60], rbx')]))

    def test_volatile_alias_is_not_preserved_across_call(self):
        self.assertFalse(self.writer([
            ('mov', 'r10, rcx'), ('call', '0x1000'),
            ('mov', 'qword ptr [r10 + 0x60], rax')]))

    def test_adjusted_this_and_overwritten_alias(self):
        self.assertTrue(self.writer([
            ('lea', 'rbx, [rcx + 0x20]'), ('mov', 'qword ptr [rbx + 0x40], rax')]))
        self.assertFalse(self.writer([
            ('mov', 'rbx, rcx'), ('mov', 'rbx, qword ptr [rcx]'),
            ('mov', 'qword ptr [rbx + 0x60], rax')]))

    def slot(self, military, supply):
        rec = Recover.__new__(Recover)
        tables = {10: [1, 2], 20: [1, military], 30: [1, supply]}
        rec._class_vtable = lambda name, label, steps: (0, {}, {
            rec.ARROW_OBJECT: 10, rec.MILITARY_ARROW: 20, rec.SUPPLY_ARROW: 30}[name])
        rec.rtti = SimpleNamespace(slots=lambda vt: tables[vt])
        rec._slot_read_offset = lambda address: {3: 0x60, 4: 0x60, 5: 0x68}[address]
        return rec.constant_buffer_slot()[0]

    def test_identical_and_distinct_uploaders(self):
        self.assertEqual(self.slot(3, 3)['slot_index'], 1)
        self.assertEqual(self.slot(3, 4)['subclass_target'], 4)

    def test_different_buffer_fields_are_rejected(self):
        with self.assertRaises(AnchorBroken):
            self.slot(3, 5)

    def test_ambiguous_slot_is_rejected(self):
        rec = Recover.__new__(Recover)
        rec._class_vtable = lambda name, label, steps: (0, {}, name)
        rec.rtti = SimpleNamespace(slots=lambda name: [1, 2] if name == rec.ARROW_OBJECT else [3, 4])
        rec._slot_read_offset = lambda address: 0x60
        with self.assertRaises(AnchorBroken):
            rec.constant_buffer_slot()


if __name__ == '__main__':
    unittest.main()
