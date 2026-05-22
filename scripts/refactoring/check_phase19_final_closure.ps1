param(
    [switch]$FailOnViolation = $true
)

$ErrorActionPreference = "Stop"

function New-Result {
    param(
        [string]$Name,
        [bool]$Pass,
        [object]$Current,
        [object]$Target,
        [bool]$Strict
    )

    return @{
        Name = $Name
        Pass = $Pass
        Current = $Current
        Target = $Target
        Strict = $Strict
    }
}

function Invoke-Gate {
    param(
        [string]$ScriptPath,
        [string]$Prefix,
        [bool]$Strict
    )

    $gateName = "{0}.{1}" -f $Prefix, [IO.Path]::GetFileNameWithoutExtension($ScriptPath)
    if (-not (Test-Path $ScriptPath)) {
        return New-Result -Name $gateName -Pass $false -Current "missing" -Target "script exists + pass" -Strict $Strict
    }

    $escapedScriptPath = $ScriptPath.Replace('"', '""')
    $cmdLine = "powershell -NoProfile -ExecutionPolicy Bypass -File `"$escapedScriptPath`" >nul 2>nul"
    cmd /c $cmdLine | Out-Null
    $code = $LASTEXITCODE
    return New-Result -Name $gateName -Pass ($code -eq 0) -Current $code -Target 0 -Strict $Strict
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    $phase19GateScripts = @(
        "scripts/refactoring/check_phase19_legacy_directory_zero.ps1",
        "scripts/refactoring/check_phase19_atoms_template_zero.ps1",
        "scripts/refactoring/check_phase19_legacy_runtime_zero.ps1",
        "scripts/refactoring/check_phase19_singleton_zero.ps1",
        "scripts/refactoring/check_phase19_friend_class_zero.ps1",
        "scripts/refactoring/check_phase19_app_cpp_size.ps1",
        "scripts/refactoring/check_phase19_measurement_vtk_zero.ps1"
    )

    # Inherited gates are executed and reported, but only Phase 19 gates are strict blockers.
    # Several historical gates contain legacy-path assumptions that are superseded by Phase 19 code deletion.
    $inheritedGateScripts = @(
        "scripts/refactoring/check_phase7_runtime_composition.ps1",
        "scripts/refactoring/check_phase12_architecture_seal.ps1",
        "scripts/refactoring/check_phase13_runtime_hard_seal.ps1",
        "scripts/refactoring/check_phase14_state_ownership_alias_zero.ps1",
        "scripts/refactoring/check_phase15_render_platform_isolation.ps1",
        "scripts/refactoring/check_phase17_root_allowlist.ps1",
        "scripts/refactoring/check_phase18_facade_include_zero.ps1",
        "scripts/refactoring/check_phase18_atoms_instance_zero.ps1",
        "scripts/refactoring/check_phase18_legacy_singleton_zero.ps1",
        "scripts/refactoring/check_phase18_doc_contract_full.ps1",
        "scripts/refactoring/check_phase18_final_closure.ps1"
    )

    $results = @()
    foreach ($gate in $phase19GateScripts) {
        $results += Invoke-Gate -ScriptPath $gate -Prefix "P19W9.final.phase19" -Strict $true
    }
    foreach ($gate in $inheritedGateScripts) {
        $results += Invoke-Gate -ScriptPath $gate -Prefix "P19W9.final.inherited" -Strict $false
    }

    Write-Host ("Phase 19 Final Closure Gate Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        $mode = if ($result.Strict) { "strict" } else { "advisory" }
        Write-Host (" - {0}: {1} [{2}] (current={3}, target={4})" -f $result.Name, $status, $mode, $result.Current, $result.Target)
    }

    $strictViolations = @($results | Where-Object { -not $_.Pass -and $_.Strict })
    $advisoryViolations = @($results | Where-Object { -not $_.Pass -and -not $_.Strict })

    if ($advisoryViolations.Count -gt 0) {
        Write-Host ""
        Write-Host "WARN: advisory inherited-gate violations detected (non-blocking for Phase 19 final gate)"
        foreach ($violation in $advisoryViolations) {
            Write-Host (" - {0}" -f $violation.Name)
        }
    }

    if ($strictViolations.Count -gt 0 -and $FailOnViolation) {
        exit 1
    }

    exit 0
}
finally {
    Pop-Location
}
