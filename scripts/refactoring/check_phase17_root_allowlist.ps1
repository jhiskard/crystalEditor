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

function Get-CodeFileNames {
    param(
        [string]$Path
    )

    if (-not (Test-Path $Path)) {
        return @()
    }

    return @(
        Get-ChildItem -Path $Path -File |
            Where-Object { $_.Extension -in ".cpp", ".h" } |
            Sort-Object Name |
            Select-Object -ExpandProperty Name
    )
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    # Phase 17-R0 snapshot (2026-04-08)
    $baselineSrcRootFiles = @(
        "app.cpp",
        "app.h",
        "atoms_template_bravais_lattice.cpp",
        "atoms_template_periodic_table.cpp",
        "bind_function.cpp",
        "custom_ui.cpp",
        "custom_ui.h",
        "file_loader.cpp",
        "file_loader.h",
        "font_manager.cpp",
        "font_manager.h",
        "image.cpp",
        "image.h",
        "lcrs_tree.cpp",
        "lcrs_tree.h",
        "main.cpp",
        "mesh.cpp",
        "mesh.h",
        "mesh_detail.cpp",
        "mesh_detail.h",
        "mesh_group.cpp",
        "mesh_group.h",
        "mesh_group_detail.cpp",
        "mesh_group_detail.h",
        "mesh_manager.cpp",
        "mesh_manager.h",
        "model_tree.cpp",
        "model_tree.h",
        "mouse_interactor_style.cpp",
        "mouse_interactor_style.h",
        "test_window.cpp",
        "test_window.h",
        "texture.cpp",
        "texture.h",
        "toolbar.cpp",
        "toolbar.h",
        "unv_reader.cpp",
        "unv_reader.h",
        "vtk_viewer.cpp",
        "vtk_viewer.h"
    )
    $baselineAtomsRootFiles = @(
        "atoms_template.cpp",
        "atoms_template.h"
    )

    $requiredFacadeFiles = @(
        "main.cpp",
        "app.cpp",
        "app.h"
    )

    $documentationOnlyFiles = @(
        "atoms_template_bravais_lattice.cpp",
        "atoms_template_periodic_table.cpp"
    )

    $currentSrcRootFiles = Get-CodeFileNames -Path "webassembly/src"
    $currentAtomsRootFiles = Get-CodeFileNames -Path "webassembly/src/atoms"

    $unknownSrcRootFiles = @(
        $currentSrcRootFiles | Where-Object { $_ -notin $baselineSrcRootFiles }
    )
    $unknownAtomsRootFiles = @(
        $currentAtomsRootFiles | Where-Object { $_ -notin $baselineAtomsRootFiles }
    )

    $missingFacadeFiles = @(
        $requiredFacadeFiles | Where-Object { $_ -notin $currentSrcRootFiles }
    )

    $unexpectedAtomsTemplateRootFiles = @(
        $currentSrcRootFiles |
            Where-Object { $_ -like "atoms_template_*" -and $_ -notin $documentationOnlyFiles }
    )

    $results = @(
        (New-Result "P17R0.src_root_set_within_baseline_snapshot" ($unknownSrcRootFiles.Count -eq 0) $unknownSrcRootFiles.Count 0),
        (New-Result "P17R0.atoms_root_set_within_baseline_snapshot" ($unknownAtomsRootFiles.Count -eq 0) $unknownAtomsRootFiles.Count 0),
        (New-Result "P17R0.src_root_file_count_not_increased" ($currentSrcRootFiles.Count -le $baselineSrcRootFiles.Count) $currentSrcRootFiles.Count ("<= {0}" -f $baselineSrcRootFiles.Count)),
        (New-Result "P17R0.atoms_root_file_count_not_increased" ($currentAtomsRootFiles.Count -le $baselineAtomsRootFiles.Count) $currentAtomsRootFiles.Count ("<= {0}" -f $baselineAtomsRootFiles.Count)),
        (New-Result "P17R0.facade_files_present" ($missingFacadeFiles.Count -eq 0) $missingFacadeFiles.Count 0),
        (New-Result "P17R0.documentation_only_files_classified" ($unexpectedAtomsTemplateRootFiles.Count -eq 0) $unexpectedAtomsTemplateRootFiles.Count 0)
    )

    Write-Host ("Phase 17-R0 Root Allowlist Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host "Phase 17-R0 snapshot:"
    Write-Host (" - src root code files: {0}" -f $currentSrcRootFiles.Count)
    Write-Host (" - atoms root code files: {0}" -f $currentAtomsRootFiles.Count)

    if ($unknownSrcRootFiles.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R0.src_root_set_within_baseline_snapshot] Unknown src root files:"
        foreach ($entry in $unknownSrcRootFiles) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($unknownAtomsRootFiles.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R0.atoms_root_set_within_baseline_snapshot] Unknown atoms root files:"
        foreach ($entry in $unknownAtomsRootFiles) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($missingFacadeFiles.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R0.facade_files_present] Missing facade files:"
        foreach ($entry in $missingFacadeFiles) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($unexpectedAtomsTemplateRootFiles.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R0.documentation_only_files_classified] Unexpected atoms_template_* files:"
        foreach ($entry in $unexpectedAtomsTemplateRootFiles) {
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
