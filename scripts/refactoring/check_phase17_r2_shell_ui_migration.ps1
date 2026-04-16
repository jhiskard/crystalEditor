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

function Get-MissingPaths {
    param(
        [string[]]$Paths
    )

    return @($Paths | Where-Object { -not (Test-Path $_) })
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
    $requiredLegacyRootFiles = @(
        "webassembly/src/font_manager.cpp",
        "webassembly/src/font_manager.h",
        "webassembly/src/custom_ui.cpp",
        "webassembly/src/custom_ui.h",
        "webassembly/src/test_window.cpp",
        "webassembly/src/test_window.h",
        "webassembly/src/toolbar.cpp",
        "webassembly/src/toolbar.h"
    )
    $requiredMigratedFiles = @(
        "webassembly/src/shell/presentation/font/font_registry.cpp",
        "webassembly/src/shell/presentation/font/font_registry.h",
        "webassembly/src/shell/presentation/widgets/icon_button.cpp",
        "webassembly/src/shell/presentation/widgets/icon_button.h",
        "webassembly/src/shell/presentation/debug/test_window_panel.cpp",
        "webassembly/src/shell/presentation/debug/test_window_panel.h",
        "webassembly/src/shell/presentation/toolbar/viewer_toolbar_presenter.cpp",
        "webassembly/src/shell/presentation/toolbar/viewer_toolbar_presenter.h"
    )

    $optionalLegacyFiles = @(
        "webassembly/src/image.cpp",
        "webassembly/src/image.h",
        "webassembly/src/texture.cpp",
        "webassembly/src/texture.h"
    )
    $optionalMigratedFiles = @(
        "webassembly/src/render/infrastructure/resources/image_loader.cpp",
        "webassembly/src/render/infrastructure/resources/image_loader.h",
        "webassembly/src/render/infrastructure/resources/texture_resource.cpp",
        "webassembly/src/render/infrastructure/resources/texture_resource.h"
    )

    $requiredLegacyStillPresent = @($requiredLegacyRootFiles | Where-Object { Test-Path $_ })
    $requiredMigratedMissing = Get-MissingPaths -Paths $requiredMigratedFiles

    $optionalLegacyMissing = Get-MissingPaths -Paths $optionalLegacyFiles
    $optionalMigratedMissing = Get-MissingPaths -Paths $optionalMigratedFiles
    $optionalLegacyAllPresent = ($optionalLegacyMissing.Count -eq 0)
    $optionalMigratedAllPresent = ($optionalMigratedMissing.Count -eq 0)
    $optionalLegacyAnyPresent = ($optionalLegacyMissing.Count -lt $optionalLegacyFiles.Count)
    $optionalMigratedAnyPresent = ($optionalMigratedMissing.Count -lt $optionalMigratedFiles.Count)

    $optionalState = "mixed_or_missing"
    if ($optionalMigratedAllPresent -and -not $optionalLegacyAnyPresent) {
        $optionalState = "migrated"
    }
    elseif ($optionalLegacyAllPresent -and -not $optionalMigratedAnyPresent) {
        $optionalState = "legacy"
    }
    $optionalStateConsistent = $optionalState -ne "mixed_or_missing"

    $cmakePaths = @(
        "webassembly/cmake/modules/wb_shell.cmake",
        "webassembly/cmake/modules/wb_render.cmake"
    )
    $cmakeText = @(
        $cmakePaths |
            Where-Object { Test-Path $_ } |
            ForEach-Object { Get-Content -Raw -Encoding utf8 $_ }
    ) -join "`n"

    $requiredCmakeEntries = @(
        "webassembly/src/shell/presentation/widgets/icon_button.cpp",
        "webassembly/src/shell/presentation/font/font_registry.cpp",
        "webassembly/src/shell/presentation/debug/test_window_panel.cpp",
        "webassembly/src/shell/presentation/toolbar/viewer_toolbar_presenter.cpp"
    )
    $requiredForbiddenCmakeEntries = @(
        "webassembly/src/custom_ui.cpp",
        "webassembly/src/font_manager.cpp",
        "webassembly/src/test_window.cpp",
        "webassembly/src/toolbar.cpp"
    )

    $optionalExpectedCmakeEntries = @()
    $optionalForbiddenCmakeEntries = @()
    if ($optionalState -eq "migrated") {
        $optionalExpectedCmakeEntries = @(
            "webassembly/src/render/infrastructure/resources/image_loader.cpp",
            "webassembly/src/render/infrastructure/resources/texture_resource.cpp"
        )
        $optionalForbiddenCmakeEntries = @(
            "webassembly/src/image.cpp",
            "webassembly/src/texture.cpp"
        )
    }
    elseif ($optionalState -eq "legacy") {
        $optionalExpectedCmakeEntries = @(
            "webassembly/src/image.cpp",
            "webassembly/src/texture.cpp"
        )
        $optionalForbiddenCmakeEntries = @(
            "webassembly/src/render/infrastructure/resources/image_loader.cpp",
            "webassembly/src/render/infrastructure/resources/texture_resource.cpp"
        )
    }
    else {
        # optional state가 혼합이면 CMake 정합성은 별도 실패로 귀결된다.
        $optionalExpectedCmakeEntries = @(
            "webassembly/src/render/infrastructure/resources/image_loader.cpp",
            "webassembly/src/render/infrastructure/resources/texture_resource.cpp"
        )
        $optionalForbiddenCmakeEntries = @(
            "webassembly/src/image.cpp",
            "webassembly/src/texture.cpp"
        )
    }

    $cmakeMissingRequiredEntries = Get-CMakeMissingEntries -CmakeText $cmakeText -Entries @($requiredCmakeEntries + $optionalExpectedCmakeEntries)
    $cmakePresentForbiddenEntries = Get-CMakePresentEntries -CmakeText $cmakeText -Entries @($requiredForbiddenCmakeEntries + $optionalForbiddenCmakeEntries)

    $toolbarPath = "webassembly/src/shell/presentation/toolbar/viewer_toolbar_presenter.cpp"
    $toolbarText = Get-TextWithoutComments -Path $toolbarPath
    $toolbarSingletonCount = [regex]::Matches($toolbarText, 'AtomsTemplate::Instance\s*\(').Count

    $appPath = "webassembly/src/app.cpp"
    $appText = Get-TextWithoutComments -Path $appPath
    $hasLayoutHelper = [regex]::IsMatch($appText, 'applyLayoutVisibilityPreset\s*=')
    $setWindowVisibleCount = [regex]::Matches($appText, 'SetWindowVisible\s*\(').Count
    $hasLayoutCommandPath = $hasLayoutHelper -and $setWindowVisibleCount -gt 0

    $editorPattern = '(?s)auto\s+openEditorPanel\s*=\s*\[.*?\]\s*\(.*?\)\s*\{\s*controller\.OpenEditorPanel\(action\);\s*syncShellStateFromStore\(\);\s*\};'
    $builderPattern = '(?s)auto\s+openBuilderPanel\s*=\s*\[.*?\]\s*\(.*?\)\s*\{\s*controller\.OpenBuilderPanel\(action\);\s*syncShellStateFromStore\(\);\s*\};'
    $dataPattern = '(?s)auto\s+openDataPanel\s*=\s*\[.*?\]\s*\(.*?\)\s*\{\s*controller\.OpenDataPanel\(action\);\s*syncShellStateFromStore\(\);\s*\};'
    $menuDelegationSynced =
        [regex]::IsMatch($appText, $editorPattern) -and
        [regex]::IsMatch($appText, $builderPattern) -and
        [regex]::IsMatch($appText, $dataPattern)

    $layoutStartToken = "if (m_PendingLayoutPreset != LayoutPreset::None) {"
    $layoutEndToken = "m_PendingLayoutPreset = LayoutPreset::None;"
    $layoutStartIndex = $appText.IndexOf($layoutStartToken)
    $layoutEndIndex = if ($layoutStartIndex -ge 0) { $appText.IndexOf($layoutEndToken, $layoutStartIndex) } else { -1 }
    $layoutRegionFound = ($layoutStartIndex -ge 0 -and $layoutEndIndex -gt $layoutStartIndex)
    $layoutRegionText = if ($layoutRegionFound) {
        $appText.Substring($layoutStartIndex, $layoutEndIndex - $layoutStartIndex)
    }
    else {
        ""
    }
    $layoutDirectShowAssignCount = [regex]::Matches($layoutRegionText, 'm_bShow[A-Za-z0-9_]+\s*=').Count

    $menuDelegationCurrent = if ($menuDelegationSynced) { 0 } else { 1 }
    $layoutRegionCurrent = if ($layoutRegionFound) { 0 } else { 1 }

    $results = @(
        (New-Result "P17R2.required_legacy_root_files_removed" ($requiredLegacyStillPresent.Count -eq 0) $requiredLegacyStillPresent.Count 0),
        (New-Result "P17R2.required_migrated_files_present" ($requiredMigratedMissing.Count -eq 0) $requiredMigratedMissing.Count 0),
        (New-Result "P17R2.optional_image_texture_state_consistent" $optionalStateConsistent $optionalState "legacy_or_migrated_consistent"),
        (New-Result "P17R2.shell_render_sources_registered" ($cmakeMissingRequiredEntries.Count -eq 0) $cmakeMissingRequiredEntries.Count 0),
        (New-Result "P17R2.shell_render_legacy_sources_zero" ($cmakePresentForbiddenEntries.Count -eq 0) $cmakePresentForbiddenEntries.Count 0),
        (New-Result "P17R2.toolbar_singleton_direct_call_zero" ($toolbarSingletonCount -eq 0) $toolbarSingletonCount 0),
        (New-Result "P17R2.app_layout_command_helper_present" $hasLayoutCommandPath $setWindowVisibleCount ">= 1"),
        (New-Result "P17R2.app_menu_open_actions_synced" $menuDelegationSynced $menuDelegationCurrent 0),
        (New-Result "P17R2.app_layout_region_found" $layoutRegionFound $layoutRegionCurrent 0),
        (New-Result "P17R2.app_layout_direct_show_assign_zero" ($layoutDirectShowAssignCount -eq 0) $layoutDirectShowAssignCount 0)
    )

    Write-Host ("Phase 17-R2 Shell/UI Migration Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host "Phase 17-R2 migration snapshot:"
    Write-Host (" - optional image/texture state: {0}" -f $optionalState)
    Write-Host (" - SetWindowVisible count in app.cpp: {0}" -f $setWindowVisibleCount)
    Write-Host (" - layout direct show assign count: {0}" -f $layoutDirectShowAssignCount)

    if ($requiredLegacyStillPresent.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R2.required_legacy_root_files_removed] Remaining legacy root files:"
        foreach ($entry in $requiredLegacyStillPresent) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($requiredMigratedMissing.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R2.required_migrated_files_present] Missing migrated files:"
        foreach ($entry in $requiredMigratedMissing) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if (-not $optionalStateConsistent) {
        Write-Host ""
        Write-Host "[P17R2.optional_image_texture_state_consistent] Optional file-state mismatch:"
        foreach ($entry in $optionalLegacyFiles) {
            Write-Host (" - legacy {0}: {1}" -f $entry, (Test-Path $entry))
        }
        foreach ($entry in $optionalMigratedFiles) {
            Write-Host (" - migrated {0}: {1}" -f $entry, (Test-Path $entry))
        }
    }

    if ($cmakeMissingRequiredEntries.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R2.shell_render_sources_registered] Missing CMake source entries:"
        foreach ($entry in $cmakeMissingRequiredEntries) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($cmakePresentForbiddenEntries.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R2.shell_render_legacy_sources_zero] Forbidden CMake source entries:"
        foreach ($entry in $cmakePresentForbiddenEntries) {
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
