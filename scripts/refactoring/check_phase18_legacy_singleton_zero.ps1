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

function Invoke-RgLines {
    param(
        [string]$Pattern,
        [string[]]$Paths,
        [string[]]$Globs
    )

    $args = @("--line-number", $Pattern) + $Paths
    foreach ($glob in $Globs) {
        $args += @("-g", $glob)
    }

    $lines = @(& rg @args 2>$null)
    if ($LASTEXITCODE -gt 1) {
        throw "rg failed for pattern: $Pattern"
    }
    if ($LASTEXITCODE -eq 1) {
        return @()
    }
    return $lines
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    $singletonDeclMatches = Invoke-RgLines -Pattern 'DECLARE_SINGLETON\(AtomsTemplate\)' -Paths @("webassembly/src", "webassembly/tests") -Globs @("*.cpp", "*.h")
    $singletonMacroIncludeMatches = Invoke-RgLines -Pattern '#include\s+\".*singleton_macro.h\"' -Paths @("webassembly/src/workspace/legacy/atoms_template_facade.h") -Globs @("*.h")

    $legacyRuntimeHeaderPath = "webassembly/src/workspace/legacy/legacy_atoms_runtime.h"
    $legacyRuntimeSourcePath = "webassembly/src/workspace/legacy/legacy_atoms_runtime.cpp"
    $workspaceModulePath = "webassembly/cmake/modules/wb_workspace.cmake"
    $workspaceModuleText = if (Test-Path $workspaceModulePath) { Get-Content -Raw -Encoding utf8 $workspaceModulePath } else { "" }
    $workspaceModuleBindsLegacyRuntime = [regex]::IsMatch($workspaceModuleText, "legacy_atoms_runtime\.cpp")

    $results = @(
        (New-Result "P18W7.legacy_singleton_decl_count_zero" ($singletonDeclMatches.Count -eq 0) $singletonDeclMatches.Count 0),
        (New-Result "P18W7.legacy_singleton_macro_include_removed" ($singletonMacroIncludeMatches.Count -eq 0) $singletonMacroIncludeMatches.Count 0),
        (New-Result "P18W7.legacy_runtime_header_present" (Test-Path $legacyRuntimeHeaderPath) ([int](Test-Path $legacyRuntimeHeaderPath)) 1),
        (New-Result "P18W7.legacy_runtime_source_present" (Test-Path $legacyRuntimeSourcePath) ([int](Test-Path $legacyRuntimeSourcePath)) 1),
        (New-Result "P18W7.workspace_module_binds_legacy_runtime_source" $workspaceModuleBindsLegacyRuntime ([int]$workspaceModuleBindsLegacyRuntime) 1)
    )

    Write-Host ("Phase 18-W7 Legacy Singleton Zero Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    if ($singletonDeclMatches.Count -gt 0) {
        Write-Host ""
        Write-Host "[P18W7.legacy_singleton_decl_count_zero] remaining declarations:"
        foreach ($line in $singletonDeclMatches) {
            Write-Host (" - {0}" -f $line)
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
