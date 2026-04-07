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
    # Phase 15 W2 snapshot lock.
    # W5 gate must reject any new non-render direct manipulation inflow above this level.
    $maxAllowedNonRenderDirectManipulation = 110

    $allHeaders = @(
        Get-ChildItem "webassembly/src" -Recurse -Include *.h |
            Select-Object -ExpandProperty FullName
    )
    $allCppFiles = @(
        Get-ChildItem "webassembly/src" -Recurse -Include *.cpp |
            Select-Object -ExpandProperty FullName
    )
    $allSourceFiles = @($allHeaders + $allCppFiles)

    $nonRenderFiles = @(
        $allSourceFiles |
            Where-Object {
                (To-RelativePath -FullPath $_ -RepoRootPath $repoRootPath) -notmatch "^webassembly/src/render/"
            }
    )

    # W5-1) non-render render/infrastructure include must remain zero.
    $renderInfraRecords = Get-MatchRecordsWithoutComments `
        -Paths $nonRenderFiles `
        -Pattern "render/infrastructure" `
        -RepoRootPath $repoRootPath
    $renderInfraCount = Sum-RecordCounts -Records $renderInfraRecords

    # W5-2) non-render direct manipulation inflow must not increase.
    $directManipPattern = "AddActor\(|RemoveActor\(|AddVolume\(|RemoveVolume\(|SetMapper\(|GetProperty\("
    $directManipRecords = Get-MatchRecordsWithoutComments `
        -Paths $nonRenderFiles `
        -Pattern $directManipPattern `
        -RepoRootPath $repoRootPath
    $nonRenderDirectManipCount = Sum-RecordCounts -Records $directManipRecords
    $directManipulationPass = ($nonRenderDirectManipCount -le $maxAllowedNonRenderDirectManipulation)

    # W5-3) platform boundary code/docs must be present and legacy path removed.
    $platformRequiredPaths = @(
        "webassembly/src/platform/browser/browser_file_picker.h",
        "webassembly/src/platform/browser/browser_file_picker.cpp",
        "docs/refactoring/phase15/logs/platform_boundary_inventory_phase15_latest.md",
        "docs/refactoring/phase15/logs/render_camera_picking_overlay_contract_phase15_latest.md"
    )
    $platformRequiredMissing = @(
        $platformRequiredPaths | Where-Object { -not (Test-Path $_) }
    )
    $platformRequiredPass = ($platformRequiredMissing.Count -eq 0)

    $legacyPlatformPaths = @(
        "webassembly/src/io/platform/browser_file_picker.h",
        "webassembly/src/io/platform/browser_file_picker.cpp"
    )
    $legacyPlatformRemaining = @(
        $legacyPlatformPaths | Where-Object { Test-Path $_ }
    )
    $legacyPlatformRemovedPass = ($legacyPlatformRemaining.Count -eq 0)

    $platformInventoryText = Get-TextWithoutComments -Path "docs/refactoring/phase15/logs/platform_boundary_inventory_phase15_latest.md"
    $platformInventoryHasBrowserBoundary = [regex]::IsMatch($platformInventoryText, "platform/browser")

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
    $bugLogPath = "docs/refactoring/phase15/logs/bug_p15_vasp_grid_sequence_latest.md"
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
        (New-Result "P15.non_render_render_infra_include_zero" ($renderInfraCount -eq 0) $renderInfraCount 0),
        (New-Result "P15.non_render_direct_manipulation_inflow_zero" $directManipulationPass $nonRenderDirectManipCount ("<= {0}" -f $maxAllowedNonRenderDirectManipulation)),
        (New-Result "P15.platform_boundary_required_paths_exist" $platformRequiredPass ($platformRequiredPaths.Count - $platformRequiredMissing.Count) $platformRequiredPaths.Count),
        (New-Result "P15.platform_boundary_legacy_io_platform_removed" $legacyPlatformRemovedPass ($legacyPlatformPaths.Count - $legacyPlatformRemaining.Count) $legacyPlatformPaths.Count),
        (New-Result "P15.platform_boundary_inventory_mentions_platform_browser" $platformInventoryHasBrowserBoundary ([int]$platformInventoryHasBrowserBoundary) 1),
        (New-Result "P15.menu_open_regression_guard_codepath" $menuRegressionGuardPass ($missingMenuPatterns.Count) 0),
        (New-Result "P15.bug_log_exists" $bugLogExists ([int]$bugLogExists) 1),
        (New-Result "P15.bug_log_has_bug_id" $bugLogHasBugId ([int]$bugLogHasBugId) 1),
        (New-Result "P15.bug_log_has_status_tag" $bugLogHasStatusTag ([int]$bugLogHasStatusTag) 1)
    )

    Write-Host ("Phase 15 Render/Platform Isolation Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host "Phase 15 guard snapshot:"
    Write-Host (" - non-render render/infrastructure refs: {0}" -f $renderInfraCount)
    Write-Host (" - non-render direct manipulation refs: {0}" -f $nonRenderDirectManipCount)
    Write-Host (" - bug log status: {0}" -f $bugStatusValue)

    if ($renderInfraCount -gt 0) {
        Write-Host ""
        Write-Host "[P15.non_render_render_infra_include_zero] Violations:"
        foreach ($entry in $renderInfraRecords) {
            Write-Host (" - {0} (count={1})" -f $entry.Path, $entry.Count)
        }
    }

    if (-not $directManipulationPass) {
        Write-Host ""
        Write-Host "[P15.non_render_direct_manipulation_inflow_zero] Top entries:"
        $directManipRecords |
            Sort-Object -Property Count -Descending |
            Select-Object -First 15 |
            ForEach-Object {
                Write-Host (" - {0} (count={1})" -f $_.Path, $_.Count)
            }
    }

    if (-not $platformRequiredPass) {
        Write-Host ""
        Write-Host "[P15.platform_boundary_required_paths_exist] Missing paths:"
        foreach ($path in $platformRequiredMissing) {
            Write-Host (" - {0}" -f $path)
        }
    }

    if (-not $legacyPlatformRemovedPass) {
        Write-Host ""
        Write-Host "[P15.platform_boundary_legacy_io_platform_removed] Remaining legacy paths:"
        foreach ($path in $legacyPlatformRemaining) {
            Write-Host (" - {0}" -f $path)
        }
    }

    if (-not $menuRegressionGuardPass) {
        Write-Host ""
        Write-Host "[P15.menu_open_regression_guard_codepath] Missing patterns:"
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
