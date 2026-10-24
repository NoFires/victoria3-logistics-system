"""Exercise the real proxy in isolated processes without loading the game core."""
import argparse
import ctypes
from ctypes import wintypes as w
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
GPP = Path(r"E:\Program\C Compiler\MSYS2\ucrt64\bin\g++.exe")
EXPORTS = ("GetFileVersionInfoA GetFileVersionInfoW GetFileVersionInfoSizeA "
           "GetFileVersionInfoSizeW VerQueryValueA VerQueryValueW VerFindFileA "
           "VerFindFileW VerInstallFileA VerInstallFileW VerLanguageNameA "
           "VerLanguageNameW GetFileVersionInfoExA GetFileVersionInfoExW "
           "GetFileVersionInfoSizeExA GetFileVersionInfoSizeExW "
           "GetFileVersionInfoByHandle").split()


def child(stage, available):
    kernel = ctypes.WinDLL("kernel32", use_last_error=True)
    kernel.GetModuleHandleW.argtypes = [w.LPCWSTR]
    kernel.GetModuleHandleW.restype = w.HMODULE
    kernel.GetProcAddress.argtypes = [w.HMODULE, ctypes.c_char_p]
    kernel.GetProcAddress.restype = ctypes.c_void_p
    proxy = ctypes.WinDLL(str(stage / "version.dll"))
    system = ctypes.WinDLL(str(Path(os.environ["SystemRoot"]) / "System32/version.dll"))
    for name in EXPORTS:
        assert kernel.GetProcAddress(proxy._handle, name.encode()), name
    for module in [proxy, system]:
        module.GetFileVersionInfoSizeW.argtypes = [w.LPCWSTR, ctypes.POINTER(w.DWORD)]
        module.GetFileVersionInfoSizeW.restype = w.DWORD
        module.GetFileVersionInfoW.argtypes = [w.LPCWSTR, w.DWORD, w.DWORD, ctypes.c_void_p]
        module.GetFileVersionInfoW.restype = w.BOOL
    target = str(Path(os.environ["SystemRoot"]) / "System32/kernel32.dll")
    size = proxy.GetFileVersionInfoSizeW(target, ctypes.byref(w.DWORD()))
    assert size > 0 and size == system.GetFileVersionInfoSizeW(target, ctypes.byref(w.DWORD()))
    a, b = ctypes.create_string_buffer(size), ctypes.create_string_buffer(size)
    assert proxy.GetFileVersionInfoW(target, 0, size, a)
    assert system.GetFileVersionInfoW(target, 0, size, b)
    assert a.raw == b.raw, "forwarded version data differs"
    log = stage / "freight/loader.log"
    deadline = time.monotonic() + 5
    while time.monotonic() < deadline:
        if log.exists() and ("loaded freight.dll" if available else "failed") in log.read_text():
            break
        time.sleep(0.02)
    else:
        raise AssertionError("loader did not finish or report failure")
    module = kernel.GetModuleHandleW(str(stage / "freight/freight.dll"))
    if available:
        assert module, "payload was not loaded from freight subdirectory"
        fn = kernel.GetProcAddress(module, b"FreightLoaderTest")
        assert fn and ctypes.WINFUNCTYPE(ctypes.c_int)(fn)() == 83
    else:
        assert not module, "invalid payload was loaded"
    print("PASS", stage.name, "exports/system parity and isolated payload loading")


def runtime_child(stage):
    # Unknown Python EXE keeps native game calls disabled; exercise real resource/migration code.
    ctypes.WinDLL(str(stage / "version.dll"))
    log = stage / "freight/freight.log"
    deadline = time.monotonic() + 8
    while time.monotonic() < deadline:
        if log.exists() and "self-contained DLL resources prepared" in log.read_text(errors="replace"):
            break
        time.sleep(0.02)
    else:
        raise AssertionError("real core did not prepare its runtime directory")
    assert (stage / "freight/freight.ini").read_bytes() == b"market_override=123\n"
    assert (stage / "freight/ironroad.ini").read_bytes() == b"market_override=123\n"
    assert (stage / "freight/data/splines.bin").is_file()
    # A current file must beat the still-present legacy file on the next process start.
    (stage / "freight/freight.ini").write_bytes(b"market_override=321\n")
    print("PASS real core: colocated log/cache and non-destructive preference migration")


