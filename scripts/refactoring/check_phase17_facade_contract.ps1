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

function Get-LocalIncludes {
    param(
        [string]$Path
    )

    if (-not (Test-Path $Path)) {
        return @()
    }

    $lines = Get-Content -Encoding utf8 $Path
    $includes = @()
    foreach ($line in $lines) {
        $match = [regex]::Match($line, '^\s*#include\s+"([^"]+)"')
        if ($match.Success) {
            $includes += $match.Groups[1].Value
        }
    }

    return @($includes)
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    $mainCppPath = "webassembly/src/main.cpp"
    $appCppPath = "webassembly/src/app.cpp"
    $appHPath = "webassembly/src/app.h"

    $mainIncludes = Get-LocalIncludes -Path $mainCppPath
    $appIncludes = Get-LocalIncludes -Path $appCppPath
    $appHIncludes = Get-LocalIncludes -Path $appHPath

    $mainRequiredIncludeSet = @(
        "app.h",
        "config/log_config.h",
        "shell/runtime/workbench_runtime.h"
    )
    $mainUnknownIncludes = @(
        $mainIncludes | Where-Object { $_ -notin $mainRequiredIncludeSet }
    )
    $mainMissingIncludes = @(
        $mainRequiredIncludeSet | Where-Object { $_ -notin $mainIncludes }
    )

    $forbiddenRootHeaderPattern =
        '^(font_manager\.h|vtk_viewer\.h|model_tree\.h|mesh(_detail|_group(_detail)?|_manager)?\.h|mesh\.h|toolbar\.h|file_loader\.h|unv_reader\.h|custom_ui\.h|image\.h|texture\.h|test_window\.h|lcrs_tree\.h|atoms/atoms_template\.h)$'
    $appHForbiddenIncludes = @(
        $appHIncludes | Where-Object { $_ -match $forbiddenRootHeaderPattern }
    )

    # R1 snapshot budget: app.cpp의 루트 legacy include는 현재치(8) 이내로만 허용하고 새 유입은 금지.
    $appCppLegacyIncludeAllowlist = @(
        "font_manager.h",
        "vtk_viewer.h",
        "model_tree.h",
        "mesh_detail.h",
        "mesh_manager.h",
        "mesh_group_detail.h",
        "mesh.h",
        "atoms/atoms_template.h"
    )
    $appCppLegacyIncludeBudget = $appCppLegacyIncludeAllowlist.Count

    $appCppLegacyIncludes = @(
        $appIncludes | Where-Object { $_ -in $appCppLegacyIncludeAllowlist }
    )
    $appCppUnexpectedRootIncludes = @(
        $appIncludes |
            Where-Object {
                (
                    ($_ -notmatch "/") -and
                    ($_ -ne "app.h") -and
                    ($_ -notin $appCppLegacyIncludeAllowlist)
                ) -or
                (
                    ($_ -match "^atoms/") -and
                    ($_ -notin $appCppLegacyIncludeAllowlist)
                )
            }
    )

    $mainText = Get-TextWithoutComments -Path $mainCppPath
    $appText = Get-TextWithoutComments -Path $appCppPath
    $appHText = Get-TextWithoutComments -Path $appHPath

    $singletonPattern = '\b(App|Toolbar|FileLoader|MeshManager|AtomsTemplate|VtkViewer|ModelTree|MeshDetail|MeshGroupDetail|FontManager)::Instance\s*\('
    $singletonInstanceCount =
        [regex]::Matches($mainText, $singletonPattern).Count +
        [regex]::Matches($appText, $singletonPattern).Count +
        [regex]::Matches($appHText, $singletonPattern).Count

    $mainForbiddenFeatureAccessPattern = 'runtime\.(StructureFeature|MeasurementFeature|DensityFeature|MeshRepository|FileLoaderService|AtomsTemplateFacade|ModelTreePanel|MeshDetailPanel|MeshGroupDetailPanel|Viewer)\s*\('
    $mainForbiddenFeatureAccessCount = [regex]::Matches($mainText, $mainForbiddenFeatureAccessPattern).Count

    $mainAppHForbiddenTypePattern = '\b(AtomsTemplate|MeshManager|VtkViewer|FileLoader|FontManager)\b'
    $mainAppHForbiddenTypeCount =
        [regex]::Matches($mainText, $mainAppHForbiddenTypePattern).Count +
        [regex]::Matches($appHText, $mainAppHForbiddenTypePattern).Count

    $results = @(
        (New-Result "P17R1.main_local_include_allowlist_exact" ($mainUnknownIncludes.Count -eq 0 -and $mainMissingIncludes.Count -eq 0) ($mainUnknownIncludes.Count + $mainMissingIncludes.Count) 0),
        (New-Result "P17R1.app_h_forbidden_root_include_zero" ($appHForbiddenIncludes.Count -eq 0) $appHForbiddenIncludes.Count 0),
        (New-Result "P17R1.app_cpp_legacy_include_not_increased" ($appCppLegacyIncludes.Count -le $appCppLegacyIncludeBudget) $appCppLegacyIncludes.Count ("<= {0}" -f $appCppLegacyIncludeBudget)),
        (New-Result "P17R1.app_cpp_unexpected_root_include_zero" ($appCppUnexpectedRootIncludes.Count -eq 0) $appCppUnexpectedRootIncludes.Count 0),
        (New-Result "P17R1.main_app_forbidden_singleton_instance_zero" ($singletonInstanceCount -eq 0) $singletonInstanceCount 0),
        (New-Result "P17R1.main_forbidden_feature_access_zero" ($mainForbiddenFeatureAccessCount -eq 0) $mainForbiddenFeatureAccessCount 0),
        (New-Result "P17R1.main_apph_forbidden_type_token_zero" ($mainAppHForbiddenTypeCount -eq 0) $mainAppHForbiddenTypeCount 0)
    )

    Write-Host ("Phase 17-R1 Facade Contract Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host "Phase 17-R1 facade snapshot:"
    Write-Host (" - app.cpp legacy includes: {0}" -f $appCppLegacyIncludes.Count)

    if ($mainUnknownIncludes.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R1.main_local_include_allowlist_exact] Unknown main includes:"
        foreach ($entry in $mainUnknownIncludes) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($mainMissingIncludes.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R1.main_local_include_allowlist_exact] Missing main includes:"
        foreach ($entry in $mainMissingIncludes) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($appHForbiddenIncludes.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R1.app_h_forbidden_root_include_zero] Violations:"
        foreach ($entry in ($appHForbiddenIncludes | Sort-Object -Unique)) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($appCppUnexpectedRootIncludes.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R1.app_cpp_unexpected_root_include_zero] Violations:"
        foreach ($entry in ($appCppUnexpectedRootIncludes | Sort-Object -Unique)) {
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
