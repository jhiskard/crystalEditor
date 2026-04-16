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
    $appPath = "webassembly/src/app.cpp"
    $controllerPath = "webassembly/src/shell/application/workbench_controller.cpp"

    $appText = Get-TextWithoutComments -Path $appPath
    $controllerText = Get-TextWithoutComments -Path $controllerPath

    $consumeBootstrapSetsFlagFalse = [regex]::IsMatch(
        $controllerText,
        '(?s)void\s+consumeInitialLayoutBootstrap\s*\(.*?\)\s*\{\s*state\.shouldApplyInitialLayout\s*=\s*false\s*;\s*\}')

    $openEditorConsumesBootstrap = [regex]::IsMatch(
        $controllerText,
        '(?s)void\s+WorkbenchController::OpenEditorPanel\s*\(.*?\)\s*\{.*?consumeInitialLayoutBootstrap\s*\(\s*state\s*\)\s*;')
    $openBuilderConsumesBootstrap = [regex]::IsMatch(
        $controllerText,
        '(?s)void\s+WorkbenchController::OpenBuilderPanel\s*\(.*?\)\s*\{.*?consumeInitialLayoutBootstrap\s*\(\s*state\s*\)\s*;')
    $openDataConsumesBootstrap = [regex]::IsMatch(
        $controllerText,
        '(?s)void\s+WorkbenchController::OpenDataPanel\s*\(.*?\)\s*\{.*?consumeInitialLayoutBootstrap\s*\(\s*state\s*\)\s*;')
    $requestLayoutConsumesBootstrap = [regex]::IsMatch(
        $controllerText,
        '(?s)void\s+WorkbenchController::RequestLayoutPreset\s*\(.*?\)\s*\{.*?consumeInitialLayoutBootstrap\s*\(\s*state\s*\)\s*;')

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

    $layoutConsumesShellBootstrapFlag = [regex]::IsMatch(
        $layoutRegionText,
        'GetWorkbenchRuntime\(\)\.ShellStateCommand\(\)\.MutableState\(\)\.shouldApplyInitialLayout\s*=\s*false\s*;')
    $layoutResetsPendingShellPreset = [regex]::IsMatch(
        $appText,
        '(?s)GetWorkbenchRuntime\(\)\.ShellStateCommand\(\)\.MutableState\(\)\.pendingLayoutPreset\s*=\s*shell::domain::ShellLayoutPreset::None\s*;')

    $menuEditorSync = [regex]::IsMatch(
        $appText,
        '(?s)auto\s+openEditorPanel\s*=\s*\[.*?\]\s*\(.*?\)\s*\{\s*controller\.OpenEditorPanel\(action\);\s*syncShellStateFromStore\(\);\s*\};')
    $menuBuilderSync = [regex]::IsMatch(
        $appText,
        '(?s)auto\s+openBuilderPanel\s*=\s*\[.*?\]\s*\(.*?\)\s*\{\s*controller\.OpenBuilderPanel\(action\);\s*syncShellStateFromStore\(\);\s*\};')
    $menuDataSync = [regex]::IsMatch(
        $appText,
        '(?s)auto\s+openDataPanel\s*=\s*\[.*?\]\s*\(.*?\)\s*\{\s*controller\.OpenDataPanel\(action\);\s*syncShellStateFromStore\(\);\s*\};')

    $resetGeometryPassConfigured = [regex]::IsMatch(
        $appText,
        '(?s)LayoutPreset::ResetDocking.*?m_ResetWindowGeometryPassesRemaining\s*=\s*2\s*;')

    $layoutButtonRequestCount = [regex]::Matches(
        $appText,
        'controller\.RequestLayoutPreset\s*\(\s*shell::domain::ShellLayoutPreset::(DefaultFloating|DockRight|DockBottom|ResetDocking)\s*\)\s*;').Count

    $results = @(
        (New-Result "P17R5L.consume_bootstrap_sets_flag_false" $consumeBootstrapSetsFlagFalse ([int]$consumeBootstrapSetsFlagFalse) 1),
        (New-Result "P17R5L.open_editor_consumes_bootstrap" $openEditorConsumesBootstrap ([int]$openEditorConsumesBootstrap) 1),
        (New-Result "P17R5L.open_builder_consumes_bootstrap" $openBuilderConsumesBootstrap ([int]$openBuilderConsumesBootstrap) 1),
        (New-Result "P17R5L.open_data_consumes_bootstrap" $openDataConsumesBootstrap ([int]$openDataConsumesBootstrap) 1),
        (New-Result "P17R5L.request_layout_consumes_bootstrap" $requestLayoutConsumesBootstrap ([int]$requestLayoutConsumesBootstrap) 1),
        (New-Result "P17R5L.layout_region_found" $layoutRegionFound ([int]$layoutRegionFound) 1),
        (New-Result "P17R5L.layout_consumes_shell_bootstrap_flag" $layoutConsumesShellBootstrapFlag ([int]$layoutConsumesShellBootstrapFlag) 1),
        (New-Result "P17R5L.layout_resets_pending_shell_preset" $layoutResetsPendingShellPreset ([int]$layoutResetsPendingShellPreset) 1),
        (New-Result "P17R5L.menu_editor_sync_immediate" $menuEditorSync ([int]$menuEditorSync) 1),
        (New-Result "P17R5L.menu_builder_sync_immediate" $menuBuilderSync ([int]$menuBuilderSync) 1),
        (New-Result "P17R5L.menu_data_sync_immediate" $menuDataSync ([int]$menuDataSync) 1),
        (New-Result "P17R5L.reset_geometry_pass_configured" $resetGeometryPassConfigured ([int]$resetGeometryPassConfigured) 1),
        (New-Result "P17R5L.layout_button_request_count_expected" ($layoutButtonRequestCount -ge 4) $layoutButtonRequestCount ">= 4")
    )

    Write-Host ("Phase 17-R5 Shell Layout/Reset Contract Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host "Phase 17-R5 shell-layout snapshot:"
    Write-Host (" - layout region found: {0}" -f $layoutRegionFound)
    Write-Host (" - layout button request count: {0}" -f $layoutButtonRequestCount)
    Write-Host (" - menu sync flags: editor={0}, builder={1}, data={2}" -f $menuEditorSync, $menuBuilderSync, $menuDataSync)

    $violations = @($results | Where-Object { -not $_.Pass })
    if ($violations.Count -gt 0 -and $FailOnViolation) {
        exit 1
    }

    exit 0
}
finally {
    Pop-Location
}
