param(
    [switch]$FailOnViolation = $true
)

$ErrorActionPreference = "Stop"

function Get-RgLines {
    param(
        [string[]]$Arguments
    )

    $lines = & rg @Arguments 2>$null
    if ($LASTEXITCODE -eq 1) {
        return @()
    }
    if ($LASTEXITCODE -ne 0) {
        throw "rg failed (exit=$LASTEXITCODE): $($Arguments -join ' ')"
    }
    return @($lines | Where-Object { $_ -and $_.Trim() -ne "" })
}

function Get-LineCount {
    param(
        [string]$Path
    )

    $raw = Get-Content -Raw -Path $Path
    if ([string]::IsNullOrEmpty($raw)) {
        return 0
    }

    $count = ([regex]::Matches($raw, "`n")).Count
    if (-not $raw.EndsWith("`n")) {
        $count += 1
    }
    return $count
}

function New-Result {
    param(
        [string]$Name,
        [bool]$Pass,
        [object]$Current,
        [object]$Target,
        [bool]$Advisory = $false
    )

    return @{
        Name = $Name
        Pass = $Pass
        Current = $Current
        Target = $Target
        Advisory = $Advisory
    }
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    $baseline = @{
        "webassembly/src/atoms/atoms_template.cpp" = 6130
        "webassembly/src/model_tree.cpp" = 1782
        "webassembly/src/atoms/ui/charge_density_ui.cpp" = 2260
        "webassembly/src/vtk_viewer.cpp" = 2449
    }

    $lineTargets = @{
        "webassembly/src/atoms/atoms_template.cpp" = [int][Math]::Floor($baseline["webassembly/src/atoms/atoms_template.cpp"] * 0.7)
        "webassembly/src/model_tree.cpp" = [int][Math]::Floor($baseline["webassembly/src/model_tree.cpp"] * 0.75)
        "webassembly/src/atoms/ui/charge_density_ui.cpp" = [int][Math]::Floor($baseline["webassembly/src/atoms/ui/charge_density_ui.cpp"] * 0.75)
        "webassembly/src/vtk_viewer.cpp" = [int][Math]::Floor($baseline["webassembly/src/vtk_viewer.cpp"] * 0.8)
    }

    $currentLines = @{
        "webassembly/src/atoms/atoms_template.cpp" = Get-LineCount "webassembly/src/atoms/atoms_template.cpp"
        "webassembly/src/model_tree.cpp" = Get-LineCount "webassembly/src/model_tree.cpp"
        "webassembly/src/atoms/ui/charge_density_ui.cpp" = Get-LineCount "webassembly/src/atoms/ui/charge_density_ui.cpp"
        "webassembly/src/vtk_viewer.cpp" = Get-LineCount "webassembly/src/vtk_viewer.cpp"
    }

    $requiredFiles = @(
        "webassembly/src/atoms/presentation/builder_windows_controller.cpp",
        "webassembly/src/atoms/presentation/editor_windows_controller.cpp",
        "webassembly/src/atoms/presentation/slice_view_panel.cpp",
        "webassembly/src/atoms/presentation/isosurface_panel.cpp",
        "webassembly/src/atoms/application/measurement_controller.cpp",
        "webassembly/src/atoms/application/structure_lifecycle_service.cpp",
        "webassembly/src/atoms/application/visibility_service.cpp",
        "webassembly/src/mesh/presentation/model_tree_dialogs.cpp",
        "webassembly/src/mesh/presentation/model_tree_structure_section.cpp",
        "webassembly/src/mesh/presentation/model_tree_mesh_section.cpp",
        "webassembly/src/render/application/viewer_interaction_controller.cpp",
        "webassembly/src/render/application/camera_align_service.cpp"
    )
    $missingFiles = @($requiredFiles | Where-Object { -not (Test-Path $_) })

    $atomsMovedDefsPattern =
        "void AtomsTemplate::(RenderPeriodicTableWindow|RenderCrystalTemplatesWindow|RenderBrillouinZonePlotWindow|RenderCreatedAtomsWindow|RenderBondsManagementWindow|RenderCellInformationWindow|EnterMeasurementMode|ExitMeasurementMode|HandleMeasurementClickByPicker|RenderMeasurementModeOverlay|SetStructureVisible|SetUnitCellVisible|SetAtomVisibleById|SetBondVisibleById|SetAtomLabelVisibleById|SetBondLabelVisibleById|RegisterStructure|RemoveStructure|RemoveUnassignedData)\("
    $atomsOriginMovedDefLines = Get-RgLines @(
        "-n",
        $atomsMovedDefsPattern,
        "webassembly/src/atoms/atoms_template.cpp"
    )
    $atomsNewMovedDefLines = Get-RgLines @(
        "-n",
        $atomsMovedDefsPattern,
        "webassembly/src/atoms/presentation/builder_windows_controller.cpp",
        "webassembly/src/atoms/presentation/editor_windows_controller.cpp",
        "webassembly/src/atoms/application/measurement_controller.cpp",
        "webassembly/src/atoms/application/structure_lifecycle_service.cpp",
        "webassembly/src/atoms/application/visibility_service.cpp"
    )

    $chargeMovedDefsPattern =
        "void ChargeDensityUI::(renderSliceControls|renderSlicePreview|updateSlicePreviewForTarget|updateSlicePreview|renderIsosurfaceSection|rebuildMultipleIsosurfaces)\("
    $chargeOriginMovedDefLines = Get-RgLines @(
        "-n",
        $chargeMovedDefsPattern,
        "webassembly/src/atoms/ui/charge_density_ui.cpp"
    )
    $chargeNewMovedDefLines = Get-RgLines @(
        "-n",
        $chargeMovedDefsPattern,
        "webassembly/src/atoms/presentation/slice_view_panel.cpp",
        "webassembly/src/atoms/presentation/isosurface_panel.cpp"
    )

    $modelTreeMovedDefsPattern =
        "void ModelTree::(renderDeleteConfirmPopup|renderClearMeasurementsConfirmPopup|renderXsfStructureTable|renderMeshTable|renderMeshTree)\("
    $modelTreeOriginMovedDefLines = Get-RgLines @(
        "-n",
        $modelTreeMovedDefsPattern,
        "webassembly/src/model_tree.cpp"
    )
    $modelTreeNewMovedDefLines = Get-RgLines @(
        "-n",
        $modelTreeMovedDefsPattern,
        "webassembly/src/mesh/presentation/model_tree_dialogs.cpp",
        "webassembly/src/mesh/presentation/model_tree_structure_section.cpp",
        "webassembly/src/mesh/presentation/model_tree_mesh_section.cpp"
    )

    $viewerMovedDefsPattern =
        "void VtkViewer::(processEvents|AlignCameraToCellAxis|AlignCameraToIcellAxis)\("
    $viewerOriginMovedDefLines = Get-RgLines @(
        "-n",
        $viewerMovedDefsPattern,
        "webassembly/src/vtk_viewer.cpp"
    )
    $viewerNewMovedDefLines = Get-RgLines @(
        "-n",
        $viewerMovedDefsPattern,
        "webassembly/src/render/application/viewer_interaction_controller.cpp",
        "webassembly/src/render/application/camera_align_service.cpp"
    )

    $cmakeWiringLines = Get-RgLines @(
        "-n",
        "webassembly/src/(mesh/presentation/model_tree_dialogs\.cpp|mesh/presentation/model_tree_structure_section\.cpp|mesh/presentation/model_tree_mesh_section\.cpp|render/application/viewer_interaction_controller\.cpp|render/application/camera_align_service\.cpp|atoms/application/measurement_controller\.cpp|atoms/application/structure_lifecycle_service\.cpp|atoms/application/visibility_service\.cpp|atoms/presentation/builder_windows_controller\.cpp|atoms/presentation/editor_windows_controller\.cpp|atoms/presentation/isosurface_panel\.cpp|atoms/presentation/slice_view_panel\.cpp)",
        "CMakeLists.txt"
    )

    $missingFileCount = if ($missingFiles.Count -eq 0) { 0 } else { $missingFiles.Count }

    $results = @(
        (New-Result "P4.required_files_exist" ($missingFiles.Count -eq 0) $missingFileCount 0),
        (New-Result "P4.atoms_moved_defs_removed_from_origin" ($atomsOriginMovedDefLines.Count -eq 0) $atomsOriginMovedDefLines.Count 0),
        (New-Result "P4.atoms_moved_defs_present_in_new_files" ($atomsNewMovedDefLines.Count -eq 20) $atomsNewMovedDefLines.Count 20),
        (New-Result "P4.charge_moved_defs_removed_from_origin" ($chargeOriginMovedDefLines.Count -eq 0) $chargeOriginMovedDefLines.Count 0),
        (New-Result "P4.charge_moved_defs_present_in_new_files" ($chargeNewMovedDefLines.Count -eq 6) $chargeNewMovedDefLines.Count 6),
        (New-Result "P4.model_tree_moved_defs_removed_from_origin" ($modelTreeOriginMovedDefLines.Count -eq 0) $modelTreeOriginMovedDefLines.Count 0),
        (New-Result "P4.model_tree_moved_defs_present_in_new_files" ($modelTreeNewMovedDefLines.Count -eq 5) $modelTreeNewMovedDefLines.Count 5),
        (New-Result "P4.viewer_moved_defs_removed_from_origin" ($viewerOriginMovedDefLines.Count -eq 0) $viewerOriginMovedDefLines.Count 0),
        (New-Result "P4.viewer_moved_defs_present_in_new_files" ($viewerNewMovedDefLines.Count -eq 3) $viewerNewMovedDefLines.Count 3),
        (New-Result "P4.cmake_wiring_entries" ($cmakeWiringLines.Count -eq 12) $cmakeWiringLines.Count 12),
        (New-Result "P4.atoms_template_lines_reduction_recommended" ($currentLines["webassembly/src/atoms/atoms_template.cpp"] -le $lineTargets["webassembly/src/atoms/atoms_template.cpp"]) $currentLines["webassembly/src/atoms/atoms_template.cpp"] ("<= {0}" -f $lineTargets["webassembly/src/atoms/atoms_template.cpp"]) $true),
        (New-Result "P4.model_tree_lines_reduction_recommended" ($currentLines["webassembly/src/model_tree.cpp"] -le $lineTargets["webassembly/src/model_tree.cpp"]) $currentLines["webassembly/src/model_tree.cpp"] ("<= {0}" -f $lineTargets["webassembly/src/model_tree.cpp"]) $true),
        (New-Result "P4.charge_density_ui_lines_reduction_recommended" ($currentLines["webassembly/src/atoms/ui/charge_density_ui.cpp"] -le $lineTargets["webassembly/src/atoms/ui/charge_density_ui.cpp"]) $currentLines["webassembly/src/atoms/ui/charge_density_ui.cpp"] ("<= {0}" -f $lineTargets["webassembly/src/atoms/ui/charge_density_ui.cpp"]) $true),
        (New-Result "P4.vtk_viewer_lines_reduction_recommended" ($currentLines["webassembly/src/vtk_viewer.cpp"] -le $lineTargets["webassembly/src/vtk_viewer.cpp"]) $currentLines["webassembly/src/vtk_viewer.cpp"] ("<= {0}" -f $lineTargets["webassembly/src/vtk_viewer.cpp"]) $true)
    )

    Write-Host ("Phase 4 UI/Renderer Split Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($r in $results) {
        $status = if ($r.Pass) { "PASS" } else { "FAIL" }
        $advisory = if ($r.Advisory) { " [ADVISORY]" } else { "" }
        Write-Host (" - {0}: {1}{2} (current={3}, target={4})" -f $r.Name, $status, $advisory, $r.Current, $r.Target)
    }

    Write-Host ""
    Write-Host "Line count snapshot:"
    foreach ($path in @(
        "webassembly/src/atoms/atoms_template.cpp",
        "webassembly/src/model_tree.cpp",
        "webassembly/src/atoms/ui/charge_density_ui.cpp",
        "webassembly/src/vtk_viewer.cpp"
    )) {
        $base = $baseline[$path]
        $cur = $currentLines[$path]
        $ratio = if ($base -gt 0) { [Math]::Round((100.0 * ($base - $cur) / $base), 2) } else { 0.0 }
        Write-Host (" - {0}: baseline={1}, current={2}, reduction={3}%" -f $path, $base, $cur, $ratio)
    }

    if ($missingFiles.Count -gt 0) {
        Write-Host ""
        Write-Host "[P4.required_files_exist] Missing files:"
        foreach ($file in $missingFiles) {
            Write-Host (" - {0}" -f $file)
        }
    }

    $advisoryMisses = @($results | Where-Object { $_.Advisory -and -not $_.Pass })
    if ($advisoryMisses.Count -gt 0) {
        Write-Host ""
        Write-Host "Advisory misses:"
        foreach ($r in $advisoryMisses) {
            Write-Host (" - {0}: current={1}, target={2}" -f $r.Name, $r.Current, $r.Target)
        }
    }

    $violations = @($results | Where-Object { -not $_.Pass -and -not $_.Advisory })
    if ($violations.Count -gt 0 -and $FailOnViolation) {
        exit 1
    }

    exit 0
}
finally {
    Pop-Location
}
