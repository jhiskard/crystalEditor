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

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    $expectedModules = @(
        "wb_common",
        "wb_platform",
        "wb_shell",
        "wb_workspace",
        "wb_render",
        "wb_mesh",
        "wb_structure",
        "wb_measurement",
        "wb_density",
        "wb_io"
    )

    $forbiddenLegacyModules = @(
        "wb_core",
        "wb_ui",
        "wb_atoms"
    )

    $moduleDir = "webassembly/cmake/modules"
    $moduleFiles = @(
        Get-ChildItem -Path $moduleDir -File -Filter "wb_*.cmake" |
            Select-Object -ExpandProperty BaseName |
            Sort-Object -Unique
    )

    $missingModules = @($expectedModules | Where-Object { $_ -notin $moduleFiles })
    $unexpectedModules = @($moduleFiles | Where-Object { $_ -notin $expectedModules })
    $legacyModuleFilesPresent = @(
        $forbiddenLegacyModules |
            Where-Object { Test-Path (Join-Path $moduleDir ("{0}.cmake" -f $_)) }
    )

    $cmakePath = "CMakeLists.txt"
    $cmakeText = if (Test-Path $cmakePath) { Get-Content -Raw -Encoding utf8 $cmakePath } else { "" }

    $missingModuleTokensInCmake = @(
        $expectedModules |
            Where-Object { -not [regex]::IsMatch($cmakeText, "\b$([regex]::Escape($_))\b") }
    )
    $legacyModuleTokensInCmake = @(
        $forbiddenLegacyModules |
            Where-Object { [regex]::IsMatch($cmakeText, "\b$([regex]::Escape($_))\b") }
    )

    $moduleSourceBindingViolations = @()
    foreach ($module in $expectedModules) {
        $moduleFilePath = Join-Path $moduleDir ("{0}.cmake" -f $module)
        if (-not (Test-Path $moduleFilePath)) {
            continue
        }
        $moduleText = Get-Content -Raw -Encoding utf8 $moduleFilePath
        if (-not [regex]::IsMatch($moduleText, "target_sources\s*\(\s*$([regex]::Escape($module))\s+PRIVATE", [System.Text.RegularExpressions.RegexOptions]::IgnoreCase)) {
            $moduleSourceBindingViolations += $module
        }
    }

    $results = @(
        (New-Result "P17R6.target_graph_module_file_set_exact" ($missingModules.Count -eq 0 -and $unexpectedModules.Count -eq 0) ($missingModules.Count + $unexpectedModules.Count) 0),
        (New-Result "P17R6.target_graph_legacy_module_file_zero" ($legacyModuleFilesPresent.Count -eq 0) $legacyModuleFilesPresent.Count 0),
        (New-Result "P17R6.target_graph_expected_module_tokens_in_cmake" ($missingModuleTokensInCmake.Count -eq 0) $missingModuleTokensInCmake.Count 0),
        (New-Result "P17R6.target_graph_legacy_module_tokens_in_cmake_zero" ($legacyModuleTokensInCmake.Count -eq 0) $legacyModuleTokensInCmake.Count 0),
        (New-Result "P17R6.target_graph_module_source_binding_valid" ($moduleSourceBindingViolations.Count -eq 0) $moduleSourceBindingViolations.Count 0)
    )

    Write-Host ("Phase 17-R6 Final Target Graph Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    if ($missingModules.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R6.target_graph_module_file_set_exact] Missing modules:"
        foreach ($entry in $missingModules) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($unexpectedModules.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R6.target_graph_module_file_set_exact] Unexpected modules:"
        foreach ($entry in $unexpectedModules) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($legacyModuleFilesPresent.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R6.target_graph_legacy_module_file_zero] Legacy module files still present:"
        foreach ($entry in $legacyModuleFilesPresent) {
            Write-Host (" - {0}.cmake" -f $entry)
        }
    }

    if ($missingModuleTokensInCmake.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R6.target_graph_expected_module_tokens_in_cmake] Missing module tokens in CMakeLists.txt:"
        foreach ($entry in $missingModuleTokensInCmake) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($legacyModuleTokensInCmake.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R6.target_graph_legacy_module_tokens_in_cmake_zero] Legacy module tokens still referenced:"
        foreach ($entry in $legacyModuleTokensInCmake) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($moduleSourceBindingViolations.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R6.target_graph_module_source_binding_valid] Invalid module source bindings:"
        foreach ($entry in $moduleSourceBindingViolations) {
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
