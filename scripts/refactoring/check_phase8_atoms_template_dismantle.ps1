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
        [string[]]$Path,
        [string]$Pattern
    )

    $existingPaths = @($Path | Where-Object { Test-Path $_ })
    if ($existingPaths.Count -eq 0) {
        return 0
    }

    $sum = (Select-String -Path $existingPaths -Pattern $Pattern -AllMatches | ForEach-Object {
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
    $structureDir = "webassembly/src/structure"
    $measurementDir = "webassembly/src/measurement"
    $densityDir = "webassembly/src/density"

    $structureRepoHeaderPath = "webassembly/src/structure/domain/structure_repository.h"
    $structureRepoSourcePath = "webassembly/src/structure/domain/structure_repository.cpp"
    $structureServiceHeaderPath = "webassembly/src/structure/application/structure_service.h"
    $structureServiceSourcePath = "webassembly/src/structure/application/structure_service.cpp"
    $measurementServiceHeaderPath = "webassembly/src/measurement/application/measurement_service.h"
    $measurementServiceSourcePath = "webassembly/src/measurement/application/measurement_service.cpp"
    $densityServiceHeaderPath = "webassembly/src/density/application/density_service.h"
    $densityServiceSourcePath = "webassembly/src/density/application/density_service.cpp"

    $atomsHeaderPath = "webassembly/src/atoms/atoms_template.h"
    $atomsSourcePath = "webassembly/src/atoms/atoms_template.cpp"
    $appPath = "webassembly/src/app.cpp"
    $runtimeHeaderPath = "webassembly/src/shell/runtime/workbench_runtime.h"
    $runtimeSourcePath = "webassembly/src/shell/runtime/workbench_runtime.cpp"
    $mainPath = "webassembly/src/main.cpp"
    $modelTreeStructurePath = "webassembly/src/mesh/presentation/model_tree_structure_section.cpp"
    $chargeDensityUiPath = "webassembly/src/atoms/ui/charge_density_ui.cpp"
    $cmakeAtomsPath = "webassembly/cmake/modules/wb_atoms.cmake"

    $structureDirExists = Test-Path $structureDir
    $measurementDirExists = Test-Path $measurementDir
    $densityDirExists = Test-Path $densityDir

    $structureRepoHeaderExists = Test-Path $structureRepoHeaderPath
    $structureRepoSourceExists = Test-Path $structureRepoSourcePath
    $structureServiceHeaderExists = Test-Path $structureServiceHeaderPath
    $structureServiceSourceExists = Test-Path $structureServiceSourcePath
    $measurementServiceHeaderExists = Test-Path $measurementServiceHeaderPath
    $measurementServiceSourceExists = Test-Path $measurementServiceSourcePath
    $densityServiceHeaderExists = Test-Path $densityServiceHeaderPath
    $densityServiceSourceExists = Test-Path $densityServiceSourcePath

    $friendClassCount = Get-MatchCount -Path $atomsHeaderPath -Pattern "^\s*friend\s+class\s+"
    $atomsTotalInstanceCount = Get-MatchCount -Path $atomsSourcePath -Pattern "::Instance\("
    $atomsVtkViewerInstanceCount = Get-MatchCount -Path $atomsSourcePath -Pattern "VtkViewer::Instance\("

    $createdAtomsCount = Get-MatchCount -Path (Get-ChildItem "webassembly/src/atoms" -Recurse -Include *.h,*.cpp | Select-Object -ExpandProperty FullName) -Pattern "\bcreatedAtoms\b"
    $createdBondsCount = Get-MatchCount -Path (Get-ChildItem "webassembly/src/atoms" -Recurse -Include *.h,*.cpp | Select-Object -ExpandProperty FullName) -Pattern "\bcreatedBonds\b"
    $cellInfoCount = Get-MatchCount -Path (Get-ChildItem "webassembly/src/atoms" -Recurse -Include *.h,*.cpp | Select-Object -ExpandProperty FullName) -Pattern "\bcellInfo\b"

    $runtimeHeaderRaw = if (Test-Path $runtimeHeaderPath) { Get-Content -Raw -Encoding utf8 $runtimeHeaderPath } else { "" }
    $runtimeSourceRaw = if (Test-Path $runtimeSourcePath) { Get-Content -Raw -Encoding utf8 $runtimeSourcePath } else { "" }
    $appRaw = if (Test-Path $appPath) { Get-Content -Raw -Encoding utf8 $appPath } else { "" }
    $mainRaw = if (Test-Path $mainPath) { Get-Content -Raw -Encoding utf8 $mainPath } else { "" }
    $modelTreeRaw = if (Test-Path $modelTreeStructurePath) { Get-Content -Raw -Encoding utf8 $modelTreeStructurePath } else { "" }
    $chargeDensityUiRaw = if (Test-Path $chargeDensityUiPath) { Get-Content -Raw -Encoding utf8 $chargeDensityUiPath } else { "" }
    $cmakeAtomsRaw = if (Test-Path $cmakeAtomsPath) { Get-Content -Raw -Encoding utf8 $cmakeAtomsPath } else { "" }

    $runtimeHasStructureFeature = $runtimeSourceRaw.Contains("StructureFeature()")
    $runtimeHasDensityFeature = $runtimeSourceRaw.Contains("DensityFeature()")
    $runtimeHeaderHasModuleApis = (
        $runtimeHeaderRaw.Contains("StructureFeature()") -and
        $runtimeHeaderRaw.Contains("MeasurementFeature()") -and
        $runtimeHeaderRaw.Contains("DensityFeature()")
    )
    $appUsesMeasurementFeature = $appRaw.Contains("MeasurementFeature()")
    $modelTreeUsesModuleServices = (
        $modelTreeRaw.Contains("structureService") -and
        $modelTreeRaw.Contains("densityService")
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

    $cmakeRegistersStructure = $cmakeAtomsRaw.Contains("webassembly/src/structure/domain/structure_repository.cpp")
    $cmakeRegistersMeasurement = $cmakeAtomsRaw.Contains("webassembly/src/measurement/application/measurement_service.cpp")
    $cmakeRegistersDensity = $cmakeAtomsRaw.Contains("webassembly/src/density/application/density_service.cpp")

    $results = @(
        (New-Result "P8.structure_dir_exists" $structureDirExists ([int]$structureDirExists) 1),
        (New-Result "P8.measurement_dir_exists" $measurementDirExists ([int]$measurementDirExists) 1),
        (New-Result "P8.density_dir_exists" $densityDirExists ([int]$densityDirExists) 1),

        (New-Result "P8.structure_repository_header_exists" $structureRepoHeaderExists ([int]$structureRepoHeaderExists) 1),
        (New-Result "P8.structure_repository_source_exists" $structureRepoSourceExists ([int]$structureRepoSourceExists) 1),
        (New-Result "P8.structure_service_header_exists" $structureServiceHeaderExists ([int]$structureServiceHeaderExists) 1),
        (New-Result "P8.structure_service_source_exists" $structureServiceSourceExists ([int]$structureServiceSourceExists) 1),
        (New-Result "P8.measurement_service_header_exists" $measurementServiceHeaderExists ([int]$measurementServiceHeaderExists) 1),
        (New-Result "P8.measurement_service_source_exists" $measurementServiceSourceExists ([int]$measurementServiceSourceExists) 1),
        (New-Result "P8.density_service_header_exists" $densityServiceHeaderExists ([int]$densityServiceHeaderExists) 1),
        (New-Result "P8.density_service_source_exists" $densityServiceSourceExists ([int]$densityServiceSourceExists) 1),

        (New-Result "P8.atoms_template_friend_class_zero" ($friendClassCount -eq 0) $friendClassCount 0),
        (New-Result "P8.atoms_template_instance_total_budget" ($atomsTotalInstanceCount -le 80) $atomsTotalInstanceCount "<= 80"),
        (New-Result "P8.atoms_template_vtk_viewer_instance_budget" ($atomsVtkViewerInstanceCount -le 50) $atomsVtkViewerInstanceCount "<= 50"),

        (New-Result "P8.runtime_header_module_api" $runtimeHeaderHasModuleApis ([int]$runtimeHeaderHasModuleApis) 1),
        (New-Result "P8.runtime_structure_feature_usage" $runtimeHasStructureFeature ([int]$runtimeHasStructureFeature) 1),
        (New-Result "P8.runtime_density_feature_usage" $runtimeHasDensityFeature ([int]$runtimeHasDensityFeature) 1),
        (New-Result "P8.app_measurement_feature_usage" $appUsesMeasurementFeature ([int]$appUsesMeasurementFeature) 1),
        (New-Result "P8.model_tree_module_service_usage" $modelTreeUsesModuleServices ([int]$modelTreeUsesModuleServices) 1),

        (New-Result "P8.runtime_prime_excludes_font_registry" (-not $primeIncludesFontRegistry) ([int]$primeIncludesFontRegistry) 0),
        (New-Result "P8.main_font_registry_after_imgui_context" $mainFontRegistryAfterImGuiContext ([int]$mainFontRegistryAfterImGuiContext) 1),
        (New-Result "P8.xsf_first_grid_bootstrap_guard" $xsfBootstrapGuardPresent ([int]$xsfBootstrapGuardPresent) 1),

        (New-Result "P8.cmake_registers_structure_module_sources" $cmakeRegistersStructure ([int]$cmakeRegistersStructure) 1),
        (New-Result "P8.cmake_registers_measurement_module_sources" $cmakeRegistersMeasurement ([int]$cmakeRegistersMeasurement) 1),
        (New-Result "P8.cmake_registers_density_module_sources" $cmakeRegistersDensity ([int]$cmakeRegistersDensity) 1),

        (New-Result "P8.legacy_createdAtoms_budget" ($createdAtomsCount -le 117) $createdAtomsCount "<= 117"),
        (New-Result "P8.legacy_createdBonds_budget" ($createdBondsCount -le 47) $createdBondsCount "<= 47"),
        (New-Result "P8.legacy_cellInfo_budget" ($cellInfoCount -le 250) $cellInfoCount "<= 250")
    )

    Write-Host ("Phase 8 AtomsTemplate Dismantle Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host "Phase 8 inventory snapshot:"
    Write-Host (" - atoms_template.cpp ::Instance() count: {0}" -f $atomsTotalInstanceCount)
    Write-Host (" - atoms_template.cpp VtkViewer::Instance() count: {0}" -f $atomsVtkViewerInstanceCount)
    Write-Host (" - atoms_template.h friend class count: {0}" -f $friendClassCount)
    Write-Host (" - legacy aliases (createdAtoms/createdBonds/cellInfo): {0}/{1}/{2}" -f $createdAtomsCount, $createdBondsCount, $cellInfoCount)
    Write-Host (" - module dirs present (structure/measurement/density): {0}/{1}/{2}" -f $structureDirExists, $measurementDirExists, $densityDirExists)
    Write-Host (" - runtime Phase 7 safeguards (Prime excludes FontRegistry, main order): {0}/{1}" -f (-not $primeIncludesFontRegistry), $mainFontRegistryAfterImGuiContext)
    Write-Host (" - XSF bootstrap guard present: {0}" -f $xsfBootstrapGuardPresent)

    $violations = @($results | Where-Object { -not $_.Pass })
    if ($violations.Count -gt 0 -and $FailOnViolation) {
        exit 1
    }

    exit 0
}
finally {
    Pop-Location
}
