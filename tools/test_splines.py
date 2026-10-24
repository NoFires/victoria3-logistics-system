"""Regression checks for the real spline decoder, including corrupt inputs."""
import unittest
import struct
from pathlib import Path
from gen_splines import decode


class DecoderTests(unittest.TestCase):
    def fixture(self, refs=(257, 513), end=200):
        def token(value):
            return struct.pack('<H', value)
        def value(v, floating=False):
            return token(0x0D if floating else 0x029C) + struct.pack('<f' if floating else '<Q', v)
        def block(values):
            return token(3) + b''.join(values) + token(4)
        def record(identity, key, values):
            return block([token(11), token(1), value(identity), token(key), token(1), block(values)])
        points = [record(i, 76, [value(x, True), value(z, True)])
                  for i, x, z in ((100, 0, 0), (150, 5, 1), (200, 10, 0))]
        strips = [record(257, 1527, [value(100), value(150)]),
                  record(513, 1527, [value(150), value(200)])]
        link = record((end << 35) | (100 << 6) | 1, 1525, [value(i) for i in refs])
        return b''.join([token(238), token(1), value(4), token(1114), token(1),
                         block([value(3), value(2), value(1)]), token(1524), token(1), block(points),
                         token(1525), token(1), block(strips), token(1526), token(1), block([link])])

    def test_link_validation(self):
        # Strip IDs also carry their transport type in the low byte.
        raw = self.fixture()
        self.assertEqual(decode(raw)[2][(200 << 35) | (100 << 6) | 1], [257, 513])
        for refs in ((257, 257), (257, 999), (513, 257), (257,), ()):
            invalid = self.fixture(refs)
            with self.assertRaises(ValueError):
                decode(invalid)
        with self.assertRaisesRegex(ValueError, 'endpoints'):
            decode(raw.replace(struct.pack('<Q', (200 << 35) | (100 << 6) | 1),
                               struct.pack('<Q', (201 << 35) | (100 << 6) | 1)))

    @classmethod
    def setUpClass(cls):
        cls.raw = (Path(r'C:\Program Files (x86)\Steam\steamapps\common\Victoria 3') /
                   'game/gfx/map/spline_network/spline_network.splnet').read_bytes()

    def test_installed_schema(self):
        points, strips, links = decode(self.raw)
        self.assertEqual(len(points), 33780)
        self.assertEqual(len(strips), 4273)
        self.assertAlmostEqual(points[100][1], 3053.0359, places=3)
        self.assertTrue(any(sid & 255 == 3 for sid in strips))
        self.assertEqual(len(links), 4271)
        self.assertEqual(links[(903 << 35) | (900 << 6) | 1], [601089, 601345])
        self.assertEqual({sid for refs in links.values() for sid in refs}, strips.keys())

    def test_truncated(self):
        for n in (0, 1, 7, 65, len(self.raw)-1, len(self.raw)-2):
            with self.assertRaises((ValueError, TypeError)):
                decode(self.raw[:n])

    def test_unknown_token(self):
        with self.assertRaisesRegex(ValueError, 'unknown'):
            decode(b'\xff\xff' + self.raw[2:])

    def test_unknown_version(self):
        with self.assertRaisesRegex(ValueError, 'version'):
            decode(self.raw[:6] + b'\x05\x00\x00\x00' + self.raw[10:])


if __name__ == '__main__':
    unittest.main()
