# Exercise opt-in frame timing through the real proxy/core/D3D Present hook.
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
if (Get-Process victoria3 -ErrorAction SilentlyContinue) { throw 'Close the game before the standalone log-writing test.' }
$saved = $env:IRONROAD_FRAME_TIMING
try {
  foreach ($mode in @('1','2')) {
    $env:IRONROAD_FRAME_TIMING = $mode
    $p = Start-Process -FilePath (Join-Path $root 'native/build/tests/harness.exe') -ArgumentList (Join-Path $root 'dist/version.dll') -WorkingDirectory $root -WindowStyle Hidden -PassThru
    if (-not $p.WaitForExit(12000)) { throw 'Frame timing harness did not exit normally.' }
    if ($p.ExitCode -ne 0) { throw "Frame timing harness failed: $($p.ExitCode)" }
    $text = [IO.File]::ReadAllText((Join-Path $root 'dist/freight/freight.log'))
    $index = $text.LastIndexOf("loader attached: pid=$($p.Id) ")
    if ($index -lt 0) { throw 'No current timing harness evidence.' }
    $text = $text.Substring($index)
    $capture = Join-Path $root "native/build/frame-timing-mode$mode.log"
    [IO.File]::WriteAllText($capture,$text,(New-Object Text.UTF8Encoding($false)))
    if ($mode -eq '1') {
      if ($text -notmatch 'FRAME PERF enabled' -or $text -notmatch 'FRAME PERF thread=') { throw 'Opt-in timing did not capture complete raw batches.' }
      & python (Join-Path $root 'tools/frame_timing.py') --log $capture
      if ($LASTEXITCODE -ne 0) { throw 'Actual hook produced invalid/discontinuous raw timing.' }
    } elseif ($text -match 'FRAME PERF') { throw 'Timing accepted an invalid opt-in value.' }
  }
  Write-Output 'PASS: actual D3D Present timing batches parse exactly; invalid opt-in remains disabled.'
} finally {
  $env:IRONROAD_FRAME_TIMING = $saved
}
