param(
    [switch]$FailOnViolation = $true
)

$ErrorActionPreference = "Stop"

function New-Result {
    param(
        [string]$Name,
        [bool]$Pass,
        [object]$Current,
        [object]$Target
    )

    return @{
        Name = $Name
        Pass = $Pass
        Current = $Current
        Target = $Target
    }
}

function Invoke-RgLines {
    param(
        [string]$Pattern,
        [string[]]$Paths,
        [string[]]$Globs
    )

    $args = @("--line-number", $Pattern) + $Paths
    foreach ($glob in $Globs) {
        $args += @("-g", $glob)
    }

    $lines = @(& rg @args 2>$null)
    if ($LASTEXITCODE -gt 1) {
        throw "rg failed for pattern: $Pattern"
    }
    if ($LASTEXITCODE -eq 1) {
        return @()
    }
    return $lines
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    $legacyRuntimeSymbols = Invoke-RgLines -Pattern '\bLegacyAtomsRuntime\b' -Paths @("webassembly/src") -Globs @("*.h", "*.hpp", "*.cpp", "*.cc", "*.cxx")
    $legacyRuntimeIncludes = Invoke-RgLines -Pattern 'workspace/runtime/legacy_atoms_runtime\.h' -Paths @("webassembly/src", "webassembly/cmake/modules") -Globs @("*.h", "*.hpp", "*.cpp", "*.cc", "*.cxx", "*.cmake", "*.txt")

    $legacyRuntimeFiles = @(
        "webassembly/src/workspace/runtime/legacy_atoms_runtime.h",
        "webassembly/src/workspace/runtime/legacy_atoms_runtime.cpp"
    )
    $remainingLegacyRuntimeFiles = @(
        $legacyRuntimeFiles |
            Where-Object { Test-Path $_ }
    )

    $results = @(
        (New-Result "P19W9.legacy_runtime_symbol_count_zero" ($legacyRuntimeSymbols.Count -eq 0) $legacyRuntimeSymbols.Count 0),
        (New-Result "P19W9.legacy_runtime_include_count_zero" ($legacyRuntimeIncludes.Count -eq 0) $legacyRuntimeIncludes.Count 0),
        (New-Result "P19W9.legacy_runtime_file_count_zero" ($remainingLegacyRuntimeFiles.Count -eq 0) $remainingLegacyRuntimeFiles.Count 0)
    )

    Write-Host ("Phase 19 Legacy Runtime Zero Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    if ($legacyRuntimeSymbols.Count -gt 0) {
        Write-Host ""
        Write-Host "[P19W9.legacy_runtime_symbol_count_zero] remaining symbols:"
        foreach ($line in $legacyRuntimeSymbols) {
            Write-Host (" - {0}" -f $line)
        }
    }

    if ($legacyRuntimeIncludes.Count -gt 0) {
        Write-Host ""
        Write-Host "[P19W9.legacy_runtime_include_count_zero] remaining includes:"
        foreach ($line in $legacyRuntimeIncludes) {
            Write-Host (" - {0}" -f $line)
        }
    }

    if ($remainingLegacyRuntimeFiles.Count -gt 0) {
        Write-Host ""
        Write-Host "[P19W9.legacy_runtime_file_count_zero] remaining files:"
        foreach ($entry in $remainingLegacyRuntimeFiles) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    $violations = @($results | Where-Object { -not $_.Pass })
    if ($violations.Count -gt 0 -and $FailOnViolation) {
        exit 1
    }

    exit 0
}
finally {
    Pop-Location
}
