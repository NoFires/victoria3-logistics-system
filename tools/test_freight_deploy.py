"""Test deploy/upgrade/uninstall in a sandbox, redirecting legacy Documents only."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def run():
    build = ROOT / "native/build/tests"
    build.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="freight-deploy-", dir=build) as temp:
        repo = Path(temp)
        docs = repo / "documents"
        docs.mkdir()
        source = (ROOT / "deploy.ps1").read_text(encoding="utf-8")
        old = "([Environment]::GetFolderPath('MyDocuments'))"
        assert source.count(old) == 1
        # Only redirect legacy cleanup paths; execute every install/uninstall branch unchanged.
        script = repo / "deploy.ps1"
        script.write_text(source.replace(old, "'" + str(docs).replace("'", "''") + "'"), encoding="utf-8")
        (repo / "dist/freight").mkdir(parents=True)
        proxy, core = b"Ironroad freight bootstrap test", b"Ironroad freight core test"
        (repo / "dist/version.dll").write_bytes(proxy)
        (repo / "dist/freight/freight.dll").write_bytes(core)
        game = repo / "game space"
        binary = game / "binaries"
        (binary / "freight").mkdir(parents=True)
        foreign = b"foreign version library"
        (binary / "version.dll").write_bytes(foreign)
        ini = binary / "freight/freight.ini"
        ini.write_bytes(b"keep=1")
        env = dict(os.environ)
        env["PSModulePath"] = str(Path(os.environ["SystemRoot"]) / "System32/WindowsPowerShell/v1.0/Modules")
        def deploy(*args, succeeds=True):
            result = subprocess.run(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass",
                                     "-File", str(script), "-GameRoot", str(game), *args],
                                    capture_output=True, timeout=20, env=env)
            assert (result.returncode == 0) == succeeds, result.stdout + result.stderr
        deploy()
        assert (binary / "version.dll").read_bytes() == proxy
        assert (binary / "freight/freight.dll").read_bytes() == core
        assert (binary / "version.dll.ironroad-backup").read_bytes() == foreign
        deploy()
        assert ini.read_bytes() == b"keep=1"
        deploy("-Uninstall")
        assert (binary / "version.dll").read_bytes() == foreign
        assert not (binary / "freight/freight.dll").exists()
        assert ini.read_bytes() == b"keep=1"
        (binary / "freight/freight.dll").write_bytes(b"unrelated payload")
        deploy(succeeds=False)
        assert (binary / "version.dll").read_bytes() == foreign
        assert (binary / "freight/freight.dll").read_bytes() == b"unrelated payload"
        print("PASS sandbox deployment: install, upgrade, uninstall, backup restoration, config retention, foreign payload refusal")


if __name__ == "__main__":
    run()
