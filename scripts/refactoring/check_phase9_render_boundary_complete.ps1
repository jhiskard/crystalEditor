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
    $renderGatewayHeaderPath = "webassembly/src/render/application/render_gateway.h"
    $vtkRenderGatewayHeaderPath = "webassembly/src/render/infrastructure/vtk_render_gateway.h"
    $vtkRenderGatewaySourcePath = "webassembly/src/render/infrastructure/vtk_render_gateway.cpp"
    $mainPath = "webassembly/src/main.cpp"
    $runtimeSourcePath = "webassembly/src/shell/runtime/workbench_runtime.cpp"
    $chargeDensityUiPath = "webassembly/src/atoms/ui/charge_density_ui.cpp"
    $bugLogPath = "docs/refactoring/phase9/logs/bug_p9_vasp_grid_sequence_latest.md"

    $sourceFiles = @(
        Get-ChildItem "webassembly/src" -Recurse -Include *.h,*.cpp |
            Where-Object { $_.FullName.Replace('\', '/') -notlike "*/render/*" } |
            Select-Object -ExpandProperty FullName
    )

    $nonRenderViewerInstanceCount = 0
    foreach ($path in $sourceFiles) {
        $nonRenderViewerInstanceCount += Get-MatchCountWithoutComments -Path $path -Pattern "VtkViewer::Instance\s*\("
    }

    $renderGatewayHeaderExists = Test-Path $renderGatewayHeaderPath
    $vtkRenderGatewayHeaderExists = Test-Path $vtkRenderGatewayHeaderPath
    $vtkRenderGatewaySourceExists = Test-Path $vtkRenderGatewaySourcePath

    $renderGatewayHeaderRaw = if ($renderGatewayHeaderExists) {
        Get-Content -Raw -Encoding utf8 $renderGatewayHeaderPath
    } else {
        ""
    }
    $runtimeSourceRaw = if (Test-Path $runtimeSourcePath) {
        Get-Content -Raw -Encoding utf8 $runtimeSourcePath
    } else {
        ""
    }
    $mainRaw = if (Test-Path $mainPath) {
        Get-Content -Raw -Encoding utf8 $mainPath
    } else {
        ""
    }
    $chargeDensityUiRaw = if (Test-Path $chargeDensityUiPath) {
        Get-Content -Raw -Encoding utf8 $chargeDensityUiPath
    } else {
        ""
    }
    $bugLogRaw = if (Test-Path $bugLogPath) {
        Get-Content -Raw -Encoding utf8 $bugLogPath
    } else {
        ""
    }

    $renderGatewayHasDoxygen = (
        $renderGatewayHeaderRaw.Contains("@brief") -and
        $renderGatewayHeaderRaw.Contains("Render port contract consumed by non-render feature modules")
    )
    $renderGatewayHasInteractionLodContract = (
        $renderGatewayHeaderRaw.Contains("IsInteractionLodActive") -and
        $renderGatewayHeaderRaw.Contains("BeginInteractionLod") -and
        $renderGatewayHeaderRaw.Contains("EndInteractionLod")
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
    $bugLogHasStatusTag = [regex]::IsMatch($bugLogRaw, "\b(Open|Resolved|Deferred)\b")

    $results = @(
        (New-Result "P9.non_render_vtk_viewer_instance_calls" ($nonRenderViewerInstanceCount -eq 0) $nonRenderViewerInstanceCount 0),
        (New-Result "P9.render_gateway_header_exists" $renderGatewayHeaderExists ([int]$renderGatewayHeaderExists) 1),
        (New-Result "P9.vtk_render_gateway_header_exists" $vtkRenderGatewayHeaderExists ([int]$vtkRenderGatewayHeaderExists) 1),
        (New-Result "P9.vtk_render_gateway_source_exists" $vtkRenderGatewaySourceExists ([int]$vtkRenderGatewaySourceExists) 1),
        (New-Result "P9.render_gateway_public_doxygen" $renderGatewayHasDoxygen ([int]$renderGatewayHasDoxygen) 1),
        (New-Result "P9.render_gateway_interaction_lod_contract" $renderGatewayHasInteractionLodContract ([int]$renderGatewayHasInteractionLodContract) 1),
        (New-Result "P9.runtime_prime_excludes_font_registry" (-not $primeIncludesFontRegistry) ([int]$primeIncludesFontRegistry) 0),
        (New-Result "P9.main_font_registry_after_imgui_context" $mainFontRegistryAfterImGuiContext ([int]$mainFontRegistryAfterImGuiContext) 1),
        (New-Result "P9.xsf_first_grid_bootstrap_guard" $xsfBootstrapGuardPresent ([int]$xsfBootstrapGuardPresent) 1),
        (New-Result "P9.bug_log_exists" $bugLogExists ([int]$bugLogExists) 1),
        (New-Result "P9.bug_log_has_bug_id" $bugLogHasBugId ([int]$bugLogHasBugId) 1),
        (New-Result "P9.bug_log_has_status_tag" $bugLogHasStatusTag ([int]$bugLogHasStatusTag) 1)
    )

    Write-Host ("Phase 9 Render Boundary Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host "Phase 9 inventory snapshot:"
    Write-Host (" - non-render VtkViewer::Instance() call count (comments excluded): {0}" -f $nonRenderViewerInstanceCount)
    Write-Host (" - render gateway files present: {0}/{1}/{2}" -f $renderGatewayHeaderExists, $vtkRenderGatewayHeaderExists, $vtkRenderGatewaySourceExists)
    Write-Host (" - render gateway doxygen + interaction LOD API: {0}/{1}" -f $renderGatewayHasDoxygen, $renderGatewayHasInteractionLodContract)
    Write-Host (" - runtime/font safeguards: {0}/{1}" -f (-not $primeIncludesFontRegistry), $mainFontRegistryAfterImGuiContext)
    Write-Host (" - XSF bootstrap guard present: {0}" -f $xsfBootstrapGuardPresent)
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
