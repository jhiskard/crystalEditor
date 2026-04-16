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
        "webassembly/src/workspace/legacy/atoms_template_facade.cpp",
        "webassembly/src/workspace/legacy/atoms_template_facade.h"
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

    $wbWorkspacePath = "webassembly/cmake/modules/wb_workspace.cmake"
    $wbWorkspaceText = if (Test-Path $wbWorkspacePath) {
        Get-Content -Raw -Encoding utf8 $wbWorkspacePath
    }
    else {
        ""
    }

    $cmakeHasFacadeEntry = [regex]::IsMatch(
        $wbWorkspaceText,
        [regex]::Escape("webassembly/src/workspace/legacy/atoms_template_facade.cpp"))
    $cmakeHasLegacyEntry = [regex]::IsMatch(
        $wbWorkspaceText,
        [regex]::Escape("webassembly/src/atoms/atoms_template.cpp"))
    $legacyWbAtomsModuleStillPresent = Test-Path "webassembly/cmake/modules/wb_atoms.cmake"

    $singletonMatchesRaw = & rg -n '\bAtomsTemplate::Instance\s*\(' webassembly/src 2>$null
    if ($LASTEXITCODE -notin @(0, 1)) {
        throw "Failed to execute AtomsTemplate::Instance scan."
    }
    $singletonMatches = @($singletonMatchesRaw)
    $singletonReferencePresent = $singletonMatches.Count -gt 0

    $results = @(
        (New-Result "P17R5.legacy_atoms_template_files_removed" ($legacyFilesStillPresent.Count -eq 0) $legacyFilesStillPresent.Count 0),
        (New-Result "P17R5.facade_files_present" ($facadeFilesMissing.Count -eq 0) $facadeFilesMissing.Count 0),
        (New-Result "P17R5.atoms_root_code_file_count_zero" ($atomsRootCodeFiles.Count -eq 0) $atomsRootCodeFiles.Count 0),
        (New-Result "P17R5.legacy_atoms_template_include_zero" ($legacyHeaderIncludeMatches.Count -eq 0) $legacyHeaderIncludeMatches.Count 0),
        (New-Result "P17R5.facade_include_present" ($facadeHeaderIncludeMatches.Count -gt 0) $facadeHeaderIncludeMatches.Count "> 0"),
        (New-Result "P17R5.wb_workspace_facade_entry_present" $cmakeHasFacadeEntry ([int]$cmakeHasFacadeEntry) 1),
        (New-Result "P17R5.wb_workspace_legacy_entry_zero" (-not $cmakeHasLegacyEntry) ([int]$cmakeHasLegacyEntry) 0),
        (New-Result "P17R6.wb_atoms_module_removed" (-not $legacyWbAtomsModuleStillPresent) ([int]$legacyWbAtomsModuleStillPresent) 0),
        (New-Result "P17R5.singleton_instance_reference_present" $singletonReferencePresent $singletonMatches.Count "> 0")
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
    Write-Host (" - singleton reference count: {0}" -f $singletonMatches.Count)

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

    $violations = @($results | Where-Object { -not $_.Pass })
    if ($violations.Count -gt 0 -and $FailOnViolation) {
        exit 1
    }

    exit 0
}
finally {
    Pop-Location
}
