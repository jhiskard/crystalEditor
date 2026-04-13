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

function New-OwnershipEntry {
    param(
        [string]$Owner,
        [string]$Category,
        [string]$Action
    )

    return [pscustomobject]@{
        Owner = $Owner
        Category = $Category
        Action = $Action
    }
}

function Get-RootRelativeCodeFilePaths {
    param(
        [string]$Path,
        [string]$Prefix
    )

    if (-not (Test-Path $Path)) {
        return @()
    }

    return @(
        Get-ChildItem -Path $Path -File |
            Where-Object { $_.Extension -in ".cpp", ".h" } |
            Sort-Object Name |
            ForEach-Object {
                if ([string]::IsNullOrEmpty($Prefix)) {
                    $_.Name
                }
                else {
                    "{0}/{1}" -f $Prefix, $_.Name
                }
            }
    )
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    # Phase 17-R0 ownership map snapshot (2026-04-08)
    $ownershipMap = [ordered]@{
        "app.cpp" = (New-OwnershipEntry "shell" "facade_keep" "facade 유지")
        "app.h" = (New-OwnershipEntry "shell" "facade_keep" "facade 유지")
        "main.cpp" = (New-OwnershipEntry "shell/runtime" "facade_keep" "entrypoint 유지")

        "bind_function.cpp" = (New-OwnershipEntry "platform/wasm" "dismantle_target" "치환 후 삭제")
        "custom_ui.cpp" = (New-OwnershipEntry "shell/presentation" "dismantle_target" "이관 후 삭제")
        "custom_ui.h" = (New-OwnershipEntry "shell/presentation" "dismantle_target" "이관 후 삭제")
        "file_loader.cpp" = (New-OwnershipEntry "io/application,io/infrastructure,platform/browser,platform/worker" "dismantle_target" "분해 후 삭제")
        "file_loader.h" = (New-OwnershipEntry "io/application,io/infrastructure,platform/browser,platform/worker" "dismantle_target" "분해 후 삭제")
        "font_manager.cpp" = (New-OwnershipEntry "shell/presentation|platform" "dismantle_target" "이관 후 삭제")
        "font_manager.h" = (New-OwnershipEntry "shell/presentation|platform" "dismantle_target" "이관 후 삭제")
        "image.cpp" = (New-OwnershipEntry "render/infrastructure" "dismantle_target" "이관 후 삭제")
        "image.h" = (New-OwnershipEntry "render/infrastructure" "dismantle_target" "이관 후 삭제")
        "lcrs_tree.cpp" = (New-OwnershipEntry "mesh/domain" "dismantle_target" "이관 후 삭제")
        "lcrs_tree.h" = (New-OwnershipEntry "mesh/domain" "dismantle_target" "이관 후 삭제")
        "mesh.cpp" = (New-OwnershipEntry "mesh/domain" "dismantle_target" "이관 후 삭제")
        "mesh.h" = (New-OwnershipEntry "mesh/domain" "dismantle_target" "이관 후 삭제")
        "mesh_detail.cpp" = (New-OwnershipEntry "mesh/presentation" "dismantle_target" "이관 후 삭제")
        "mesh_detail.h" = (New-OwnershipEntry "mesh/presentation" "dismantle_target" "이관 후 삭제")
        "mesh_group.cpp" = (New-OwnershipEntry "mesh/domain" "dismantle_target" "이관 후 삭제")
        "mesh_group.h" = (New-OwnershipEntry "mesh/domain" "dismantle_target" "이관 후 삭제")
        "mesh_group_detail.cpp" = (New-OwnershipEntry "mesh/presentation" "dismantle_target" "이관 후 삭제")
        "mesh_group_detail.h" = (New-OwnershipEntry "mesh/presentation" "dismantle_target" "이관 후 삭제")
        "mesh_manager.cpp" = (New-OwnershipEntry "mesh/application,mesh/domain" "dismantle_target" "분해 후 삭제")
        "mesh_manager.h" = (New-OwnershipEntry "mesh/application,mesh/domain" "dismantle_target" "분해 후 삭제")
        "model_tree.cpp" = (New-OwnershipEntry "mesh/presentation" "dismantle_target" "이관 후 삭제")
        "model_tree.h" = (New-OwnershipEntry "mesh/presentation" "dismantle_target" "이관 후 삭제")
        "mouse_interactor_style.cpp" = (New-OwnershipEntry "render/infrastructure" "dismantle_target" "이관 후 삭제")
        "mouse_interactor_style.h" = (New-OwnershipEntry "render/infrastructure" "dismantle_target" "이관 후 삭제")
        "test_window.cpp" = (New-OwnershipEntry "shell/presentation" "dismantle_target" "이관 후 삭제")
        "test_window.h" = (New-OwnershipEntry "shell/presentation" "dismantle_target" "이관 후 삭제")
        "texture.cpp" = (New-OwnershipEntry "render/infrastructure" "dismantle_target" "이관 후 삭제")
        "texture.h" = (New-OwnershipEntry "render/infrastructure" "dismantle_target" "이관 후 삭제")
        "toolbar.cpp" = (New-OwnershipEntry "shell/presentation" "dismantle_target" "이관 후 삭제")
        "toolbar.h" = (New-OwnershipEntry "shell/presentation" "dismantle_target" "이관 후 삭제")
        "unv_reader.cpp" = (New-OwnershipEntry "io/infrastructure" "dismantle_target" "이관 후 삭제")
        "unv_reader.h" = (New-OwnershipEntry "io/infrastructure" "dismantle_target" "이관 후 삭제")
        "vtk_viewer.cpp" = (New-OwnershipEntry "render/application,render/infrastructure" "dismantle_target" "분해 후 삭제")
        "vtk_viewer.h" = (New-OwnershipEntry "render/application,render/infrastructure" "dismantle_target" "분해 후 삭제")

        "atoms_template_bravais_lattice.cpp" = (New-OwnershipEntry "plan_excluded" "documentation_material_excluded" "설명 자료 유지")
        "atoms_template_periodic_table.cpp" = (New-OwnershipEntry "plan_excluded" "documentation_material_excluded" "설명 자료 유지")

        "atoms/atoms_template.cpp" = (New-OwnershipEntry "structure,measurement,density,shell/presentation,render/application" "dismantle_target" "분해 후 삭제")
        "atoms/atoms_template.h" = (New-OwnershipEntry "structure,measurement,density,shell/presentation,render/application" "dismantle_target" "분해 후 삭제")
    }

    $currentSrcRootFiles = Get-RootRelativeCodeFilePaths -Path "webassembly/src" -Prefix ""
    $currentAtomsRootFiles = Get-RootRelativeCodeFilePaths -Path "webassembly/src/atoms" -Prefix "atoms"
    $currentRootFiles = @($currentSrcRootFiles + $currentAtomsRootFiles)

    $unknownCurrentFiles = @(
        $currentRootFiles | Where-Object { -not $ownershipMap.Contains($_) }
    )

    $unclassifiedFiles = @()
    foreach ($path in $currentRootFiles) {
        if (-not $ownershipMap.Contains($path)) {
            continue
        }

        $entry = $ownershipMap[$path]
        if ([string]::IsNullOrWhiteSpace($entry.Owner) -or
            [string]::IsNullOrWhiteSpace($entry.Category) -or
            [string]::IsNullOrWhiteSpace($entry.Action)) {
            $unclassifiedFiles += $path
        }
    }

    $requiredFacadePaths = @(
        "main.cpp",
        "app.cpp",
        "app.h"
    )
    $missingFacadeInMap = @(
        $requiredFacadePaths |
            Where-Object {
                -not $ownershipMap.Contains($_) -or $ownershipMap[$_].Category -ne "facade_keep"
            }
    )
    $missingFacadeInCurrent = @(
        $requiredFacadePaths | Where-Object { $_ -notin $currentRootFiles }
    )

    $docExcludedPaths = @(
        "atoms_template_bravais_lattice.cpp",
        "atoms_template_periodic_table.cpp"
    )
    $docExcludedMisclassified = @(
        $docExcludedPaths |
            Where-Object {
                $_ -in $currentRootFiles -and (
                    -not $ownershipMap.Contains($_) -or
                    $ownershipMap[$_].Category -ne "documentation_material_excluded"
                )
            }
    )

    $currentByCategory = @{}
    foreach ($path in $currentRootFiles) {
        if (-not $ownershipMap.Contains($path)) {
            continue
        }

        $category = $ownershipMap[$path].Category
        if (-not $currentByCategory.ContainsKey($category)) {
            $currentByCategory[$category] = 0
        }
        $currentByCategory[$category] += 1
    }

    $results = @(
        (New-Result "P17R0.ownership_map_covers_current_root_files" ($unknownCurrentFiles.Count -eq 0) $unknownCurrentFiles.Count 0),
        (New-Result "P17R0.ownership_map_entries_classified" ($unclassifiedFiles.Count -eq 0) $unclassifiedFiles.Count 0),
        (New-Result "P17R0.facade_entries_defined_in_map" ($missingFacadeInMap.Count -eq 0) $missingFacadeInMap.Count 0),
        (New-Result "P17R0.required_facade_files_present" ($missingFacadeInCurrent.Count -eq 0) $missingFacadeInCurrent.Count 0),
        (New-Result "P17R0.doc_material_exclusion_classified" ($docExcludedMisclassified.Count -eq 0) $docExcludedMisclassified.Count 0)
    )

    Write-Host ("Phase 17-R0 Root Ownership Map Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host ("Phase 17-R0 ownership snapshot: current root files={0}" -f $currentRootFiles.Count)
    foreach ($category in ($currentByCategory.Keys | Sort-Object)) {
        Write-Host (" - {0}: {1}" -f $category, $currentByCategory[$category])
    }

    if ($unknownCurrentFiles.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R0.ownership_map_covers_current_root_files] Unknown current files:"
        foreach ($entry in $unknownCurrentFiles) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($unclassifiedFiles.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R0.ownership_map_entries_classified] Unclassified entries:"
        foreach ($entry in $unclassifiedFiles) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($missingFacadeInMap.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R0.facade_entries_defined_in_map] Missing/invalid facade map entries:"
        foreach ($entry in $missingFacadeInMap) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($missingFacadeInCurrent.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R0.required_facade_files_present] Missing facade files in current tree:"
        foreach ($entry in $missingFacadeInCurrent) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($docExcludedMisclassified.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R0.doc_material_exclusion_classified] Misclassified documentation material files:"
        foreach ($entry in $docExcludedMisclassified) {
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
