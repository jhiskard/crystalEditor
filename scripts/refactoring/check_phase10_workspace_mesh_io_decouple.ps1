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
    $workspaceStoreHeaderPath = "webassembly/src/workspace/domain/workspace_store.h"
    $workspaceQueryHeaderPath = "webassembly/src/workspace/application/workspace_query_service.h"
    $workspaceCommandHeaderPath = "webassembly/src/workspace/application/workspace_command_service.h"
    $meshRepositoryHeaderPath = "webassembly/src/mesh/domain/mesh_repository.h"
    $meshQueryHeaderPath = "webassembly/src/mesh/application/mesh_query_service.h"
    $meshCommandHeaderPath = "webassembly/src/mesh/application/mesh_command_service.h"
    $importWorkflowHeaderPath = "webassembly/src/io/application/import_workflow_service.h"
    $importWorkflowSourcePath = "webassembly/src/io/application/import_workflow_service.cpp"

    $fileLoaderHeaderPath = "webassembly/src/file_loader.h"
    $fileLoaderSourcePath = "webassembly/src/file_loader.cpp"
    $importOrchestratorPath = "webassembly/src/io/application/import_orchestrator.cpp"
    $modelTreePath = "webassembly/src/model_tree.cpp"
    $meshGroupDetailPath = "webassembly/src/mesh_group_detail.cpp"
    $modelTreeMeshSectionPath = "webassembly/src/mesh/presentation/model_tree_mesh_section.cpp"
    $modelTreeStructureSectionPath = "webassembly/src/mesh/presentation/model_tree_structure_section.cpp"
    $modelTreeDialogsPath = "webassembly/src/mesh/presentation/model_tree_dialogs.cpp"
    $meshDetailPath = "webassembly/src/mesh_detail.cpp"

    $runtimeSourcePath = "webassembly/src/shell/runtime/workbench_runtime.cpp"
    $mainPath = "webassembly/src/main.cpp"
    $chargeDensityUiPath = "webassembly/src/atoms/ui/charge_density_ui.cpp"
    $bugLogPath = "docs/refactoring/phase10/logs/bug_p10_vasp_grid_sequence_latest.md"

    $featurePanelFiles = @(
        $modelTreePath,
        $meshGroupDetailPath,
        $modelTreeMeshSectionPath,
        $modelTreeStructureSectionPath,
        $modelTreeDialogsPath
    )

    $panelMeshManagerInstanceCount = 0
    foreach ($path in $featurePanelFiles) {
        $panelMeshManagerInstanceCount += Get-MatchCountWithoutComments -Path $path -Pattern "MeshManager::Instance\s*\("
    }

    $meshDetailMeshManagerInstanceCount = Get-MatchCountWithoutComments -Path $meshDetailPath -Pattern "MeshManager::Instance\s*\("
    $importOrchestratorMeshManagerInstanceCount = Get-MatchCountWithoutComments -Path $importOrchestratorPath -Pattern "MeshManager::Instance\s*\("
    $importWorkflowMeshManagerInstanceCount = Get-MatchCountWithoutComments -Path $importWorkflowSourcePath -Pattern "MeshManager::Instance\s*\("

    $panelFileLoaderInstanceCount = 0
    foreach ($path in $featurePanelFiles) {
        $panelFileLoaderInstanceCount += Get-MatchCountWithoutComments -Path $path -Pattern "FileLoader::Instance\s*\("
    }

    $sourceFiles = @(
        Get-ChildItem "webassembly/src" -Recurse -Include *.h,*.cpp |
            Where-Object { $_.FullName.Replace('\', '/') -notlike "*/render/*" } |
            Select-Object -ExpandProperty FullName
    )
    $nonRenderViewerInstanceCount = 0
    foreach ($path in $sourceFiles) {
        $nonRenderViewerInstanceCount += Get-MatchCountWithoutComments -Path $path -Pattern "VtkViewer::Instance\s*\("
    }

    $fileLoaderHeaderRaw = if (Test-Path $fileLoaderHeaderPath) { Get-Content -Raw -Encoding utf8 $fileLoaderHeaderPath } else { "" }
    $fileLoaderSourceRaw = if (Test-Path $fileLoaderSourcePath) { Get-Content -Raw -Encoding utf8 $fileLoaderSourcePath } else { "" }
    $importWorkflowHeaderRaw = if (Test-Path $importWorkflowHeaderPath) { Get-Content -Raw -Encoding utf8 $importWorkflowHeaderPath } else { "" }
    $workspaceStoreHeaderRaw = if (Test-Path $workspaceStoreHeaderPath) { Get-Content -Raw -Encoding utf8 $workspaceStoreHeaderPath } else { "" }
    $meshRepositoryHeaderRaw = if (Test-Path $meshRepositoryHeaderPath) { Get-Content -Raw -Encoding utf8 $meshRepositoryHeaderPath } else { "" }
    $runtimeSourceRaw = if (Test-Path $runtimeSourcePath) { Get-Content -Raw -Encoding utf8 $runtimeSourcePath } else { "" }
    $mainRaw = if (Test-Path $mainPath) { Get-Content -Raw -Encoding utf8 $mainPath } else { "" }
    $chargeDensityUiRaw = if (Test-Path $chargeDensityUiPath) { Get-Content -Raw -Encoding utf8 $chargeDensityUiPath } else { "" }
    $bugLogRaw = if (Test-Path $bugLogPath) { Get-Content -Raw -Encoding utf8 $bugLogPath } else { "" }

    $fileLoaderUsesWorkflowService = (
        $fileLoaderHeaderRaw.Contains("ImportWorkflowService") -and
        $fileLoaderSourceRaw.Contains("m_ImportWorkflowService")
    )

    $workspaceStoreHasDoxygen = (
        $workspaceStoreHeaderRaw.Contains("@brief") -and
        $workspaceStoreHeaderRaw.Contains("active workspace context ids")
    )
    $meshRepositoryHasDoxygen = (
        $meshRepositoryHeaderRaw.Contains("@brief") -and
        $meshRepositoryHeaderRaw.Contains("centralizes mesh access/mutation")
    )
    $importWorkflowHasDoxygen = (
        $importWorkflowHeaderRaw.Contains("@brief") -and
        $importWorkflowHeaderRaw.Contains("replace-scene import transaction")
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

    $results = @(
        (New-Result "P10.workspace_store_header_exists" (Test-Path $workspaceStoreHeaderPath) ([int](Test-Path $workspaceStoreHeaderPath)) 1),
        (New-Result "P10.workspace_query_header_exists" (Test-Path $workspaceQueryHeaderPath) ([int](Test-Path $workspaceQueryHeaderPath)) 1),
        (New-Result "P10.workspace_command_header_exists" (Test-Path $workspaceCommandHeaderPath) ([int](Test-Path $workspaceCommandHeaderPath)) 1),
        (New-Result "P10.mesh_repository_header_exists" (Test-Path $meshRepositoryHeaderPath) ([int](Test-Path $meshRepositoryHeaderPath)) 1),
        (New-Result "P10.mesh_query_header_exists" (Test-Path $meshQueryHeaderPath) ([int](Test-Path $meshQueryHeaderPath)) 1),
        (New-Result "P10.mesh_command_header_exists" (Test-Path $meshCommandHeaderPath) ([int](Test-Path $meshCommandHeaderPath)) 1),
        (New-Result "P10.import_workflow_header_exists" (Test-Path $importWorkflowHeaderPath) ([int](Test-Path $importWorkflowHeaderPath)) 1),
        (New-Result "P10.import_workflow_source_exists" (Test-Path $importWorkflowSourcePath) ([int](Test-Path $importWorkflowSourcePath)) 1),

        (New-Result "P10.file_loader_uses_import_workflow_service" $fileLoaderUsesWorkflowService ([int]$fileLoaderUsesWorkflowService) 1),
        (New-Result "P10.import_orchestrator_mesh_manager_instance_zero" ($importOrchestratorMeshManagerInstanceCount -eq 0) $importOrchestratorMeshManagerInstanceCount 0),
        (New-Result "P10.import_workflow_mesh_manager_instance_zero" ($importWorkflowMeshManagerInstanceCount -eq 0) $importWorkflowMeshManagerInstanceCount 0),
        (New-Result "P10.panel_mesh_manager_instance_zero" ($panelMeshManagerInstanceCount -eq 0) $panelMeshManagerInstanceCount 0),
        (New-Result "P10.panel_file_loader_instance_zero" ($panelFileLoaderInstanceCount -eq 0) $panelFileLoaderInstanceCount 0),
        (New-Result "P10.mesh_detail_mesh_manager_instance_budget" ($meshDetailMeshManagerInstanceCount -le 33) $meshDetailMeshManagerInstanceCount "<= 33"),

        (New-Result "P10.workspace_store_public_doxygen" $workspaceStoreHasDoxygen ([int]$workspaceStoreHasDoxygen) 1),
        (New-Result "P10.mesh_repository_public_doxygen" $meshRepositoryHasDoxygen ([int]$meshRepositoryHasDoxygen) 1),
        (New-Result "P10.import_workflow_public_doxygen" $importWorkflowHasDoxygen ([int]$importWorkflowHasDoxygen) 1),

        (New-Result "P10.non_render_vtk_viewer_instance_calls" ($nonRenderViewerInstanceCount -eq 0) $nonRenderViewerInstanceCount 0),
        (New-Result "P10.runtime_prime_excludes_font_registry" (-not $primeIncludesFontRegistry) ([int]$primeIncludesFontRegistry) 0),
        (New-Result "P10.main_font_registry_after_imgui_context" $mainFontRegistryAfterImGuiContext ([int]$mainFontRegistryAfterImGuiContext) 1),
        (New-Result "P10.xsf_first_grid_bootstrap_guard" $xsfBootstrapGuardPresent ([int]$xsfBootstrapGuardPresent) 1),
        (New-Result "P10.bug_log_exists" $bugLogExists ([int]$bugLogExists) 1),
        (New-Result "P10.bug_log_has_bug_id" $bugLogHasBugId ([int]$bugLogHasBugId) 1),
        (New-Result "P10.bug_log_has_status_tag" $bugLogHasStatusTag ([int]$bugLogHasStatusTag) 1)
    )

    Write-Host ("Phase 10 Workspace/Mesh/IO Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host "Phase 10 inventory snapshot:"
    Write-Host (" - MeshManager::Instance() in panel scope: {0}" -f $panelMeshManagerInstanceCount)
    Write-Host (" - MeshManager::Instance() in mesh_detail.cpp: {0}" -f $meshDetailMeshManagerInstanceCount)
    Write-Host (" - FileLoader::Instance() in panel scope: {0}" -f $panelFileLoaderInstanceCount)
    Write-Host (" - Import orchestrator/workflow direct MeshManager::Instance(): {0}/{1}" -f $importOrchestratorMeshManagerInstanceCount, $importWorkflowMeshManagerInstanceCount)
    Write-Host (" - non-render VtkViewer::Instance() count: {0}" -f $nonRenderViewerInstanceCount)
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