def current_child(stage):
    ctypes.WinDLL(str(stage / "version.dll"))
    log = stage / "freight/freight.log"
    deadline = time.monotonic() + 8
    while time.monotonic() < deadline:
        if log.exists() and log.read_text(errors="replace").count("self-contained DLL resources prepared") >= 2:
            break
        time.sleep(0.02)
    else:
        raise AssertionError("second core initialization did not finish")
    assert (stage / "freight/freight.ini").read_bytes() == b"market_override=321\n"
    assert (stage / "freight/ironroad.ini").read_bytes() == b"market_override=123\n"
    print("PASS real core: existing freight.ini takes precedence over legacy preferences")


def runtime():
    with tempfile.TemporaryDirectory(prefix="freight-runtime-", dir=ROOT / "native/build/tests") as temp:
        stage = Path(temp)
        (stage / "freight").mkdir()
        shutil.copy2(ROOT / "dist/version.dll", stage / "version.dll")
        shutil.copy2(ROOT / "dist/freight/freight.dll", stage / "freight/freight.dll")
        (stage / "freight/ironroad.ini").write_bytes(b"market_override=123\n")
        for mode in ["--runtime-child", "--current-child"]:
            subprocess.run([sys.executable, str(Path(__file__).resolve()), mode, str(stage)],
                           cwd=stage.parent, check=True, timeout=15)


def run():
    os.environ["PATH"] = str(GPP.parent) + os.pathsep + os.environ["PATH"]
    build = ROOT / "native/build/tests"
    build.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="freight-loader-", dir=build) as tmp:
        base = Path(tmp)
        proxy = base / "version.dll"
        subprocess.run([str(GPP), "-std=c++20", "-O2", "-shared", "-static",
                        "-DUNICODE", "-D_UNICODE", "-D_WIN32_WINNT=0x0A00",
                        str(ROOT / "native/src/proxy.cpp"), "-o", str(proxy)], check=True)
        stub = base / "stub.cpp"
        stub.write_text('extern "C" __declspec(dllexport) int FreightLoaderTest() { return 83; }')
        payload = base / "stub.dll"
        subprocess.run([str(GPP), "-shared", "-static", str(stub), "-o", str(payload)], check=True)
        decoy = base / "cwd"
        decoy.mkdir()
        (decoy / "freight.dll").write_bytes(b"invalid decoy must not be loaded")
        for name in ["valid space 中文", "missing", "corrupt"]:
            stage = base / name
            (stage / "freight").mkdir(parents=True)
            shutil.copy2(proxy, stage / "version.dll")
            if name.startswith("valid"):
                shutil.copy2(payload, stage / "freight/freight.dll")
            elif name == "corrupt":
                (stage / "freight/freight.dll").write_bytes(b"invalid DLL")
            subprocess.run([sys.executable, str(Path(__file__).resolve()), "--child", str(stage),
                            "--available" if name.startswith("valid") else "--unavailable"],
                           cwd=decoy, check=True, timeout=12)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--child", type=Path)
    parser.add_argument("--available", action="store_true")
    parser.add_argument("--unavailable", action="store_true")
    parser.add_argument("--runtime", action="store_true")
    parser.add_argument("--runtime-child", type=Path)
    parser.add_argument("--current-child", type=Path)
    args = parser.parse_args()
    if args.runtime_child:
        runtime_child(args.runtime_child)
    elif args.current_child:
        current_child(args.current_child)
    elif args.runtime:
        runtime()
    elif args.child:
        child(args.child, args.available)
    else:
        run()
