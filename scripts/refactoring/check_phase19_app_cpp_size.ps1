$ErrorActionPreference = 'Stop'

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$appPath = Join-Path $repoRoot 'webassembly\src\app.cpp'
$maxLines = 400

if (-not (Test-Path $appPath)) {
    Write-Host "FAIL: app.cpp not found at $appPath"
    exit 1
}

$lineCount = (Get-Content -Path $appPath | Measure-Object -Line).Lines

if ($lineCount -gt $maxLines) {
    Write-Host "FAIL: app.cpp size gate exceeded ($lineCount > $maxLines lines)"
    exit 1
}

Write-Host "PASS: app.cpp size gate satisfied ($lineCount <= $maxLines lines)"
exit 0
