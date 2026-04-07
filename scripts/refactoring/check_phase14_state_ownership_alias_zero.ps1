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

function To-RelativePath {
    param(
        [string]$FullPath,
        [string]$RepoRootPath
    )

    $resolved = (Resolve-Path $FullPath).Path
    if ($resolved.StartsWith($RepoRootPath, [System.StringComparison]::OrdinalIgnoreCase)) {
        return $resolved.Substring($RepoRootPath.Length + 1).Replace('\', '/')
    }
    return $resolved.Replace('\', '/')
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

function Get-MatchRecordsWithoutComments {
    param(
        [string[]]$Paths,
        [string]$Pattern,
        [string]$RepoRootPath
    )

    $records = @()
    foreach ($path in $Paths) {
        if (-not (Test-Path $path)) {
            continue
        }

        $text = Get-TextWithoutComments -Path $path
        if ([string]::IsNullOrEmpty($text)) {
            continue
        }

        $count = [regex]::Matches($text, $Pattern).Count
        if ($count -le 0) {
            continue
        }

        $records += [pscustomobject]@{
            Path = To-RelativePath -FullPath $path -RepoRootPath $RepoRootPath
            Count = $count
        }
    }

    return $records
}

function Sum-RecordCounts {
    param(
        [object[]]$Records
    )

    if ($null -eq $Records -or $Records.Count -eq 0) {
        return 0
    }

    $sum = ($Records | Measure-Object -Property Count -Sum).Sum
    if ($null -eq $sum) {
        return 0
    }
    return [int]$sum
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$repoRootPath = $repoRoot.Path
Push-Location $repoRoot

try {
    $allHeaders = @(
        Get-ChildItem "webassembly/src" -Recurse -Include *.h |
            Select-Object -ExpandProperty FullName
    )
    $allCppFiles = @(
        Get-ChildItem "webassembly/src" -Recurse -Include *.cpp |
            Select-Object -ExpandProperty FullName
    )
    $allSourceFiles = @($allHeaders + $allCppFiles)

    # W5-1) legacy alias references must be zero.
    $legacyAliasPattern =
        "atoms::domain::createdAtoms|atoms::domain::createdBonds|atoms::domain::cellInfo|using\s+atoms::domain::(createdAtoms|createdBonds|cellInfo)\b"
    $legacyAliasRecords = Get-MatchRecordsWithoutComments `
        -Paths $allSourceFiles `
        -Pattern $legacyAliasPattern `
        -RepoRootPath $repoRootPath
    $legacyAliasCount = Sum-RecordCounts -Records $legacyAliasRecords

    # W5-2) legacy alias exposure outside atoms module must remain zero.
    $legacyAliasOutsideAtoms = @(
        $legacyAliasRecords |
            Where-Object { $_.Path -notmatch "^webassembly/src/atoms/" }
    )
    $legacyAliasOutsideAtomsCount = Sum-RecordCounts -Records $legacyAliasOutsideAtoms

    # W5-3) friend class declarations must be zero.
    $friendClassRecords = Get-MatchRecordsWithoutComments `
        -Paths $allHeaders `
        -Pattern "\bfriend\s+class\b" `
        -RepoRootPath $repoRootPath
    $friendClassCount = Sum-RecordCounts -Records $friendClassRecords

    # W5-4) menu open regression guard code path must exist.
    $appCppPath = "webassembly/src/app.cpp"
    $appCppText = Get-TextWithoutComments -Path $appCppPath
    $menuRequiredPatterns = @(
        'auto\s+openEditorPanel\s*=\s*\[&\]\(shell::application::EditorPanelAction',
        'auto\s+openBuilderPanel\s*=\s*\[&\]\(shell::application::BuilderPanelAction',
        'auto\s+openDataPanel\s*=\s*\[&\]\(shell::application::DataPanelAction',
        'ImGui::MenuItem\("Atoms"\)',
        'ImGui::MenuItem\("Bonds"\)',
        'ImGui::MenuItem\("Cell"\)',
        'ImGui::MenuItem\("Add atoms"\)',
        'ImGui::MenuItem\("Bravais Lattice Templates"\)',
        'ImGui::MenuItem\("Isosurface"\)',
        'ImGui::MenuItem\("Surface"\)',
        'ImGui::MenuItem\("Volumetric"\)',
        'ImGui::MenuItem\("Plane"\)',
        'ImGui::MenuItem\("Brillouin Zone"\)',
        'm_bShowCreatedAtomsWindow\s*=\s*true;',
        'm_bShowBondsManagementWindow\s*=\s*true;',
        'm_bShowCellInformationWindow\s*=\s*true;',
        'm_bShowPeriodicTableWindow\s*=\s*true;',
        'm_bShowCrystalTemplatesWindow\s*=\s*true;',
        'm_bShowChargeDensityViewerWindow\s*=\s*true;',
        'm_bShowSliceViewerWindow\s*=\s*true;',
        'm_bShowBrillouinZonePlotWindow\s*=\s*true;',
        'requestLocalFocus\(FocusTarget::BrillouinZonePlot\)',
        'syncShellStateToStore\s*\(\s*\)\s*;'
    )
    $missingMenuPatterns = @()
    foreach ($pattern in $menuRequiredPatterns) {
        if (-not [regex]::IsMatch($appCppText, $pattern)) {
            $missingMenuPatterns += $pattern
        }
    }
    $menuRegressionGuardPass = ($missingMenuPatterns.Count -eq 0)

    # W5-5) bug tracking file exists and has status tag.
    $bugLogPath = "docs/refactoring/phase14/logs/bug_p14_vasp_grid_sequence_latest.md"
    $bugLogExists = Test-Path $bugLogPath
    $bugLogRaw = if ($bugLogExists) { Get-Content -Raw -Encoding utf8 $bugLogPath } else { "" }
    $bugLogHasBugId = $bugLogRaw.Contains("P9-BUG-01")
    $bugStatusMatch = [regex]::Match(
        $bugLogRaw,
        'Status:\s*`?(Open|Resolved|Deferred)`?',
        [System.Text.RegularExpressions.RegexOptions]::IgnoreCase)
    $bugLogHasStatusTag = $bugStatusMatch.Success
    $bugStatusValue = if ($bugStatusMatch.Success) { $bugStatusMatch.Groups[1].Value } else { "N/A" }

    $results = @(
        (New-Result "P14.legacy_alias_refs_zero" ($legacyAliasCount -eq 0) $legacyAliasCount 0),
        (New-Result "P14.legacy_alias_outside_atoms_zero" ($legacyAliasOutsideAtomsCount -eq 0) $legacyAliasOutsideAtomsCount 0),
        (New-Result "P14.friend_class_decl_zero" ($friendClassCount -eq 0) $friendClassCount 0),
        (New-Result "P14.menu_open_regression_guard_codepath" $menuRegressionGuardPass ($missingMenuPatterns.Count) 0),
        (New-Result "P14.bug_log_exists" $bugLogExists ([int]$bugLogExists) 1),
        (New-Result "P14.bug_log_has_bug_id" $bugLogHasBugId ([int]$bugLogHasBugId) 1),
        (New-Result "P14.bug_log_has_status_tag" $bugLogHasStatusTag ([int]$bugLogHasStatusTag) 1)
    )

    Write-Host ("Phase 14 State Ownership/Alias Zero Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host "Phase 14 guard snapshot:"
    Write-Host (" - legacy alias refs: {0}" -f $legacyAliasCount)
    Write-Host (" - legacy alias refs outside atoms: {0}" -f $legacyAliasOutsideAtomsCount)
    Write-Host (" - friend class declarations: {0}" -f $friendClassCount)
    Write-Host (" - bug log status: {0}" -f $bugStatusValue)

    if ($legacyAliasCount -gt 0) {
        Write-Host ""
        Write-Host "[P14.legacy_alias_refs_zero] Violations:"
        foreach ($entry in $legacyAliasRecords) {
            Write-Host (" - {0} (count={1})" -f $entry.Path, $entry.Count)
        }
    }

    if ($legacyAliasOutsideAtomsCount -gt 0) {
        Write-Host ""
        Write-Host "[P14.legacy_alias_outside_atoms_zero] Violations:"
        foreach ($entry in $legacyAliasOutsideAtoms) {
            Write-Host (" - {0} (count={1})" -f $entry.Path, $entry.Count)
        }
    }

    if ($friendClassCount -gt 0) {
        Write-Host ""
        Write-Host "[P14.friend_class_decl_zero] Violations:"
        foreach ($entry in $friendClassRecords) {
            Write-Host (" - {0} (count={1})" -f $entry.Path, $entry.Count)
        }
    }

    if (-not $menuRegressionGuardPass) {
        Write-Host ""
        Write-Host "[P14.menu_open_regression_guard_codepath] Missing patterns:"
        foreach ($pattern in $missingMenuPatterns) {
            Write-Host (" - {0}" -f $pattern)
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
