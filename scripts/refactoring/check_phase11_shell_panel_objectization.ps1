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

function Get-MatchCountWithoutComments {
    param(
        [string]$Path,
        [string]$Pattern
    )

    $text = Get-TextWithoutComments -Path $Path
    if ([string]::IsNullOrEmpty($text)) {
        return 0
    }

    $matches = [regex]::Matches($text, $Pattern)
    return $matches.Count
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    $appHeaderPath = "webassembly/src/app.h"
    $toolbarHeaderPath = "webassembly/src/toolbar.h"
    $modelTreeHeaderPath = "webassembly/src/model_tree.h"
    $meshDetailHeaderPath = "webassembly/src/mesh_detail.h"
    $meshGroupDetailHeaderPath = "webassembly/src/mesh_group_detail.h"
    $testWindowHeaderPath = "webassembly/src/test_window.h"

    $appSourcePath = "webassembly/src/app.cpp"
    $toolbarSourcePath = "webassembly/src/toolbar.cpp"
    $meshDetailSourcePath = "webassembly/src/mesh_detail.cpp"
    $runtimeSourcePath = "webassembly/src/shell/runtime/workbench_runtime.cpp"

    $shellStateStoreHeaderPath = "webassembly/src/shell/domain/shell_state_store.h"
    $shellStateStoreSourcePath = "webassembly/src/shell/domain/shell_state_store.cpp"
    $workbenchControllerHeaderPath = "webassembly/src/shell/application/workbench_controller.h"
    $workbenchControllerSourcePath = "webassembly/src/shell/application/workbench_controller.cpp"

    $mainPath = "webassembly/src/main.cpp"
    $chargeDensityUiPath = "webassembly/src/atoms/ui/charge_density_ui.cpp"
    $bugLogPath = "docs/refactoring/phase11/logs/bug_p11_vasp_grid_sequence_latest.md"

    $singletonHeaders = @(
        $appHeaderPath,
        $toolbarHeaderPath,
        $modelTreeHeaderPath,
        $meshDetailHeaderPath,
        $meshGroupDetailHeaderPath,
        $testWindowHeaderPath
    )

    $singletonDeclCount = 0
    foreach ($path in $singletonHeaders) {
        $singletonDeclCount += Get-MatchCountWithoutComments -Path $path -Pattern "DECLARE_SINGLETON\s*\("
    }

    $modelTreeSingletonDeclCount = Get-MatchCountWithoutComments -Path $modelTreeHeaderPath -Pattern "DECLARE_SINGLETON\s*\("
    $meshDetailSingletonDeclCount = Get-MatchCountWithoutComments -Path $meshDetailHeaderPath -Pattern "DECLARE_SINGLETON\s*\("
    $meshGroupDetailSingletonDeclCount = Get-MatchCountWithoutComments -Path $meshGroupDetailHeaderPath -Pattern "DECLARE_SINGLETON\s*\("
    $testWindowSingletonDeclCount = Get-MatchCountWithoutComments -Path $testWindowHeaderPath -Pattern "DECLARE_SINGLETON\s*\("

    $appDirectEditorRequestCount = Get-MatchCountWithoutComments -Path $appSourcePath -Pattern "RequestEditorSection\s*\("
    $appDirectBuilderRequestCount = Get-MatchCountWithoutComments -Path $appSourcePath -Pattern "RequestBuilderSection\s*\("
    $appDirectDataRequestCount = Get-MatchCountWithoutComments -Path $appSourcePath -Pattern "RequestDataMenu\s*\("
    $appDirectMeasurementFeatureCount = Get-MatchCountWithoutComments -Path $appSourcePath -Pattern "MeasurementFeature\s*\("

    $appUsesControllerOpenEditor = Get-MatchCountWithoutComments -Path $appSourcePath -Pattern "controller\.OpenEditorPanel\s*\(" -gt 0
    $appUsesControllerOpenBuilder = Get-MatchCountWithoutComments -Path $appSourcePath -Pattern "controller\.OpenBuilderPanel\s*\(" -gt 0
    $appUsesControllerOpenData = Get-MatchCountWithoutComments -Path $appSourcePath -Pattern "controller\.OpenDataPanel\s*\(" -gt 0
    $appUsesControllerMeasurement = Get-MatchCountWithoutComments -Path $appSourcePath -Pattern "controller\.EnterMeasurementMode\s*\(" -gt 0
    $appUsesControllerImport = Get-MatchCountWithoutComments -Path $appSourcePath -Pattern "controller\.RequestOpenStructureImport\s*\(" -gt 0

    $toolbarDirectMeshManagerInstanceCount = Get-MatchCountWithoutComments -Path $toolbarSourcePath -Pattern "MeshManager::Instance\s*\("
    $toolbarDirectRenderGatewayCount = Get-MatchCountWithoutComments -Path $toolbarSourcePath -Pattern "GetRenderGateway\s*\("

    $meshDetailMeshManagerInstanceCount = Get-MatchCountWithoutComments -Path $meshDetailSourcePath -Pattern "MeshManager::Instance\s*\("

    $nonRenderSourceFiles = @(
        Get-ChildItem "webassembly/src" -Recurse -Include *.h,*.cpp |
            Where-Object { $_.FullName.Replace('\\', '/') -notlike "*/render/*" } |
            Select-Object -ExpandProperty FullName
    )
    $renderSourceFiles = @(
        Get-ChildItem "webassembly/src/render" -Recurse -Include *.h,*.cpp |
            Select-Object -ExpandProperty FullName
    )

    $nonRenderViewerInstanceCount = 0
    foreach ($path in $nonRenderSourceFiles) {
        $nonRenderViewerInstanceCount += Get-MatchCountWithoutComments -Path $path -Pattern "VtkViewer::Instance\s*\("
    }
    $renderViewerInstanceCount = 0
    foreach ($path in $renderSourceFiles) {
        $renderViewerInstanceCount += Get-MatchCountWithoutComments -Path $path -Pattern "VtkViewer::Instance\s*\("
    }

    $runtimeSourceRaw = if (Test-Path $runtimeSourcePath) { Get-Content -Raw -Encoding utf8 $runtimeSourcePath } else { "" }
    $mainRaw = if (Test-Path $mainPath) { Get-Content -Raw -Encoding utf8 $mainPath } else { "" }
    $chargeDensityUiRaw = if (Test-Path $chargeDensityUiPath) { Get-Content -Raw -Encoding utf8 $chargeDensityUiPath } else { "" }
    $bugLogRaw = if (Test-Path $bugLogPath) { Get-Content -Raw -Encoding utf8 $bugLogPath } else { "" }

    $runtimeUsesRuntimeOwnedPanels = (
        $runtimeSourceRaw.Contains("static ModelTree panel") -and
        $runtimeSourceRaw.Contains("static MeshDetail panel") -and
        $runtimeSourceRaw.Contains("static MeshGroupDetail panel") -and
        $runtimeSourceRaw.Contains("static TestWindow panel")
    )

    $runtimePanelAccessorCallsLegacyInstance = (
        ([regex]::IsMatch($runtimeSourceRaw, "ModelTree::Instance\s*\(")) -or
        ([regex]::IsMatch($runtimeSourceRaw, "MeshDetail::Instance\s*\(")) -or
        ([regex]::IsMatch($runtimeSourceRaw, "MeshGroupDetail::Instance\s*\(")) -or
        ([regex]::IsMatch($runtimeSourceRaw, "TestWindow::Instance\s*\("))
    )

    $shellStateStoreHeaderRaw = if (Test-Path $shellStateStoreHeaderPath) { Get-Content -Raw -Encoding utf8 $shellStateStoreHeaderPath } else { "" }
    $workbenchControllerHeaderRaw = if (Test-Path $workbenchControllerHeaderPath) { Get-Content -Raw -Encoding utf8 $workbenchControllerHeaderPath } else { "" }

    $shellStateStoreHasDoxygen = (
        $shellStateStoreHeaderRaw.Contains("@brief") -and
        $shellStateStoreHeaderRaw.Contains("shell")
    )
    $workbenchControllerHasDoxygen = (
        $workbenchControllerHeaderRaw.Contains("@brief") -and
        $workbenchControllerHeaderRaw.Contains("controller")
    )

    $primeIncludesFontRegistry = $false
    $primeMatch = [regex]::Match(
        $runtimeSourceRaw,
        "void\s+WorkbenchRuntime::PrimeLegacySingletons\s*\(\)\s*\{(?<body>.*?)\}",
        [System.Text.RegularExpressions.RegexOptions]::Singleline)
    if ($primeMatch.Success) {
        $primeBody = $primeMatch.Groups["body"].Value
        $primeIncludesFontRegistry = [regex]::IsMatch($primeBody, "FontRegistry\s*\(")
    }

    $imguiCreateContextIndex = $mainRaw.IndexOf("ImGui::CreateContext()")
    $runtimeFontRegistryIndex = $mainRaw.IndexOf("runtime.FontRegistry()")
    $mainFontRegistryAfterImGuiContext = (
        $imguiCreateContextIndex -ge 0 -and
        $runtimeFontRegistryIndex -ge 0 -and
        $runtimeFontRegistryIndex -gt $imguiCreateContextIndex
    )

    $xsfBootstrapGuardPresent = $chargeDensityUiRaw.Contains("if (!loadFromGridEntry(0))")

    $bugLogExists = Test-Path $bugLogPath
    $bugLogHasBugId = $bugLogRaw.Contains("P9-BUG-01")
    $bugLogHasStatusTag = [regex]::IsMatch($bugLogRaw, "Status:\s*`?(Open|Resolved|Deferred)`?")

    $fileLoaderInstanceCount = 0
    $cppFiles = Get-ChildItem "webassembly/src" -Recurse -Include *.cpp | Select-Object -ExpandProperty FullName
    foreach ($path in $cppFiles) {
        $fileLoaderInstanceCount += Get-MatchCountWithoutComments -Path $path -Pattern "FileLoader::Instance\s*\("
    }

    $results = @(
        (New-Result "P11.shell_state_store_header_exists" (Test-Path $shellStateStoreHeaderPath) ([int](Test-Path $shellStateStoreHeaderPath)) 1),
        (New-Result "P11.shell_state_store_source_exists" (Test-Path $shellStateStoreSourcePath) ([int](Test-Path $shellStateStoreSourcePath)) 1),
        (New-Result "P11.workbench_controller_header_exists" (Test-Path $workbenchControllerHeaderPath) ([int](Test-Path $workbenchControllerHeaderPath)) 1),
        (New-Result "P11.workbench_controller_source_exists" (Test-Path $workbenchControllerSourcePath) ([int](Test-Path $workbenchControllerSourcePath)) 1),

        (New-Result "P11.shell_state_store_public_doxygen" $shellStateStoreHasDoxygen ([int]$shellStateStoreHasDoxygen) 1),
        (New-Result "P11.workbench_controller_public_doxygen" $workbenchControllerHasDoxygen ([int]$workbenchControllerHasDoxygen) 1),

        (New-Result "P11.panel_singleton_decl_budget" ($singletonDeclCount -le 2) $singletonDeclCount "<= 2"),
        (New-Result "P11.model_tree_singleton_removed" ($modelTreeSingletonDeclCount -eq 0) $modelTreeSingletonDeclCount 0),
        (New-Result "P11.mesh_detail_singleton_removed" ($meshDetailSingletonDeclCount -eq 0) $meshDetailSingletonDeclCount 0),
        (New-Result "P11.mesh_group_detail_singleton_removed" ($meshGroupDetailSingletonDeclCount -eq 0) $meshGroupDetailSingletonDeclCount 0),
        (New-Result "P11.test_window_singleton_removed" ($testWindowSingletonDeclCount -eq 0) $testWindowSingletonDeclCount 0),

        (New-Result "P11.runtime_owns_panel_instances" $runtimeUsesRuntimeOwnedPanels ([int]$runtimeUsesRuntimeOwnedPanels) 1),
        (New-Result "P11.runtime_accessors_avoid_legacy_instance" (-not $runtimePanelAccessorCallsLegacyInstance) ([int]$runtimePanelAccessorCallsLegacyInstance) 0),

        (New-Result "P11.app_menu_uses_controller_editor" $appUsesControllerOpenEditor ([int]$appUsesControllerOpenEditor) 1),
        (New-Result "P11.app_menu_uses_controller_builder" $appUsesControllerOpenBuilder ([int]$appUsesControllerOpenBuilder) 1),
        (New-Result "P11.app_menu_uses_controller_data" $appUsesControllerOpenData ([int]$appUsesControllerOpenData) 1),
        (New-Result "P11.app_menu_uses_controller_measurement" $appUsesControllerMeasurement ([int]$appUsesControllerMeasurement) 1),
        (New-Result "P11.app_menu_uses_controller_import" $appUsesControllerImport ([int]$appUsesControllerImport) 1),
        (New-Result "P11.app_direct_editor_request_zero" ($appDirectEditorRequestCount -eq 0) $appDirectEditorRequestCount 0),
        (New-Result "P11.app_direct_builder_request_zero" ($appDirectBuilderRequestCount -eq 0) $appDirectBuilderRequestCount 0),
        (New-Result "P11.app_direct_data_request_zero" ($appDirectDataRequestCount -eq 0) $appDirectDataRequestCount 0),
        (New-Result "P11.app_direct_measurement_feature_zero" ($appDirectMeasurementFeatureCount -eq 0) $appDirectMeasurementFeatureCount 0),

        (New-Result "P11.toolbar_mesh_manager_instance_zero" ($toolbarDirectMeshManagerInstanceCount -eq 0) $toolbarDirectMeshManagerInstanceCount 0),
        (New-Result "P11.toolbar_render_gateway_zero" ($toolbarDirectRenderGatewayCount -eq 0) $toolbarDirectRenderGatewayCount 0),
        (New-Result "P11.mesh_detail_mesh_manager_instance_budget" ($meshDetailMeshManagerInstanceCount -le 1) $meshDetailMeshManagerInstanceCount "<= 1"),
        (New-Result "P11.file_loader_instance_budget" ($fileLoaderInstanceCount -le 4) $fileLoaderInstanceCount "<= 4"),

        (New-Result "P11.render_vtk_viewer_instance_budget" ($renderViewerInstanceCount -le 22) $renderViewerInstanceCount "<= 22"),
        (New-Result "P11.non_render_vtk_viewer_instance_budget" ($nonRenderViewerInstanceCount -le 22) $nonRenderViewerInstanceCount "<= 22"),
        (New-Result "P11.runtime_prime_excludes_font_registry" (-not $primeIncludesFontRegistry) ([int]$primeIncludesFontRegistry) 0),
        (New-Result "P11.main_font_registry_after_imgui_context" $mainFontRegistryAfterImGuiContext ([int]$mainFontRegistryAfterImGuiContext) 1),
        (New-Result "P11.xsf_first_grid_bootstrap_guard" $xsfBootstrapGuardPresent ([int]$xsfBootstrapGuardPresent) 1),

        (New-Result "P11.bug_log_exists" $bugLogExists ([int]$bugLogExists) 1),
        (New-Result "P11.bug_log_has_bug_id" $bugLogHasBugId ([int]$bugLogHasBugId) 1),
        (New-Result "P11.bug_log_has_status_tag" $bugLogHasStatusTag ([int]$bugLogHasStatusTag) 1)
    )

    Write-Host ("Phase 11 Shell/Panel Objectization Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host "Phase 11 inventory snapshot:"
    Write-Host (" - DECLARE_SINGLETON count (App/Toolbar/ModelTree/MeshDetail/MeshGroupDetail/TestWindow): {0}" -f $singletonDeclCount)
    Write-Host (" - app direct feature request count (editor/builder/data/measurement): {0}/{1}/{2}/{3}" -f $appDirectEditorRequestCount, $appDirectBuilderRequestCount, $appDirectDataRequestCount, $appDirectMeasurementFeatureCount)
    Write-Host (" - toolbar direct MeshManager/GetRenderGateway count: {0}/{1}" -f $toolbarDirectMeshManagerInstanceCount, $toolbarDirectRenderGatewayCount)
    Write-Host (" - mesh_detail MeshManager::Instance() count: {0}" -f $meshDetailMeshManagerInstanceCount)
    Write-Host (" - FileLoader::Instance() count: {0}" -f $fileLoaderInstanceCount)
    Write-Host (" - render/non-render VtkViewer::Instance() count: {0}/{1}" -f $renderViewerInstanceCount, $nonRenderViewerInstanceCount)
    Write-Host (" - bug log present/id/status: {0}/{1}/{2}" -f $bugLogExists, $bugLogHasBugId, $bugLogHasStatusTag)

    $violations = @($results | Where-Object { -not $_.Pass })
    if ($violations.Count -gt 0 -and $FailOnViolation) {
        exit 1
    }

    exit 0
}
finally {
    Pop-Location
}
