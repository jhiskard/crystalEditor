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

function Get-MatchCount {
    param(
        [string]$Path,
        [string]$Pattern
    )

    if (-not (Test-Path $Path)) {
        return 0
    }

    $sum = (Select-String -Path $Path -Pattern $Pattern -AllMatches | ForEach-Object {
        $_.Matches.Count
    } | Measure-Object -Sum).Sum

    if (-not $sum) {
        return 0
    }
    return [int]$sum
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    $runtimeHeaderPath = "webassembly/src/shell/runtime/workbench_runtime.h"
    $runtimeSourcePath = "webassembly/src/shell/runtime/workbench_runtime.cpp"
    $activeBindingPath = "webassembly/src/platform/wasm/workbench_bindings.cpp"
    $legacyBindingPath = "webassembly/src/bind_function.cpp"
    $mainPath = "webassembly/src/main.cpp"
    $appPath = "webassembly/src/app.cpp"
    $rootCMakePath = "CMakeLists.txt"

    $runtimeHeaderExists = Test-Path $runtimeHeaderPath
    $runtimeSourceExists = Test-Path $runtimeSourcePath
    $activeBindingExists = Test-Path $activeBindingPath
    $legacyBindingExists = Test-Path $legacyBindingPath

    $mainInstanceCount = Get-MatchCount -Path $mainPath -Pattern "::Instance\("
    $activeBindingInstanceCount = Get-MatchCount -Path $activeBindingPath -Pattern "::Instance\("
    $legacyBindingInstanceCount = Get-MatchCount -Path $legacyBindingPath -Pattern "::Instance\("
    $appInstanceCount = Get-MatchCount -Path $appPath -Pattern "::Instance\("

    $featurePatterns = @(
        "AtomsTemplate::Instance\(",
        "VtkViewer::Instance\(",
        "FileLoader::Instance\(",
        "MeshManager::Instance\(",
        "ModelTree::Instance\(",
        "MeshDetail::Instance\(",
        "MeshGroupDetail::Instance\(",
        "TestWindow::Instance\("
    )
    $appFeatureLookupCount = 0
    foreach ($pattern in $featurePatterns) {
        $appFeatureLookupCount += Get-MatchCount -Path $appPath -Pattern $pattern
    }

    $singletonMatches = Select-String -Path (Get-ChildItem "webassembly/src" -Recurse -Include *.h,*.cpp | Select-Object -ExpandProperty FullName) -Pattern "DECLARE_SINGLETON"
    $singletonDeclCount = @(
        $singletonMatches | Where-Object {
            $_.Path.Replace('\', '/') -notlike "*/macro/singleton_macro.h"
        }
    ).Count

    $runtimeHeaderRaw = if ($runtimeHeaderExists) {
        Get-Content -Raw -Encoding utf8 $runtimeHeaderPath
    }
    else {
        ""
    }
    $runtimeSourceRaw = if ($runtimeSourceExists) {
        Get-Content -Raw -Encoding utf8 $runtimeSourcePath
    }
    else {
        ""
    }
    $activeBindingRaw = if ($activeBindingExists) {
        Get-Content -Raw -Encoding utf8 $activeBindingPath
    }
    else {
        ""
    }
    $mainRaw = if (Test-Path $mainPath) {
        Get-Content -Raw -Encoding utf8 $mainPath
    }
    else {
        ""
    }
    $rootCMakeRaw = Get-Content -Raw -Encoding utf8 $rootCMakePath

    $runtimeHeaderHasDoxygen = $runtimeHeaderRaw.Contains("@brief")
    $runtimeSourceHasRuntimeAccessor = $runtimeSourceRaw.Contains("GetWorkbenchRuntime()")
    $bindingHasDoxygen = $activeBindingRaw.Contains("@brief")
    $bindingUsesRuntimeFacade = $activeBindingRaw.Contains("GetWorkbenchRuntime()")

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

    $cmakeUsesNewBinding = $rootCMakeRaw.Contains("webassembly/src/platform/wasm/workbench_bindings.cpp")
    $cmakeDropsLegacyBinding = -not $rootCMakeRaw.Contains("webassembly/src/bind_function.cpp")

    $results = @(
        (New-Result "P7.runtime_header_exists" $runtimeHeaderExists ([int]$runtimeHeaderExists) 1),
        (New-Result "P7.runtime_source_exists" $runtimeSourceExists ([int]$runtimeSourceExists) 1),
        (New-Result "P7.active_binding_exists" $activeBindingExists ([int]$activeBindingExists) 1),
        (New-Result "P7.main_instance_calls" ($mainInstanceCount -eq 0) $mainInstanceCount 0),
        (New-Result "P7.active_binding_instance_calls" ($activeBindingInstanceCount -eq 0) $activeBindingInstanceCount 0),
        (New-Result "P7.legacy_binding_instance_calls" ($legacyBindingInstanceCount -eq 0) $legacyBindingInstanceCount 0),
        (New-Result "P7.app_instance_calls_total" ($appInstanceCount -le 2) $appInstanceCount "<= 2"),
        (New-Result "P7.app_feature_singleton_lookups" ($appFeatureLookupCount -eq 0) $appFeatureLookupCount 0),
        (New-Result "P7.singleton_declaration_budget" ($singletonDeclCount -le 11) $singletonDeclCount "<= 11"),
        (New-Result "P7.runtime_header_doxygen" $runtimeHeaderHasDoxygen ([int]$runtimeHeaderHasDoxygen) 1),
        (New-Result "P7.runtime_accessor_present" $runtimeSourceHasRuntimeAccessor ([int]$runtimeSourceHasRuntimeAccessor) 1),
        (New-Result "P7.binding_doxygen" $bindingHasDoxygen ([int]$bindingHasDoxygen) 1),
        (New-Result "P7.binding_runtime_facade_usage" $bindingUsesRuntimeFacade ([int]$bindingUsesRuntimeFacade) 1),
        (New-Result "P7.runtime_prime_excludes_font_registry" (-not $primeIncludesFontRegistry) ([int]$primeIncludesFontRegistry) 0),
        (New-Result "P7.main_font_registry_after_imgui_context" $mainFontRegistryAfterImGuiContext ([int]$mainFontRegistryAfterImGuiContext) 1),
        (New-Result "P7.cmake_new_binding_entrypoint" $cmakeUsesNewBinding ([int]$cmakeUsesNewBinding) 1),
        (New-Result "P7.cmake_legacy_binding_removed" $cmakeDropsLegacyBinding ([int]$cmakeDropsLegacyBinding) 1)
    )

    Write-Host ("Phase 7 Runtime Composition Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host "Phase 7 inventory snapshot:"
    Write-Host (" - app ::Instance() count: {0}" -f $appInstanceCount)
    Write-Host (" - app feature singleton lookup count: {0}" -f $appFeatureLookupCount)
    Write-Host (" - singleton declaration count (excluding macro definition): {0}" -f $singletonDeclCount)
    Write-Host (" - PrimeLegacySingletons() contains FontRegistry(): {0}" -f $primeIncludesFontRegistry)
    Write-Host (" - main.cpp FontRegistry() after ImGui::CreateContext(): {0}" -f $mainFontRegistryAfterImGuiContext)
    Write-Host (" - active binding path: {0}" -f $activeBindingPath)
    Write-Host (" - legacy binding path present: {0}" -f $legacyBindingExists)

    $violations = @($results | Where-Object { -not $_.Pass })
    if ($violations.Count -gt 0 -and $FailOnViolation) {
        exit 1
    }

    exit 0
}
finally {
    Pop-Location
}
