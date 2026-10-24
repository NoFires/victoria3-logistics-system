# Offline loader/overlay self-test. Uses a D3D11 harness (no Victoria 3).
# PASS means: proxy loaded, ImGui initialized, frames presented, and the
# rendered backbuffer contains the panel (verified by GPU readback).
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$dll = Join-Path $root 'dist\version.dll'
$harness = Join-Path $root 'native\build\tests\harness.exe'
$log = Join-Path $root 'dist\freight\freight.log'
$ppm = Join-Path $root 'dist\freight\readback.ppm'
if (Get-Process victoria3 -ErrorAction SilentlyContinue) { throw 'Close the game before the standalone log-writing test.' }
if (-not (Test-Path $dll)) { throw "missing $dll (run native\build.ps1)" }
if (-not (Test-Path $harness)) { throw "missing $harness (run native\build_tests.ps1)" }
# Preserve the previous log (it may be the only evidence from a game run).
if (Test-Path $log) { Move-Item $log "$log.before-selftest" -Force }
Remove-Item $ppm -ErrorAction SilentlyContinue
Get-Process harness -ErrorAction SilentlyContinue | Stop-Process -Force

$env:IRONROAD_READBACK = '1'
$p = Start-Process -FilePath $harness -ArgumentList $dll -WorkingDirectory $root -WindowStyle Hidden -PassThru
Start-Sleep -Seconds 3
Remove-Item Env:\IRONROAD_READBACK
$alive = -not $p.HasExited
if ($alive) { $p.WaitForExit(12000) | Out-Null }
if (-not $p.HasExited) { $p.Kill() }

$fail = @()
$text = if (Test-Path $log) { Get-Content $log -Raw } else { '' }
if ($text -notmatch 'ImGui ready') { $fail += 'ImGui did not initialize' }
if ($text -notmatch 'present #1 hr=0x00000000') { $fail += 'first Present did not succeed' }
if ($text -notmatch 'readback written') { $fail += 'readback was not written' }

$bright = 0
if (Test-Path $ppm) {
  $bytes = [System.IO.File]::ReadAllBytes($ppm)
  $nl = 0; $i = 0
  while ($i -lt $bytes.Length -and $nl -lt 3) { if ($bytes[$i] -eq 10) { $nl++ }; $i++ }
  $px = $bytes.Length - $i
  for ($j = $i; $j + 2 -lt $bytes.Length; $j += 3) {
    if ($bytes[$j] -gt 40 -or $bytes[$j+1] -gt 40 -or $bytes[$j+2] -gt 40) { $bright++ }
  }
  if ($bright -lt 5000) { $fail += "readback only $bright bright pixels (panel not drawn)" }
} else {
  $fail += 'readback.ppm missing'
}

if ($fail.Count -eq 0) {
  Write-Host "PASS: loader + ImGui overlay verified ($bright bright pixels)"
  exit 0
}
Write-Host "FAIL:"
$fail | ForEach-Object { Write-Host "  - $_" }
Write-Host "Log: $log"
exit 1

