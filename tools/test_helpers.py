import unittest
from render_model import Ribbon, RenderModel, SettleGate
from make_arrow_asset import build_text
from summarize_draw_log import summarize


def ribbon(n, style='heat_low'):
    return Ribbon(((n, 0), (n + 1, 1)), style)


class Helpers(unittest.TestCase):
    def test_cached_views(self):
        model = RenderModel()
        rows = [ribbon(i) for i in range(395)]
        model.request(rows)
        frames = 0
        while model.todo:
            before = model.uploads
            model.pump(lambda *_: True, clock=lambda: 0)
            self.assertLessEqual(model.uploads - before, 16)
            frames += 1
        self.assertEqual(frames, 25)
        for _ in range(1000):
            self.assertFalse(model.request(rows))
        for view in (rows[::-1], rows[:1], [], rows):
            model.request(view)
            model.pump(lambda *_: self.fail('Cached view uploaded'), clock=lambda: 0)
        self.assertEqual(model.uploads, 395)

    def test_capacity_and_cancellation(self):
        model = RenderModel(3)
        for i in range(100):
            model.request([ribbon(i)])
            model.pump(lambda *_: True, clock=lambda: 0)
            self.assertLessEqual(len(model.cache), 3)
        model.request([ribbon(101), ribbon(102)])
        model.pump(lambda *_: True, max_uploads=1, clock=lambda: 0)
        model.request([])
        self.assertEqual(model.todo, [])
        self.assertEqual(model.active, [])

    def test_settle(self):
        gate = SettleGate()
        gate.change(0.0)
        gate.change(0.080)
        self.assertFalse(gate.ready(0.100))
        self.assertTrue(gate.ready(0.190))
        gate.change(0.240)
        self.assertTrue(gate.ready(0.251))
        gate.consume()
        self.assertFalse(gate.ready(1.0))

    def test_asset(self):
        self.assertIn('name = "localFreightPreview"', build_text('localFreightPreview'))
        with self.assertRaises(ValueError):
            build_text('invalid " name')

    def test_log_summary(self):
        text = ('[pid=1] DRAW PERF paths=4 uploads=4 reused=0 failed=0 pool=4 cpu_us=900 repeats_skipped=0\n'
                '[pid=1] DRAW PERF paths=4 uploads=0 reused=4 failed=0 pool=4 cpu_us=30 repeats_skipped=5\n'
                '[pid=2] DRAW PERF paths=2 uploads=2 reused=0 failed=0 pool=2 cpu_us=50 repeats_skipped=0\n')
        report = summarize(text, pid=1)
        self.assertEqual(report['batches'], 2)
        self.assertEqual(report['geometry_uploads'], 4)
        self.assertEqual(report['zero_upload_batches'], 1)


if __name__ == '__main__':
    unittest.main()
