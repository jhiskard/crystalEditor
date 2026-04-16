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
        "webassembly/src/bind_function.cpp",
        "webassembly/src/file_loader.cpp",
        "webassembly/src/file_loader.h",
        "webassembly/src/unv_reader.cpp",
        "webassembly/src/unv_reader.h",
        "webassembly/src/vtk_viewer.cpp",
        "webassembly/src/vtk_viewer.h",
        "webassembly/src/mouse_interactor_style.cpp",
        "webassembly/src/mouse_interactor_style.h"
    )

    $requiredMigratedFiles = @(
        "webassembly/src/platform/wasm/workbench_bindings.cpp",
        "webassembly/src/io/application/import_entry_service.cpp",
        "webassembly/src/io/application/import_entry_service.h",
        "webassembly/src/io/infrastructure/unv/unv_mesh_reader.cpp",
        "webassembly/src/io/infrastructure/unv/unv_mesh_reader.h",
        "webassembly/src/render/presentation/viewer_window.cpp",
        "webassembly/src/render/presentation/viewer_window.h",
        "webassembly/src/render/infrastructure/interaction/mouse_interactor_style.cpp",
        "webassembly/src/render/infrastructure/interaction/mouse_interactor_style.h",
        "webassembly/src/io/application/file_dialog_port.h",
        "webassembly/src/io/application/worker_port.h",
        "webassembly/src/io/application/progress_port.h",
        "webassembly/src/io/platform/memfs_chunk_stream.cpp",
        "webassembly/src/io/platform/memfs_chunk_stream.h",
        "webassembly/src/platform/browser/browser_file_dialog_adapter.cpp",
        "webassembly/src/platform/browser/browser_file_dialog_adapter.h",
        "webassembly/src/platform/worker/emscripten_worker_port.cpp",
        "webassembly/src/platform/worker/emscripten_worker_port.h",
        "webassembly/src/platform/worker/runtime_progress_port.cpp",
        "webassembly/src/platform/worker/runtime_progress_port.h",
        "webassembly/src/platform/persistence/viewer_preferences_store.cpp",
        "webassembly/src/platform/persistence/viewer_preferences_store.h",
        "webassembly/src/shell/presentation/import/import_popup_presenter.cpp",
        "webassembly/src/shell/presentation/import/import_popup_presenter.h"
    )

    $legacyStillPresent = Get-StillPresentPaths -Paths $legacyRootFiles
    $migratedMissing = Get-MissingPaths -Paths $requiredMigratedFiles

    $wbIoPath = "webassembly/cmake/modules/wb_io.cmake"
    $wbPlatformPath = "webassembly/cmake/modules/wb_platform.cmake"
    $wbRenderPath = "webassembly/cmake/modules/wb_render.cmake"
    $wbIoText = if (Test-Path $wbIoPath) { Get-Content -Raw -Encoding utf8 $wbIoPath } else { "" }
    $wbPlatformText = if (Test-Path $wbPlatformPath) { Get-Content -Raw -Encoding utf8 $wbPlatformPath } else { "" }
    $wbRenderText = if (Test-Path $wbRenderPath) { Get-Content -Raw -Encoding utf8 $wbRenderPath } else { "" }

    $wbIoExpectedEntries = @(
        "webassembly/src/io/application/import_entry_service.cpp",
        "webassembly/src/io/infrastructure/unv/unv_mesh_reader.cpp"
    )
    $wbIoForbiddenEntries = @(
        "webassembly/src/file_loader.cpp",
        "webassembly/src/unv_reader.cpp"
    )

    $wbPlatformExpectedEntries = @(
        "webassembly/src/io/platform/memfs_chunk_stream.cpp",
        "webassembly/src/platform/browser/browser_file_dialog_adapter.cpp",
        "webassembly/src/platform/worker/emscripten_worker_port.cpp",
        "webassembly/src/platform/worker/runtime_progress_port.cpp",
        "webassembly/src/platform/persistence/viewer_preferences_store.cpp"
    )
    $wbPlatformForbiddenEntries = @(
        "webassembly/src/file_loader.cpp"
    )

    $wbRenderExpectedEntries = @(
        "webassembly/src/render/presentation/viewer_window.cpp",
        "webassembly/src/render/infrastructure/interaction/mouse_interactor_style.cpp"
    )
    $wbRenderForbiddenEntries = @(
        "webassembly/src/vtk_viewer.cpp",
        "webassembly/src/mouse_interactor_style.cpp"
    )

    $wbIoMissingEntries = Get-CMakeMissingEntries -CmakeText $wbIoText -Entries $wbIoExpectedEntries
    $wbIoLegacyEntries = Get-CMakePresentEntries -CmakeText $wbIoText -Entries $wbIoForbiddenEntries
    $wbPlatformMissingEntries = Get-CMakeMissingEntries -CmakeText $wbPlatformText -Entries $wbPlatformExpectedEntries
    $wbPlatformLegacyEntries = Get-CMakePresentEntries -CmakeText $wbPlatformText -Entries $wbPlatformForbiddenEntries
    $wbRenderMissingEntries = Get-CMakeMissingEntries -CmakeText $wbRenderText -Entries $wbRenderExpectedEntries
    $wbRenderLegacyEntries = Get-CMakePresentEntries -CmakeText $wbRenderText -Entries $wbRenderForbiddenEntries

    $legacyIncludeMatchesRaw = & rg -n '#include\s+"(file_loader\.h|unv_reader\.h|vtk_viewer\.h|mouse_interactor_style\.h)"' webassembly/src 2>$null
    if ($LASTEXITCODE -notin @(0, 1)) {
        throw "Failed to execute include scan for R3 migration check."
    }
    $legacyIncludeMatches = @($legacyIncludeMatchesRaw)

    $results = @(
        (New-Result "P17R3.legacy_root_files_removed" ($legacyStillPresent.Count -eq 0) $legacyStillPresent.Count 0),
        (New-Result "P17R3.migrated_files_present" ($migratedMissing.Count -eq 0) $migratedMissing.Count 0),
        (New-Result "P17R3.wb_io_sources_registered" ($wbIoMissingEntries.Count -eq 0) $wbIoMissingEntries.Count 0),
        (New-Result "P17R3.wb_io_legacy_sources_zero" ($wbIoLegacyEntries.Count -eq 0) $wbIoLegacyEntries.Count 0),
        (New-Result "P17R3.wb_platform_sources_registered" ($wbPlatformMissingEntries.Count -eq 0) $wbPlatformMissingEntries.Count 0),
        (New-Result "P17R3.wb_platform_legacy_sources_zero" ($wbPlatformLegacyEntries.Count -eq 0) $wbPlatformLegacyEntries.Count 0),
        (New-Result "P17R3.wb_render_sources_registered" ($wbRenderMissingEntries.Count -eq 0) $wbRenderMissingEntries.Count 0),
        (New-Result "P17R3.wb_render_legacy_sources_zero" ($wbRenderLegacyEntries.Count -eq 0) $wbRenderLegacyEntries.Count 0),
        (New-Result "P17R3.legacy_include_reference_zero" ($legacyIncludeMatches.Count -eq 0) $legacyIncludeMatches.Count 0)
    )

    Write-Host ("Phase 17-R3 Render/Platform/IO Migration Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host "Phase 17-R3 migration snapshot:"
    Write-Host (" - legacy root files present: {0}" -f $legacyStillPresent.Count)
    Write-Host (" - migrated files missing: {0}" -f $migratedMissing.Count)
    Write-Host (" - legacy include references: {0}" -f $legacyIncludeMatches.Count)

    if ($legacyStillPresent.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R3.legacy_root_files_removed] Remaining legacy files:"
        foreach ($entry in $legacyStillPresent) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($migratedMissing.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R3.migrated_files_present] Missing migrated files:"
        foreach ($entry in $migratedMissing) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($wbIoMissingEntries.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R3.wb_io_sources_registered] Missing wb_io entries:"
        foreach ($entry in $wbIoMissingEntries) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($wbIoLegacyEntries.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R3.wb_io_legacy_sources_zero] Forbidden wb_io entries:"
        foreach ($entry in $wbIoLegacyEntries) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($wbPlatformMissingEntries.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R3.wb_platform_sources_registered] Missing wb_platform entries:"
        foreach ($entry in $wbPlatformMissingEntries) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($wbPlatformLegacyEntries.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R3.wb_platform_legacy_sources_zero] Forbidden wb_platform entries:"
        foreach ($entry in $wbPlatformLegacyEntries) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($wbRenderMissingEntries.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R3.wb_render_sources_registered] Missing wb_render entries:"
        foreach ($entry in $wbRenderMissingEntries) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($wbRenderLegacyEntries.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R3.wb_render_legacy_sources_zero] Forbidden wb_render entries:"
        foreach ($entry in $wbRenderLegacyEntries) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($legacyIncludeMatches.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R3.legacy_include_reference_zero] Legacy include references:"
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
