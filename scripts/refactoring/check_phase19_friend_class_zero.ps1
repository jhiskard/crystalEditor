$ErrorActionPreference = 'Stop'

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$sourceRoot = Join-Path $repoRoot 'webassembly\src'

$matches = Get-ChildItem -Path $sourceRoot -Recurse -File -Include *.h,*.hpp,*.cpp,*.cc,*.cxx |
    Select-String -Pattern 'friend\s+class'

if ($matches) {
    Write-Host "FAIL: found friend class declarations in webassembly/src"
    $matches | ForEach-Object {
        Write-Host ("  - {0}:{1}: {2}" -f $_.Path, $_.LineNumber, $_.Line.Trim())
    }
    exit 1
}

Write-Host 'PASS: friend class declarations are zero in webassembly/src'
exit 0

