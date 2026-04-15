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

function Get-MissingPaths {
    param(
        [string[]]$Paths
    )

    return @($Paths | Where-Object { -not (Test-Path $_) })
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    $removedLegacyFiles = @(
        "webassembly/src/atoms/atoms_template.cpp",
        "webassembly/src/atoms/atoms_template.h"
    )
    $requiredFacadeFiles = @(
        "webassembly/src/atoms/legacy/atoms_template_facade.cpp",
        "webassembly/src/atoms/legacy/atoms_template_facade.h"
    )

    $legacyFilesStillPresent = @($removedLegacyFiles | Where-Object { Test-Path $_ })
    $facadeFilesMissing = Get-MissingPaths -Paths $requiredFacadeFiles

    $atomsRootCodeFiles = @(
        Get-ChildItem -Path "webassembly/src/atoms" -File |
            Where-Object { $_.Extension -in @(".cpp", ".h") } |
            ForEach-Object { $_.FullName }
    )

    $legacyHeaderIncludeMatchesRaw = & rg -n '#include\s+".*atoms_template\.h"' webassembly/src 2>$null
    if ($LASTEXITCODE -notin @(0, 1)) {
        throw "Failed to execute atoms_template.h include scan."
    }
    $legacyHeaderIncludeMatches = @($legacyHeaderIncludeMatchesRaw)

    $facadeHeaderIncludeMatchesRaw = & rg -n '#include\s+".*atoms_template_facade\.h"' webassembly/src 2>$null
    if ($LASTEXITCODE -notin @(0, 1)) {
        throw "Failed to execute atoms_template_facade.h include scan."
    }
    $facadeHeaderIncludeMatches = @($facadeHeaderIncludeMatchesRaw)

    $wbAtomsPath = "webassembly/cmake/modules/wb_atoms.cmake"
    $wbAtomsText = if (Test-Path $wbAtomsPath) {
        Get-Content -Raw -Encoding utf8 $wbAtomsPath
    }
    else {
        ""
    }

    $cmakeHasFacadeEntry = [regex]::IsMatch(
        $wbAtomsText,
        [regex]::Escape("webassembly/src/atoms/legacy/atoms_template_facade.cpp"))
    $cmakeHasLegacyEntry = [regex]::IsMatch(
        $wbAtomsText,
        [regex]::Escape("webassembly/src/atoms/atoms_template.cpp"))

    $singletonMatchesRaw = & rg -n '\bAtomsTemplate::Instance\s*\(' webassembly/src 2>$null
    if ($LASTEXITCODE -notin @(0, 1)) {
        throw "Failed to execute AtomsTemplate::Instance scan."
    }
    $singletonMatches = @($singletonMatchesRaw)

    $singletonAllowlist = @(
        "webassembly/src/atoms/legacy/atoms_template_facade.cpp",
        "webassembly/src/shell/runtime/workbench_runtime.cpp"
    )
    $singletonAllowlistSet = @{}
    foreach ($path in $singletonAllowlist) {
        $singletonAllowlistSet[$path] = $true
    }

    $singletonViolations = @()
    foreach ($line in $singletonMatches) {
        $pathToken = ($line -split ":", 3)[0]
        $normalizedPath = ($pathToken -replace "\\", "/")
        if (-not $singletonAllowlistSet.ContainsKey($normalizedPath)) {
            $singletonViolations += $line
        }
    }

    $results = @(
        (New-Result "P17R5.legacy_atoms_template_files_removed" ($legacyFilesStillPresent.Count -eq 0) $legacyFilesStillPresent.Count 0),
        (New-Result "P17R5.facade_files_present" ($facadeFilesMissing.Count -eq 0) $facadeFilesMissing.Count 0),
        (New-Result "P17R5.atoms_root_code_file_count_zero" ($atomsRootCodeFiles.Count -eq 0) $atomsRootCodeFiles.Count 0),
        (New-Result "P17R5.legacy_atoms_template_include_zero" ($legacyHeaderIncludeMatches.Count -eq 0) $legacyHeaderIncludeMatches.Count 0),
        (New-Result "P17R5.facade_include_present" ($facadeHeaderIncludeMatches.Count -gt 0) $facadeHeaderIncludeMatches.Count "> 0"),
        (New-Result "P17R5.wb_atoms_facade_entry_present" $cmakeHasFacadeEntry ([int]$cmakeHasFacadeEntry) 1),
        (New-Result "P17R5.wb_atoms_legacy_entry_zero" (-not $cmakeHasLegacyEntry) ([int]$cmakeHasLegacyEntry) 0),
        (New-Result "P17R5.singleton_instance_outside_allowlist_zero" ($singletonViolations.Count -eq 0) $singletonViolations.Count 0)
    )

    Write-Host ("Phase 17-R5 Atoms Migration Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host "Phase 17-R5 atoms migration snapshot:"
    Write-Host (" - root atoms code files: {0}" -f $atomsRootCodeFiles.Count)
    Write-Host (" - legacy header include matches: {0}" -f $legacyHeaderIncludeMatches.Count)
    Write-Host (" - facade header include matches: {0}" -f $facadeHeaderIncludeMatches.Count)
    Write-Host (" - singleton allowlist violations: {0}" -f $singletonViolations.Count)

    if ($legacyFilesStillPresent.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R5.legacy_atoms_template_files_removed] Remaining legacy files:"
        foreach ($entry in $legacyFilesStillPresent) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($facadeFilesMissing.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R5.facade_files_present] Missing facade files:"
        foreach ($entry in $facadeFilesMissing) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($atomsRootCodeFiles.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R5.atoms_root_code_file_count_zero] Unexpected root code files:"
        foreach ($entry in $atomsRootCodeFiles) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($legacyHeaderIncludeMatches.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R5.legacy_atoms_template_include_zero] Legacy include references:"
        foreach ($entry in $legacyHeaderIncludeMatches) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($singletonViolations.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R5.singleton_instance_outside_allowlist_zero] Violations:"
        foreach ($entry in $singletonViolations) {
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
