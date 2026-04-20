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
    $inventoryDoc = "docs/refactoring/phase18/logs/comment_policy_inventory_phase18_latest.md"
    $violationDoc = "docs/refactoring/phase18/logs/comment_policy_violation_list_phase18_latest.md"
    $fixReportDoc = "docs/refactoring/phase18/logs/comment_policy_fix_report_phase18_latest.md"

    $violationText = Get-TextOrEmpty -Path $violationDoc
    $fixReportText = Get-TextOrEmpty -Path $fixReportDoc
    $pageTsxText = Get-TextOrEmpty -Path "app/workbench/page.tsx"

    $violationZero = (
        [regex]::IsMatch($violationText, "W3~W6") -and
        [regex]::IsMatch($violationText, "\b0\b")
    )
    $fixReportDoxygenZero = [regex]::IsMatch($fixReportText, "Doxygen[^\r\n]*\|\s*0")
    $fixReportTemporaryZero = [regex]::IsMatch($fixReportText, "Temporary[^\r\n]*\|\s*0")
    $fixReportMismatchZero = [regex]::IsMatch($fixReportText, "W4~W6[^\r\n]*\|\s*0")
    $pageTestSeamDocumented = (
        [regex]::IsMatch($pageTsxText, "__VTK_WORKBENCH_TEST__") -and
        [regex]::IsMatch($pageTsxText, "test seam", [System.Text.RegularExpressions.RegexOptions]::IgnoreCase)
    )

    $temporaryContractHeaders = @(
        "webassembly/src/io/application/import_runtime_port.h",
        "webassembly/src/io/infrastructure/legacy/legacy_import_runtime_port.h",
        "webassembly/src/workspace/legacy/legacy_atoms_runtime.h"
    )

    $temporaryHeaderResults = @()
    foreach ($header in $temporaryContractHeaders) {
        $text = Get-TextOrEmpty -Path $header
        $exists = Test-Path $header
        $hasBrief = [regex]::IsMatch($text, "@brief")
        $hasTemporaryNote = [regex]::IsMatch($text, "Temporary compatibility path")
        $hasRemovePhase = [regex]::IsMatch($text, "remove in Phase18")
        $hasTrackId = [regex]::IsMatch($text, "Track:")
        $pass = ($exists -and $hasBrief -and $hasTemporaryNote -and $hasRemovePhase -and $hasTrackId)
        $temporaryHeaderResults += (New-Result ("P18W7.doc_contract_header_{0}" -f ([IO.Path]::GetFileName($header))) $pass ("exists={0}, brief={1}, temp={2}, phase={3}, track={4}" -f $exists, $hasBrief, $hasTemporaryNote, $hasRemovePhase, $hasTrackId) "all true")
    }

    $results = @(
        (New-Result "P18W7.doc_contract_inventory_doc_present" (Test-Path $inventoryDoc) ([int](Test-Path $inventoryDoc)) 1),
        (New-Result "P18W7.doc_contract_violation_doc_present" (Test-Path $violationDoc) ([int](Test-Path $violationDoc)) 1),
        (New-Result "P18W7.doc_contract_fix_report_present" (Test-Path $fixReportDoc) ([int](Test-Path $fixReportDoc)) 1),
        (New-Result "P18W7.doc_contract_violation_zero_recorded" $violationZero ([int]$violationZero) 1),
        (New-Result "P18W7.doc_contract_fix_report_doxygen_zero" $fixReportDoxygenZero ([int]$fixReportDoxygenZero) 1),
        (New-Result "P18W7.doc_contract_fix_report_temporary_zero" $fixReportTemporaryZero ([int]$fixReportTemporaryZero) 1),
        (New-Result "P18W7.doc_contract_fix_report_mismatch_zero" $fixReportMismatchZero ([int]$fixReportMismatchZero) 1),
        (New-Result "P18W7.doc_contract_page_test_seam_documented" $pageTestSeamDocumented ([int]$pageTestSeamDocumented) 1)
    ) + $temporaryHeaderResults

    Write-Host ("Phase 18-W7 Doc Contract Full Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
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
