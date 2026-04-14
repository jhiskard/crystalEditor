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

function Get-TextWithoutComments {
    param(
        [string]$Path
    )

    if (-not (Test-Path $Path)) {
        return ""
    }

    $raw = Get-Content -Raw -Encoding utf8 $Path
    $noBlock = [regex]::Replace($raw, "(?s)/\*.*?\*/", "")
    $noLine = [regex]::Replace($noBlock, "(?m)//.*$", "")
    return $noLine
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    $singletonMatchesRaw = & rg -n '\bMeshManager::Instance\s*\(' webassembly/src 2>$null
    if ($LASTEXITCODE -notin @(0, 1)) {
        throw "Failed to execute MeshManager singleton scan."
    }
    $singletonMatches = @($singletonMatchesRaw)

    $allowlistedPaths = @(
        "webassembly/src/mesh/domain/mesh_repository.cpp"
    )
    $allowlistedPathSet = @{}
    foreach ($path in $allowlistedPaths) {
        $allowlistedPathSet[$path] = $true
    }

    $allowlistedMatches = @()
    $violationMatches = @()
    foreach ($line in $singletonMatches) {
        $pathToken = ($line -split ":", 3)[0]
        $normalizedPath = ($pathToken -replace "\\", "/")
        if ($allowlistedPathSet.ContainsKey($normalizedPath)) {
            $allowlistedMatches += $line
        }
        else {
            $violationMatches += $line
        }
    }

    $runtimeHeaderPath = "webassembly/src/shell/runtime/workbench_runtime.h"
    $runtimeCppPath = "webassembly/src/shell/runtime/workbench_runtime.cpp"
    $runtimeHeaderText = Get-TextWithoutComments -Path $runtimeHeaderPath
    $runtimeCppText = Get-TextWithoutComments -Path $runtimeCppPath

    $legacyRuntimeReturnTypeCount = [regex]::Matches($runtimeHeaderText, '\bMeshManager&\s+MeshRepository\s*\(').Count
    $legacyRuntimePrintTreeCount = [regex]::Matches($runtimeCppText, 'MeshManager::PrintMeshTree\s*\(').Count

    $coreIncludeMatchesRaw = & rg -n '#include\s+".*mesh_repository_core\.h"' webassembly/src 2>$null
    if ($LASTEXITCODE -notin @(0, 1)) {
        throw "Failed to execute mesh_repository_core include scan."
    }
    $coreIncludeMatches = @($coreIncludeMatchesRaw)

    $coreIncludeAllowlist = @(
        "webassembly/src/mesh/domain/mesh_repository.h",
        "webassembly/src/mesh/domain/mesh_repository.cpp",
        "webassembly/src/mesh/domain/mesh_repository_core.cpp"
    )
    $coreIncludeAllowlistSet = @{}
    foreach ($path in $coreIncludeAllowlist) {
        $coreIncludeAllowlistSet[$path] = $true
    }

    $coreIncludeViolations = @()
    foreach ($line in $coreIncludeMatches) {
        $pathToken = ($line -split ":", 3)[0]
        $normalizedPath = ($pathToken -replace "\\", "/")
        if (-not $coreIncludeAllowlistSet.ContainsKey($normalizedPath)) {
            $coreIncludeViolations += $line
        }
    }

    $results = @(
        (New-Result "P17R4S.mesh_manager_instance_only_allowlisted" ($violationMatches.Count -eq 0) $violationMatches.Count 0),
        (New-Result "P17R4S.allowlisted_mesh_manager_instance_count_expected" ($allowlistedMatches.Count -eq 1) $allowlistedMatches.Count 1),
        (New-Result "P17R4S.runtime_mesh_repository_contract_legacy_type_zero" ($legacyRuntimeReturnTypeCount -eq 0) $legacyRuntimeReturnTypeCount 0),
        (New-Result "P17R4S.runtime_printmeshtree_legacy_static_call_zero" ($legacyRuntimePrintTreeCount -eq 0) $legacyRuntimePrintTreeCount 0),
        (New-Result "P17R4S.mesh_repository_core_include_boundary_enforced" ($coreIncludeViolations.Count -eq 0) $coreIncludeViolations.Count 0)
    )

    Write-Host ("Phase 17-R4 Mesh Singleton Usage Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host "Phase 17-R4 singleton snapshot:"
    Write-Host (" - MeshManager::Instance matches (total): {0}" -f $singletonMatches.Count)
    Write-Host (" - allowlisted matches: {0}" -f $allowlistedMatches.Count)
    Write-Host (" - violations: {0}" -f $violationMatches.Count)

    if ($violationMatches.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R4S.mesh_manager_instance_only_allowlisted] Violations:"
        foreach ($entry in $violationMatches) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($allowlistedMatches.Count -ne 1) {
        Write-Host ""
        Write-Host "[P17R4S.allowlisted_mesh_manager_instance_count_expected] Allowlisted matches:"
        foreach ($entry in $allowlistedMatches) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($coreIncludeViolations.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R4S.mesh_repository_core_include_boundary_enforced] Violations:"
        foreach ($entry in $coreIncludeViolations) {
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
