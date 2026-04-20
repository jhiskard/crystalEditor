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

function Get-TextOrEmpty {
    param(
        [string]$Path
    )

    if (-not (Test-Path $Path)) {
        return ""
    }

    return Get-Content -Raw -Encoding utf8 $Path
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    $layoutSpecPath = "tests/e2e/workbench-layout-panel-regression.spec.ts"
    $smokeSpecPath = "tests/e2e/workbench-smoke.spec.ts"
    $checklistPath = "docs/refactoring/phase18/logs/phase18_ui_regression_checklist_latest.md"
    $manualPath = "docs/refactoring/phase18/logs/manual_ui_regression_phase18_latest.md"
    $smokeLogPath = "docs/refactoring/phase18/logs/smoke_phase18_layout_latest.txt"

    $checklistText = Get-TextOrEmpty -Path $checklistPath
    $manualText = Get-TextOrEmpty -Path $manualPath
    $smokeLogText = Get-TextOrEmpty -Path $smokeLogPath

    $layoutSpecExists = Test-Path $layoutSpecPath
    $smokeSpecExists = Test-Path $smokeSpecPath
    $checklistExists = Test-Path $checklistPath
    $manualExists = Test-Path $manualPath
    $smokeLogExists = Test-Path $smokeLogPath

    $checklistScenarioCoverage = (
        [regex]::IsMatch($checklistText, "UI-01") -and
        [regex]::IsMatch($checklistText, "UI-13") -and
        ([regex]::Matches($checklistText, "PASS").Count -ge 5)
    )

    $manualAutomationEvidence = (
        [regex]::IsMatch($manualText, [regex]::Escape("workbench-layout-panel-regression.spec.ts")) -and
        [regex]::IsMatch($manualText, [regex]::Escape("workbench-smoke.spec.ts")) -and
        [regex]::IsMatch($manualText, "\bPASS\b")
    )

    $smokePassCountRecorded = [regex]::IsMatch($smokeLogText, "\b2 passed\b")
    $smokeBothSpecsRecorded = (
        [regex]::IsMatch($smokeLogText, [regex]::Escape("workbench-smoke.spec.ts")) -and
        [regex]::IsMatch($smokeLogText, [regex]::Escape("workbench-layout-panel-regression.spec.ts"))
    )

    $results = @(
        (New-Result "P18W7.ui.layout_regression_spec_present" $layoutSpecExists ([int]$layoutSpecExists) 1),
        (New-Result "P18W7.ui.smoke_spec_present" $smokeSpecExists ([int]$smokeSpecExists) 1),
        (New-Result "P18W7.ui.checklist_log_present" $checklistExists ([int]$checklistExists) 1),
        (New-Result "P18W7.ui.checklist_scenario_coverage" $checklistScenarioCoverage ([int]$checklistScenarioCoverage) 1),
        (New-Result "P18W7.ui.manual_log_present" $manualExists ([int]$manualExists) 1),
        (New-Result "P18W7.ui.manual_automation_evidence" $manualAutomationEvidence ([int]$manualAutomationEvidence) 1),
        (New-Result "P18W7.ui.smoke_log_present" $smokeLogExists ([int]$smokeLogExists) 1),
        (New-Result "P18W7.ui.smoke_log_pass_count_recorded" $smokePassCountRecorded ([int]$smokePassCountRecorded) 1),
        (New-Result "P18W7.ui.smoke_log_both_specs_recorded" $smokeBothSpecsRecorded ([int]$smokeBothSpecsRecorded) 1)
    )

    Write-Host ("Phase 18-W7 UI Regression Evidence Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
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
