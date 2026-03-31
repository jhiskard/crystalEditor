param(
    [switch]$FailOnViolation = $true
)

$ErrorActionPreference = "Stop"

function Get-LineCount {
    param(
        [string]$Path
    )

    return (Get-Content -Encoding utf8 -Path $Path | Measure-Object -Line).Lines
}

function Get-SourceEntries {
    param(
        [string]$Path
    )

    $entries = @()
    foreach ($line in Get-Content -Encoding utf8 -Path $Path) {
        $trimmed = $line.Trim()
        if (-not $trimmed -or $trimmed.StartsWith("#")) {
            continue
        }
        if ($trimmed -match '^webassembly/src/[A-Za-z0-9_./-]+\.cpp$') {
            $entries += $trimmed
        }
    }
    return $entries
}

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

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    $expectedRootSources = @(
        "webassembly/src/main.cpp",
        "webassembly/src/bind_function.cpp"
    )

    $expectedModuleSources = [ordered]@{
        "wb_core" = @(
            "webassembly/src/common/colormap.cpp",
            "webassembly/src/common/string_utils.cpp",
            "webassembly/src/lcrs_tree.cpp"
        )
        "wb_render" = @(
            "webassembly/src/vtk_viewer.cpp",
            "webassembly/src/mouse_interactor_style.cpp",
            "webassembly/src/render/application/camera_align_service.cpp",
            "webassembly/src/render/application/viewer_interaction_controller.cpp",
            "webassembly/src/render/infrastructure/vtk_render_gateway.cpp"
        )
        "wb_mesh" = @(
            "webassembly/src/mesh.cpp",
            "webassembly/src/mesh_detail.cpp",
            "webassembly/src/mesh_group.cpp",
            "webassembly/src/mesh_group_detail.cpp",
            "webassembly/src/mesh_manager.cpp",
            "webassembly/src/model_tree.cpp",
            "webassembly/src/mesh/presentation/model_tree_dialogs.cpp",
            "webassembly/src/mesh/presentation/model_tree_mesh_section.cpp",
            "webassembly/src/mesh/presentation/model_tree_structure_section.cpp"
        )
        "wb_atoms" = @(
            "webassembly/src/atoms/atoms_template.cpp",
            "webassembly/src/atoms/application/measurement_controller.cpp",
            "webassembly/src/atoms/application/structure_lifecycle_service.cpp",
            "webassembly/src/atoms/application/structure_read_model.cpp",
            "webassembly/src/atoms/application/visibility_service.cpp",
            "webassembly/src/atoms/domain/atom_manager.cpp",
            "webassembly/src/atoms/domain/bond_manager.cpp",
            "webassembly/src/atoms/domain/bz_plot.cpp",
            "webassembly/src/atoms/domain/cell_manager.cpp",
            "webassembly/src/atoms/domain/charge_density.cpp",
            "webassembly/src/atoms/domain/crystal_structure.cpp",
            "webassembly/src/atoms/domain/crystal_system.cpp",
            "webassembly/src/atoms/domain/element_database.cpp",
            "webassembly/src/atoms/domain/structure_state_store.cpp",
            "webassembly/src/atoms/domain/surrounding_atom_manager.cpp",
            "webassembly/src/atoms/infrastructure/batch_update_system.cpp",
            "webassembly/src/atoms/infrastructure/bond_renderer.cpp",
            "webassembly/src/atoms/infrastructure/bz_plot_layer.cpp",
            "webassembly/src/atoms/infrastructure/charge_density_renderer.cpp",
            "webassembly/src/atoms/infrastructure/vtk_renderer.cpp",
            "webassembly/src/atoms/presentation/builder_windows_controller.cpp",
            "webassembly/src/atoms/presentation/editor_windows_controller.cpp",
            "webassembly/src/atoms/presentation/isosurface_panel.cpp",
            "webassembly/src/atoms/presentation/slice_view_panel.cpp",
            "webassembly/src/atoms/ui/atom_editor_ui.cpp",
            "webassembly/src/atoms/ui/bond_ui.cpp",
            "webassembly/src/atoms/ui/bravais_lattice_ui.cpp",
            "webassembly/src/atoms/ui/bz_plot_ui.cpp",
            "webassembly/src/atoms/ui/cell_info_ui.cpp",
            "webassembly/src/atoms/ui/charge_density_ui.cpp",
            "webassembly/src/atoms/ui/periodic_table_ui.cpp"
        )
        "wb_io" = @(
            "webassembly/src/file_loader.cpp",
            "webassembly/src/unv_reader.cpp",
            "webassembly/src/io/application/import_apply_service.cpp",
            "webassembly/src/io/application/import_orchestrator.cpp",
            "webassembly/src/io/application/parser_worker_service.cpp",
            "webassembly/src/io/platform/browser_file_picker.cpp",
            "webassembly/src/atoms/infrastructure/chgcar_parser.cpp",
            "webassembly/src/atoms/infrastructure/file_io_manager.cpp"
        )
        "wb_ui" = @(
            "webassembly/src/app.cpp",
            "webassembly/src/custom_ui.cpp",
            "webassembly/src/font_manager.cpp",
            "webassembly/src/image.cpp",
            "webassembly/src/test_window.cpp",
            "webassembly/src/texture.cpp",
            "webassembly/src/toolbar.cpp"
        )
    }

    $expectedModuleFiles = @(
        "webassembly/cmake/modules/wb_core.cmake",
        "webassembly/cmake/modules/wb_render.cmake",
        "webassembly/cmake/modules/wb_mesh.cmake",
        "webassembly/cmake/modules/wb_atoms.cmake",
        "webassembly/cmake/modules/wb_io.cmake",
        "webassembly/cmake/modules/wb_ui.cmake"
    )

    $excludedLegacySources = @(
        "webassembly/src/atoms_template_periodic_table.cpp",
        "webassembly/src/atoms_template_bravais_lattice.cpp",
        "webassembly/src/atoms/ui/atoms_template_main_window_ui.cpp"
    )

    $missingModuleFiles = @($expectedModuleFiles | Where-Object { -not (Test-Path $_) })
    $rootLineCount = Get-LineCount "CMakeLists.txt"
    $rootSourceEntries = Get-SourceEntries "CMakeLists.txt"
    $rootSourceCompare = Compare-Object -ReferenceObject ($expectedRootSources | Sort-Object) -DifferenceObject ($rootSourceEntries | Sort-Object)

    $moduleEntryMismatches = @()
    $allActualEntries = @($rootSourceEntries)
    foreach ($moduleName in $expectedModuleSources.Keys) {
        $modulePath = "webassembly/cmake/modules/{0}.cmake" -f $moduleName
        $actualEntries = Get-SourceEntries $modulePath
        $allActualEntries += $actualEntries

        $diff = Compare-Object -ReferenceObject ($expectedModuleSources[$moduleName] | Sort-Object) -DifferenceObject ($actualEntries | Sort-Object)
        if ($diff) {
            $moduleEntryMismatches += @{
                Module = $moduleName
                Diff = $diff
            }
        }
    }

    $expectedAllEntries = @($expectedRootSources)
    foreach ($moduleName in $expectedModuleSources.Keys) {
        $expectedAllEntries += $expectedModuleSources[$moduleName]
    }

    $duplicateEntries = @(
        $allActualEntries |
            Group-Object |
            Where-Object { $_.Count -gt 1 } |
            Select-Object -ExpandProperty Name
    )
    $unexpectedEntries = @(
        Compare-Object -ReferenceObject ($expectedAllEntries | Sort-Object) -DifferenceObject ($allActualEntries | Sort-Object) |
            Where-Object { $_.SideIndicator -eq "=>" } |
            Select-Object -ExpandProperty InputObject
    )
    $missingEntries = @(
        Compare-Object -ReferenceObject ($expectedAllEntries | Sort-Object) -DifferenceObject ($allActualEntries | Sort-Object) |
            Where-Object { $_.SideIndicator -eq "<=" } |
            Select-Object -ExpandProperty InputObject
    )
    $excludedEntriesPresent = @($excludedLegacySources | Where-Object { $_ -in $allActualEntries })

    $rootRaw = Get-Content -Raw -Encoding utf8 "CMakeLists.txt"
    $rootModuleLinkAnchor = [regex]::IsMatch(
        $rootRaw,
        'target_link_libraries\(\$\{PROJECT_NAME\}\s+PRIVATE\s+wb_ui\s+wb_io\s+wb_atoms\s+wb_mesh\s+wb_render\s+wb_core\s*\)',
        [System.Text.RegularExpressions.RegexOptions]::Singleline
    )

    $dependencyAnchors = @(
        'target_link_libraries\(wb_render PUBLIC wb_core wb_ui wb_mesh wb_atoms\)',
        'target_link_libraries\(wb_mesh PUBLIC wb_core wb_ui wb_render wb_atoms\)',
        'target_link_libraries\(wb_atoms PUBLIC wb_core wb_ui wb_render wb_mesh\)',
        'target_link_libraries\(wb_io PUBLIC wb_core wb_ui wb_render wb_mesh wb_atoms\)',
        'target_link_libraries\(wb_ui PUBLIC wb_core wb_render wb_mesh wb_atoms wb_io\)'
    )
    $dependencyAnchorMatches = 0
    foreach ($anchor in $dependencyAnchors) {
        if ([regex]::IsMatch($rootRaw, $anchor)) {
            $dependencyAnchorMatches += 1
        }
    }

    $rootModuleLinkAnchorCurrent = if ($rootModuleLinkAnchor) { 1 } else { 0 }

    $results = @(
        (New-Result "P5.root_cmake_lines" ($rootLineCount -le 177) $rootLineCount "<= 177"),
        (New-Result "P5.module_cmake_files_exist" ($missingModuleFiles.Count -eq 0) (6 - $missingModuleFiles.Count) 6),
        (New-Result "P5.root_entrypoints_registered" ($rootSourceCompare.Count -eq 0) $rootSourceEntries.Count 2),
        (New-Result "P5.module_source_assignment_exact" ($moduleEntryMismatches.Count -eq 0) $moduleEntryMismatches.Count 0),
        (New-Result "P5.total_registered_cpp_units" ($allActualEntries.Count -eq $expectedAllEntries.Count) $allActualEntries.Count $expectedAllEntries.Count),
        (New-Result "P5.unowned_cpp_units" ($missingEntries.Count -eq 0) $missingEntries.Count 0),
        (New-Result "P5.duplicate_cpp_units" ($duplicateEntries.Count -eq 0) $duplicateEntries.Count 0),
        (New-Result "P5.unexpected_cpp_units" ($unexpectedEntries.Count -eq 0) $unexpectedEntries.Count 0),
        (New-Result "P5.excluded_legacy_units_registered" ($excludedEntriesPresent.Count -eq 0) $excludedEntriesPresent.Count 0),
        (New-Result "P5.root_module_link_anchor" $rootModuleLinkAnchor $rootModuleLinkAnchorCurrent 1),
        (New-Result "P5.module_dependency_anchor_count" ($dependencyAnchorMatches -eq $dependencyAnchors.Count) $dependencyAnchorMatches $dependencyAnchors.Count)
    )

    Write-Host ("Phase 5 Build Modules Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host "Module ownership snapshot:"
    Write-Host (" - root: {0} entrypoints" -f $expectedRootSources.Count)
    foreach ($moduleName in $expectedModuleSources.Keys) {
        Write-Host (" - {0}: {1} cpp units" -f $moduleName, $expectedModuleSources[$moduleName].Count)
    }

    if ($missingModuleFiles.Count -gt 0) {
        Write-Host ""
        Write-Host "[P5.module_cmake_files_exist] Missing files:"
        foreach ($path in $missingModuleFiles) {
            Write-Host (" - {0}" -f $path)
        }
    }

    if ($rootSourceCompare.Count -gt 0) {
        Write-Host ""
        Write-Host "[P5.root_entrypoints_registered] Root entrypoint mismatch:"
        foreach ($entry in $rootSourceCompare) {
            Write-Host (" - {0} ({1})" -f $entry.InputObject, $entry.SideIndicator)
        }
    }

    if ($moduleEntryMismatches.Count -gt 0) {
        Write-Host ""
        Write-Host "[P5.module_source_assignment_exact] Module mismatches:"
        foreach ($mismatch in $moduleEntryMismatches) {
            Write-Host (" - {0}" -f $mismatch.Module)
            foreach ($entry in $mismatch.Diff) {
                Write-Host ("   - {0} ({1})" -f $entry.InputObject, $entry.SideIndicator)
            }
        }
    }

    if ($missingEntries.Count -gt 0) {
        Write-Host ""
        Write-Host "[P5.unowned_cpp_units] Missing assignments:"
        foreach ($entry in $missingEntries) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($duplicateEntries.Count -gt 0) {
        Write-Host ""
        Write-Host "[P5.duplicate_cpp_units] Duplicated assignments:"
        foreach ($entry in $duplicateEntries) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($unexpectedEntries.Count -gt 0) {
        Write-Host ""
        Write-Host "[P5.unexpected_cpp_units] Unexpected entries:"
        foreach ($entry in $unexpectedEntries) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($excludedEntriesPresent.Count -gt 0) {
        Write-Host ""
        Write-Host "[P5.excluded_legacy_units_registered] Unexpected legacy registrations:"
        foreach ($entry in $excludedEntriesPresent) {
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
