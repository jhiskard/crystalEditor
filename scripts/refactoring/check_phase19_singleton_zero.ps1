$ErrorActionPreference = 'Stop'

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$sourceRoot = Join-Path $repoRoot 'webassembly\src'
$sourceFiles = Get-ChildItem -Path $sourceRoot -Recurse -File -Include *.h,*.hpp,*.cpp,*.cc,*.cxx

$singletonDeclMatches = $sourceFiles |
    Where-Object { $_.FullName -notmatch '[\\/]macro[\\/]singleton_macro\.h$' } |
    Select-String -Pattern 'DECLARE_SINGLETON\s*\('

$instanceSymbolMatches = $sourceFiles |
    Where-Object { $_.FullName -notmatch '[\\/]macro[\\/]singleton_macro\.h$' } |
    Select-String -Pattern '\bInstance\s*\('

$failed = $false

if ($singletonDeclMatches) {
    Write-Host 'FAIL: DECLARE_SINGLETON usage remains in production source files'
    $singletonDeclMatches | ForEach-Object {
        Write-Host ("  - {0}:{1}: {2}" -f $_.Path, $_.LineNumber, $_.Line.Trim())
    }
    $failed = $true
}

if ($instanceSymbolMatches) {
    Write-Host 'FAIL: Instance() symbols remain in production source files'
    $instanceSymbolMatches | ForEach-Object {
        Write-Host ("  - {0}:{1}: {2}" -f $_.Path, $_.LineNumber, $_.Line.Trim())
    }
    $failed = $true
}

if ($failed) {
    exit 1
}

Write-Host 'PASS: phase19 singleton symbols are zero'
exit 0
