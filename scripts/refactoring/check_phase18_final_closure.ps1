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
    $gateScripts = @(
        "scripts/refactoring/check_phase17_target_graph_final.ps1",
        "scripts/refactoring/check_phase17_root_allowlist.ps1",
        "scripts/refactoring/check_phase17_r6_atoms_legacy_zero.ps1",
        "scripts/refactoring/check_phase18_ui_regression_evidence.ps1",
        "scripts/refactoring/check_phase18_facade_include_zero.ps1",
        "scripts/refactoring/check_phase18_atoms_instance_zero.ps1",
        "scripts/refactoring/check_phase18_legacy_singleton_zero.ps1",
        "scripts/refactoring/check_phase18_unused_asset_cleanup.ps1",
        "scripts/refactoring/check_phase18_doc_contract_full.ps1"
    )

    $results = @()
    foreach ($gate in $gateScripts) {
        if (-not (Test-Path $gate)) {
            $results += (New-Result ("P18W7.final_closure.{0}" -f [IO.Path]::GetFileNameWithoutExtension($gate)) $false "missing" "script exists + pass")
            continue
        }

        & powershell -NoProfile -ExecutionPolicy Bypass -File $gate
        $code = $LASTEXITCODE
        $results += (New-Result ("P18W7.final_closure.{0}" -f [IO.Path]::GetFileNameWithoutExtension($gate)) ($code -eq 0) $code 0)
    }

    Write-Host ("Phase 18-W7 Final Closure Gate Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
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
