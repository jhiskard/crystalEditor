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

function Get-StillPresentPaths {
    param(
        [string[]]$Paths
    )

    return @($Paths | Where-Object { Test-Path $_ })
}

function Get-CMakeMissingEntries {
    param(
        [string]$CmakeText,
        [string[]]$Entries
    )

    return @(
        $Entries | Where-Object {
            $escaped = [regex]::Escape($_)
            $CmakeText -notmatch $escaped
        }
    )
}

function Get-CMakePresentEntries {
    param(
        [string]$CmakeText,
        [string[]]$Entries
    )

    return @(
        $Entries | Where-Object {
            $escaped = [regex]::Escape($_)
            $CmakeText -match $escaped
        }
    )
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    $legacyRootFiles = @(
        "webassembly/src/mesh.cpp",
        "webassembly/src/mesh.h",
        "webassembly/src/mesh_group.cpp",
        "webassembly/src/mesh_group.h",
        "webassembly/src/lcrs_tree.cpp",
        "webassembly/src/lcrs_tree.h",
        "webassembly/src/mesh_manager.cpp",
        "webassembly/src/mesh_manager.h",
        "webassembly/src/model_tree.cpp",
        "webassembly/src/model_tree.h",
        "webassembly/src/mesh_detail.cpp",
        "webassembly/src/mesh_detail.h",
        "webassembly/src/mesh_group_detail.cpp",
        "webassembly/src/mesh_group_detail.h"
    )

    $requiredMigratedFiles = @(
        "webassembly/src/mesh/domain/lcrs_tree.cpp",
        "webassembly/src/mesh/domain/lcrs_tree.h",
        "webassembly/src/mesh/domain/mesh_entity.cpp",
        "webassembly/src/mesh/domain/mesh_entity.h",
        "webassembly/src/mesh/domain/mesh_group_entity.cpp",
        "webassembly/src/mesh/domain/mesh_group_entity.h",
        "webassembly/src/mesh/domain/mesh_repository_core.cpp",
        "webassembly/src/mesh/domain/mesh_repository_core.h",
        "webassembly/src/mesh/presentation/model_tree_panel.cpp",
        "webassembly/src/mesh/presentation/model_tree_panel.h",
        "webassembly/src/mesh/presentation/mesh_detail_panel.cpp",
        "webassembly/src/mesh/presentation/mesh_detail_panel.h",
        "webassembly/src/mesh/presentation/mesh_group_detail_panel.cpp",
        "webassembly/src/mesh/presentation/mesh_group_detail_panel.h"
    )

    $legacyStillPresent = Get-StillPresentPaths -Paths $legacyRootFiles
    $migratedMissing = Get-MissingPaths -Paths $requiredMigratedFiles

    $wbCorePath = "webassembly/cmake/modules/wb_core.cmake"
    $wbMeshPath = "webassembly/cmake/modules/wb_mesh.cmake"
    $wbCoreText = if (Test-Path $wbCorePath) { Get-Content -Raw -Encoding utf8 $wbCorePath } else { "" }
    $wbMeshText = if (Test-Path $wbMeshPath) { Get-Content -Raw -Encoding utf8 $wbMeshPath } else { "" }

    $wbMeshExpectedEntries = @(
        "webassembly/src/mesh/domain/lcrs_tree.cpp",
        "webassembly/src/mesh/domain/mesh_entity.cpp",
        "webassembly/src/mesh/domain/mesh_group_entity.cpp",
        "webassembly/src/mesh/domain/mesh_repository_core.cpp",
        "webassembly/src/mesh/presentation/model_tree_panel.cpp",
        "webassembly/src/mesh/presentation/mesh_detail_panel.cpp",
        "webassembly/src/mesh/presentation/mesh_group_detail_panel.cpp"
    )
    $legacyCmakeEntries = @(
        "webassembly/src/lcrs_tree.cpp",
        "webassembly/src/mesh.cpp",
        "webassembly/src/mesh_group.cpp",
        "webassembly/src/mesh_manager.cpp",
        "webassembly/src/model_tree.cpp",
        "webassembly/src/mesh_detail.cpp",
        "webassembly/src/mesh_group_detail.cpp"
    )

    $wbMeshMissingEntries = Get-CMakeMissingEntries -CmakeText $wbMeshText -Entries $wbMeshExpectedEntries
    $wbCoreLegacyEntries = Get-CMakePresentEntries -CmakeText $wbCoreText -Entries $legacyCmakeEntries
    $wbMeshLegacyEntries = Get-CMakePresentEntries -CmakeText $wbMeshText -Entries $legacyCmakeEntries

    $legacyIncludeMatchesRaw = & rg -n '#include\s+"(\.\./|\./)?(mesh|mesh_group|mesh_manager|mesh_detail|mesh_group_detail|model_tree|lcrs_tree)\.h"' webassembly/src 2>$null
    if ($LASTEXITCODE -notin @(0, 1)) {
        throw "Failed to execute legacy include scan for R4 migration check."
    }
    $legacyIncludeMatches = @($legacyIncludeMatchesRaw)

    $results = @(
        (New-Result "P17R4.legacy_root_files_removed" ($legacyStillPresent.Count -eq 0) $legacyStillPresent.Count 0),
        (New-Result "P17R4.migrated_files_present" ($migratedMissing.Count -eq 0) $migratedMissing.Count 0),
        (New-Result "P17R4.wb_mesh_sources_registered" ($wbMeshMissingEntries.Count -eq 0) $wbMeshMissingEntries.Count 0),
        (New-Result "P17R4.wb_core_legacy_sources_zero" ($wbCoreLegacyEntries.Count -eq 0) $wbCoreLegacyEntries.Count 0),
        (New-Result "P17R4.wb_mesh_legacy_sources_zero" ($wbMeshLegacyEntries.Count -eq 0) $wbMeshLegacyEntries.Count 0),
        (New-Result "P17R4.legacy_include_reference_zero" ($legacyIncludeMatches.Count -eq 0) $legacyIncludeMatches.Count 0)
    )

    Write-Host ("Phase 17-R4 Mesh Migration Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host "Phase 17-R4 migration snapshot:"
    Write-Host (" - legacy root files present: {0}" -f $legacyStillPresent.Count)
    Write-Host (" - migrated files missing: {0}" -f $migratedMissing.Count)
    Write-Host (" - wb_mesh expected entries missing: {0}" -f $wbMeshMissingEntries.Count)
    Write-Host (" - legacy include references: {0}" -f $legacyIncludeMatches.Count)

    if ($legacyStillPresent.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R4.legacy_root_files_removed] Remaining legacy files:"
        foreach ($entry in $legacyStillPresent) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($migratedMissing.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R4.migrated_files_present] Missing migrated files:"
        foreach ($entry in $migratedMissing) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($wbMeshMissingEntries.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R4.wb_mesh_sources_registered] Missing wb_mesh entries:"
        foreach ($entry in $wbMeshMissingEntries) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($wbCoreLegacyEntries.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R4.wb_core_legacy_sources_zero] Forbidden wb_core entries:"
        foreach ($entry in $wbCoreLegacyEntries) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($wbMeshLegacyEntries.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R4.wb_mesh_legacy_sources_zero] Forbidden wb_mesh entries:"
        foreach ($entry in $wbMeshLegacyEntries) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($legacyIncludeMatches.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R4.legacy_include_reference_zero] Legacy include references:"
        foreach ($entry in $legacyIncludeMatches) {
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
