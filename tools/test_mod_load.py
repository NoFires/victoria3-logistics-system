"""Exercise loader configuration migrations without changing the user's config."""
import json
from pathlib import Path
import subprocess
import tempfile
import unittest

SCRIPT = Path(__file__).resolve().with_name('Update-ModLoad.ps1')


class ModLoaderTest(unittest.TestCase):
    def test_preserve_enable_idempotent_remove(self):
        with tempfile.TemporaryDirectory(prefix='ironroad-load-') as directory:
            config = Path(directory) / 'content_load.json'
            mod = Path(directory).resolve() / 'mod/ironroad_arrow'
            other = {'path': 'D:/unrelated/mod', 'custom': [1, 2]}
            original = {'disabledDLC': [{'paradoxAppId': 'example'}],
                        'enabledMods': [other], 'enabledUGC': [], 'custom': {'keep': True}}
            config.write_text(json.dumps(original), encoding='utf-8')

            def run(*args):
                result = subprocess.run(['powershell', '-NoProfile', '-ExecutionPolicy', 'Bypass',
                                         '-File', str(SCRIPT), '-ContentFile', str(config),
                                         '-ModPath', str(mod), *args], capture_output=True, text=True)
                self.assertEqual(result.returncode, 0, result.stderr)
                return json.loads(config.read_text(encoding='utf-8-sig'))

            first = run()
            self.assertEqual(first['enabledMods'], [other, {'path': mod.as_posix()}])
            second = run()
            self.assertEqual(second, first)
            self.assertEqual(run('-Remove'), original)
            self.assertGreaterEqual(len(list(config.parent.glob('*.bak'))), 3)

    def test_empty_configuration(self):
        with tempfile.TemporaryDirectory(prefix='ironroad-load-') as directory:
            config = Path(directory) / 'new/content_load.json'
            mod = Path(directory).resolve() / 'mod'
            result = subprocess.run(['powershell', '-NoProfile', '-ExecutionPolicy', 'Bypass',
                                     '-File', str(SCRIPT), '-ContentFile', str(config),
                                     '-ModPath', str(mod)], capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            data = json.loads(config.read_text(encoding='utf-8'))
            self.assertEqual(data['enabledMods'], [{'path': mod.as_posix()}])
            self.assertEqual(data['disabledDLC'], [])


if __name__ == '__main__':
    unittest.main()
