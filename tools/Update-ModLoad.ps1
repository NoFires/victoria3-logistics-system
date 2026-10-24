# Victoria 3 1.13 content loader: enabledMods is an array of {path: string}.
param(
  [Parameter(Mandatory=$true)][string]$ContentFile,
  [Parameter(Mandatory=$true)][string]$ModPath,
  [switch]$Remove
)
$ErrorActionPreference = 'Stop'
$normalized = [IO.Path]::GetFullPath($ModPath).Replace('\','/').TrimEnd('/')
if (Test-Path -LiteralPath $ContentFile) {
  $settings = Get-Content -LiteralPath $ContentFile -Raw | ConvertFrom-Json
} else {
  $settings = [pscustomobject]@{ disabledDLC=@(); enabledMods=@(); enabledUGC=@() }
}
if (-not $settings.PSObject.Properties['enabledMods']) {
  $settings | Add-Member -NotePropertyName enabledMods -NotePropertyValue @()
}
$mods = @($settings.enabledMods | Where-Object {
  # Keep unknown entries untouched; only remove this exact folder's entry.
  $entryPath = if ($_ -is [string]) { $_ } else { $_.path }
  -not ($entryPath -and $entryPath.Replace('\','/').TrimEnd('/') -ieq $normalized)
})
if (-not $Remove) { $mods += [pscustomobject]@{ path=$normalized } }
$settings.enabledMods = @($mods)
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $ContentFile) | Out-Null
if (Test-Path -LiteralPath $ContentFile) {
  $snapshot = $ContentFile + '.ironroad-' + [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss-fffffff') + '.bak'
  Copy-Item -LiteralPath $ContentFile -Destination $snapshot
}
$temporary = $ContentFile + '.ironroad-tmp'
[IO.File]::WriteAllText($temporary, ($settings | ConvertTo-Json -Depth 64), (New-Object Text.UTF8Encoding $false))
Move-Item -LiteralPath $temporary -Destination $ContentFile -Force
Write-Host "Updated content loader ($(@($settings.enabledMods).Count) enabled mods): $ContentFile"
