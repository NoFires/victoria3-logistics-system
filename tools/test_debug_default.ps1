# Verify the product does not open a standalone panel on startup.
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
if (Get-Process victoria3 -ErrorAction SilentlyContinue) { throw 'Close the game before the standalone log-writing test.' }
$debugSaved = $env:IRONROAD_DEBUG_UI
$readbackSaved = $env:IRONROAD_READBACK
try {
  Remove-Item Env:IRONROAD_DEBUG_UI -ErrorAction SilentlyContinue
  Remove-Item Env:IRONROAD_READBACK -ErrorAction SilentlyContinue
  $p = Start-Process -FilePath (Join-Path $root 'native/build/tests/harness.exe') -ArgumentList (Join-Path $root 'dist/version.dll') -WorkingDirectory $root -WindowStyle Hidden -PassThru
  if (-not $p.WaitForExit(12000)) { throw 'Default visibility harness did not exit normally.' }
  if ($p.ExitCode -ne 0) { throw "Default visibility harness failed: $($p.ExitCode)" }
  $text = Get-Content -LiteralPath (Join-Path $root 'dist/freight/freight.log') -Raw
  $marker = "loader attached: pid=$($p.Id) "
  $index = $text.LastIndexOf($marker)
  if ($index -lt 0) { throw 'No current harness startup evidence.' }
  $text = $text.Substring($index)
  if ($text -notmatch 'ImGui ready' -or $text -notmatch 'present #1 hr=0x00000000') { throw 'Loader/backend did not initialize.' }
  if ($text -notmatch 'present #\d+ hr=0x00000000 imgui=1 vtx=0') { throw 'No hidden-panel render evidence.' }
  if ($text -match 'present #\d+ .*vtx=[1-9]\d*') { throw 'Standalone debug panel was unexpectedly visible.' }
  Write-Output "PASS: PID $($p.Id) initialized the backend with zero UI vertices; debug window is closed by default."
} finally {
  $env:IRONROAD_DEBUG_UI = $debugSaved
  $env:IRONROAD_READBACK = $readbackSaved
}
