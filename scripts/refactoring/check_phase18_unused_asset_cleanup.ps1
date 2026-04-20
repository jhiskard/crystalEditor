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

function Get-TextOrEmpty {
    param(
        [string]$Path
    )

    if (-not (Test-Path $Path)) {
        return ""
    }

    return Get-Content -Raw -Encoding utf8 $Path
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    $atomsLegacyPath = "webassembly/src/atoms/legacy"
    $gitkeepFiles = @(
        Get-ChildItem -Path "webassembly/src" -Recurse -Filter ".gitkeep" -File -ErrorAction SilentlyContinue
    )
    $emptyDirs = @(
        Get-ChildItem -Path "webassembly/src" -Recurse -Directory -ErrorAction SilentlyContinue |
            Where-Object { (Get-ChildItem -Path $_.FullName -Force | Measure-Object).Count -eq 0 }
    )
    $dsStoreFiles = @(
        Get-ChildItem -Path "webassembly/src" -Recurse -Filter ".DS_Store" -File -ErrorAction SilentlyContinue
    )
    $vscodeDirs = @(
        Get-ChildItem -Path "webassembly/src" -Recurse -Directory -Force -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -eq ".vscode" }
    )

    $allowlistPath = "docs/refactoring/phase18/logs/unused_file_allowlist_phase18_latest.md"
    $allowlistText = Get-TextOrEmpty -Path $allowlistPath
    $allowlistExists = Test-Path $allowlistPath

    $candidateHeaders = @(
        "webassembly/src/density/infrastructure/density_render_bridge.h",
        "webassembly/src/density/infrastructure/rho_file_parser.h",
        "webassembly/src/density/infrastructure/slice_renderer.h"
    )

    $candidateResultEntries = @()
    $candidateViolationLines = @()
    foreach ($candidate in $candidateHeaders) {
        $exists = Test-Path $candidate
        if (-not $exists) {
            $candidateResultEntries += (New-Result ("P18W7.unused_candidate_{0}_handled" -f ([IO.Path]::GetFileNameWithoutExtension($candidate))) $true "deleted" "deleted or allowlisted")
            continue
        }

        $includePattern = ('#include\s+\".*{0}\"' -f [regex]::Escape([IO.Path]::GetFileName($candidate)))
        $includeMatches = Invoke-RgLines -Pattern $includePattern -Paths @("webassembly/src", "webassembly/tests") -Globs @("*.cpp", "*.h")
        $includeCount = $includeMatches.Count
        $listedInAllowlist = $allowlistText -match [regex]::Escape($candidate)

        $isHandled = ($includeCount -eq 0 -and $allowlistExists -and $listedInAllowlist)
        $candidateResultEntries += (New-Result ("P18W7.unused_candidate_{0}_handled" -f ([IO.Path]::GetFileNameWithoutExtension($candidate))) $isHandled ("exists, includes={0}, allowlisted={1}" -f $includeCount, $listedInAllowlist) "includes=0 + allowlisted")

        if (-not $isHandled) {
            $candidateViolationLines += $candidate
        }
    }

    $results = @(
        (New-Result "P18W7.unused_atoms_legacy_directory_absent" (-not (Test-Path $atomsLegacyPath)) ([int](Test-Path $atomsLegacyPath)) 0),
        (New-Result "P18W7.unused_empty_directory_count_zero" ($emptyDirs.Count -eq 0) $emptyDirs.Count 0),
        (New-Result "P18W7.unused_gitkeep_count_zero" ($gitkeepFiles.Count -eq 0) $gitkeepFiles.Count 0),
        (New-Result "P18W7.unused_ds_store_count_zero" ($dsStoreFiles.Count -eq 0) $dsStoreFiles.Count 0),
        (New-Result "P18W7.unused_vscode_directory_count_zero" ($vscodeDirs.Count -eq 0) $vscodeDirs.Count 0),
        (New-Result "P18W7.unused_allowlist_document_present" $allowlistExists ([int]$allowlistExists) 1)
    ) + $candidateResultEntries

    Write-Host ("Phase 18-W7 Unused Asset Cleanup Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    if ($emptyDirs.Count -gt 0) {
        Write-Host ""
        Write-Host "[P18W7.unused_empty_directory_count_zero] remaining directories:"
        foreach ($dir in $emptyDirs) {
            Write-Host (" - {0}" -f $dir.FullName)
        }
    }

    if ($gitkeepFiles.Count -gt 0) {
        Write-Host ""
        Write-Host "[P18W7.unused_gitkeep_count_zero] remaining files:"
        foreach ($file in $gitkeepFiles) {
            Write-Host (" - {0}" -f $file.FullName)
        }
    }

    if ($candidateViolationLines.Count -gt 0) {
        Write-Host ""
        Write-Host "[P18W7.unused_candidate_*_handled] unresolved candidate headers:"
        foreach ($candidate in $candidateViolationLines) {
            Write-Host (" - {0}" -f $candidate)
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
