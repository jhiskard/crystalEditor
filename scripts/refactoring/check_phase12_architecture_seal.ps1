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

function Test-PathPrefix {
    param(
        [string]$Path,
        [string]$Prefix
    )

    $normalizedPath = $Path.Replace('\', '/')
    $normalizedPrefix = $Prefix.Replace('\', '/')
    return $normalizedPath.StartsWith($normalizedPrefix, [System.StringComparison]::OrdinalIgnoreCase)
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

function Get-ModuleEdges {
    param(
        [string]$CMakeRaw,
        [string[]]$Modules
    )

    $edges = @{}
    foreach ($module in $Modules) {
        $edges[$module] = @()
    }

    $matches = [regex]::Matches(
        $CMakeRaw,
        "target_link_libraries\(\s*(wb_[A-Za-z0-9_]+)\s+(?:PUBLIC|PRIVATE|INTERFACE)\s*([^\)]*)\)",
        [System.Text.RegularExpressions.RegexOptions]::Singleline)

    foreach ($match in $matches) {
        $from = $match.Groups[1].Value
        if (-not ($Modules -contains $from)) {
            continue
        }

        $depMatches = [regex]::Matches($match.Groups[2].Value, "\bwb_[A-Za-z0-9_]+\b")
        foreach ($depMatch in $depMatches) {
            $dep = $depMatch.Value
            if ($Modules -contains $dep -and $dep -ne $from) {
                $edges[$from] += $dep
            }
        }
    }

    foreach ($module in $Modules) {
        $edges[$module] = @($edges[$module] | Select-Object -Unique)
    }

    return $edges
}

function Test-AcyclicModuleGraph {
    param(
        [hashtable]$Edges,
        [string[]]$Modules
    )

    $indegree = @{}
    foreach ($module in $Modules) {
        $indegree[$module] = 0
    }

    foreach ($from in $Modules) {
        foreach ($to in $Edges[$from]) {
            if ($indegree.ContainsKey($to)) {
                $indegree[$to] += 1
            }
        }
    }

    $queue = New-Object System.Collections.Generic.Queue[string]
    foreach ($module in $Modules) {
        if ($indegree[$module] -eq 0) {
            $queue.Enqueue($module)
        }
    }

    $order = @()
    while ($queue.Count -gt 0) {
        $node = $queue.Dequeue()
        $order += $node

        foreach ($to in $Edges[$node]) {
            $indegree[$to] -= 1
            if ($indegree[$to] -eq 0) {
                $queue.Enqueue($to)
            }
        }
    }

    $remaining = @($Modules | Where-Object { $indegree[$_] -gt 0 })
    return @{
        IsAcyclic = ($order.Count -eq $Modules.Count)
        Topology = $order
        Remaining = $remaining
    }
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$repoRootPath = $repoRoot.Path
Push-Location $repoRoot

try {
    $allHeaders = @(
        Get-ChildItem "webassembly/src" -Recurse -Include *.h |
            Select-Object -ExpandProperty FullName
    )
    $allCppAndHeaders = @(
        Get-ChildItem "webassembly/src" -Recurse -Include *.cpp,*.h |
            Select-Object -ExpandProperty FullName
    )
    $allCppFiles = @(
        Get-ChildItem "webassembly/src" -Recurse -Include *.cpp |
            Select-Object -ExpandProperty FullName
    )

    $expectedSingletonDecls = [ordered]@{
        "App" = "webassembly/src/app.h"
        "Toolbar" = "webassembly/src/toolbar.h"
        "FileLoader" = "webassembly/src/file_loader.h"
        "MeshManager" = "webassembly/src/mesh_manager.h"
        "AtomsTemplate" = "webassembly/src/atoms/atoms_template.h"
        "VtkViewer" = "webassembly/src/vtk_viewer.h"
        "FontManager" = "webassembly/src/font_manager.h"
    }

    $singletonDeclEntries = @()
    foreach ($path in $allHeaders) {
        $relativePath = To-RelativePath -FullPath $path -RepoRootPath $repoRootPath
        if ($relativePath -eq "webassembly/src/macro/singleton_macro.h") {
            continue
        }

        $text = Get-TextWithoutComments -Path $path
        if ([string]::IsNullOrEmpty($text)) {
            continue
        }

        $matches = [regex]::Matches($text, "DECLARE_SINGLETON\s*\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)")
        foreach ($match in $matches) {
            $singletonDeclEntries += [pscustomobject]@{
                Class = $match.Groups[1].Value
                Path = $relativePath
            }
        }
    }

    $singletonUnexpected = @()
    foreach ($entry in $singletonDeclEntries) {
        if (-not $expectedSingletonDecls.Contains($entry.Class)) {
            $singletonUnexpected += $entry
            continue
        }

        if ($expectedSingletonDecls[$entry.Class] -ne $entry.Path) {
            $singletonUnexpected += $entry
        }
    }

    $singletonMissing = @()
    foreach ($className in $expectedSingletonDecls.Keys) {
        $expectedPath = $expectedSingletonDecls[$className]
        $found = @($singletonDeclEntries | Where-Object {
            $_.Class -eq $className -and $_.Path -eq $expectedPath
        })
        if ($found.Count -eq 0) {
            $singletonMissing += [pscustomobject]@{
                Class = $className
                Path = $expectedPath
            }
        }
    }

    $singletonDuplicates = @(
        $singletonDeclEntries |
            Group-Object Class,Path |
            Where-Object { $_.Count -gt 1 }
    )

    $coreSingletonPattern = "(App|Toolbar|FileLoader|MeshManager|AtomsTemplate|VtkViewer)::Instance\s*\("
    $appDomainCppFiles = @(
        $allCppFiles |
            Where-Object {
                $relative = To-RelativePath -FullPath $_ -RepoRootPath $repoRootPath
                $relative -match "^webassembly/src/.+/(application|domain)/.+\.cpp$"
            }
    )
    $coreSingletonCallRecords = Get-MatchRecordsWithoutComments `
        -Paths $appDomainCppFiles `
        -Pattern $coreSingletonPattern `
        -RepoRootPath $repoRootPath

    $coreSingletonAllowlistBudgets = [ordered]@{
        "webassembly/src/mesh/domain/mesh_repository.cpp" = 2
    }

    $coreSingletonOutsideAllowlist = @(
        $coreSingletonCallRecords |
            Where-Object { -not $coreSingletonAllowlistBudgets.Contains($_.Path) }
    )

    $coreSingletonAllowlistOverBudget = @()
    foreach ($allowPath in $coreSingletonAllowlistBudgets.Keys) {
        $budget = $coreSingletonAllowlistBudgets[$allowPath]
        $record = @($coreSingletonCallRecords | Where-Object { $_.Path -eq $allowPath } | Select-Object -First 1)
        $current = if ($record.Count -gt 0) { $record[0].Count } else { 0 }
        if ($current -gt $budget) {
            $coreSingletonAllowlistOverBudget += [pscustomobject]@{
                Path = $allowPath
                Count = $current
                Budget = $budget
            }
        }
    }

    $friendClassRecords = Get-MatchRecordsWithoutComments `
        -Paths $allHeaders `
        -Pattern "\bfriend\s+class\b" `
        -RepoRootPath $repoRootPath

    $friendClassAllowlistBudgets = [ordered]@{
        "webassembly/src/lcrs_tree.h" = 1
    }
    $friendClassOutsideAllowlist = @(
        $friendClassRecords |
            Where-Object { -not $friendClassAllowlistBudgets.Contains($_.Path) }
    )

    $friendClassAllowlistOverBudget = @()
    foreach ($allowPath in $friendClassAllowlistBudgets.Keys) {
        $budget = $friendClassAllowlistBudgets[$allowPath]
        $record = @($friendClassRecords | Where-Object { $_.Path -eq $allowPath } | Select-Object -First 1)
        $current = if ($record.Count -gt 0) { $record[0].Count } else { 0 }
        if ($current -gt $budget) {
            $friendClassAllowlistOverBudget += [pscustomobject]@{
                Path = $allowPath
                Count = $current
                Budget = $budget
            }
        }
    }

    $legacyAliasPattern = "atoms::domain::createdAtoms|atoms::domain::createdBonds|atoms::domain::cellInfo|using\s+atoms::domain::createdAtoms|using\s+atoms::domain::createdBonds|using\s+atoms::domain::cellInfo"
    $legacyAliasRecords = Get-MatchRecordsWithoutComments `
        -Paths $allCppAndHeaders `
        -Pattern $legacyAliasPattern `
        -RepoRootPath $repoRootPath
    $legacyAliasOutsideAtoms = @(
        $legacyAliasRecords |
            Where-Object { -not (Test-PathPrefix -Path $_.Path -Prefix "webassembly/src/atoms/") }
    )

    $viewerInstanceRecords = Get-MatchRecordsWithoutComments `
        -Paths $allCppFiles `
        -Pattern "VtkViewer::Instance\s*\(" `
        -RepoRootPath $repoRootPath
    $viewerInstanceAllowlist = @(
        "webassembly/src/render/infrastructure/vtk_render_gateway.cpp"
    )
    $viewerInstanceOutsideShim = @(
        $viewerInstanceRecords |
            Where-Object { $_.Path -notin $viewerInstanceAllowlist }
    )

    $legacyViewerFacadeRecords = Get-MatchRecordsWithoutComments `
        -Paths $allCppFiles `
        -Pattern "GetLegacyViewerFacade\s*\(" `
        -RepoRootPath $repoRootPath
    $legacyViewerFacadeAllowlist = @(
        "webassembly/src/render/infrastructure/vtk_render_gateway.cpp",
        "webassembly/src/shell/runtime/workbench_runtime.cpp"
    )
    $legacyViewerFacadeOutsideAllowlist = @(
        $legacyViewerFacadeRecords |
            Where-Object { $_.Path -notin $legacyViewerFacadeAllowlist }
    )

    $infraIncludePattern = "#include\s+[\""<].*render/infrastructure/vtk_render_gateway\.h[\"">]"
    $infraIncludeRecords = Get-MatchRecordsWithoutComments `
        -Paths $allCppAndHeaders `
        -Pattern $infraIncludePattern `
        -RepoRootPath $repoRootPath
    $infraIncludeAllowlist = @(
        "webassembly/src/shell/runtime/workbench_runtime.cpp"
    )
    $infraIncludeOutsideAllowlist = @(
        $infraIncludeRecords |
            Where-Object { $_.Path -notin $infraIncludeAllowlist }
    )

    $moduleTargets = @("wb_core", "wb_render", "wb_mesh", "wb_atoms", "wb_io", "wb_ui")
    $rootCMakePath = "CMakeLists.txt"
    $rootCMakeRaw = Get-Content -Raw -Encoding utf8 $rootCMakePath
    $moduleEdges = Get-ModuleEdges -CMakeRaw $rootCMakeRaw -Modules $moduleTargets
    $moduleGraph = Test-AcyclicModuleGraph -Edges $moduleEdges -Modules $moduleTargets

    $bugLogPath = "docs/refactoring/phase12/logs/bug_p12_vasp_grid_sequence_latest.md"
    $bugLogExists = Test-Path $bugLogPath
    $bugLogRaw = if ($bugLogExists) { Get-Content -Raw -Encoding utf8 $bugLogPath } else { "" }
    $bugLogHasBugId = $bugLogRaw.Contains("P9-BUG-01")
    $bugStatusMatch = [regex]::Match(
        $bugLogRaw,
        'Status:\s*`?(Open|Resolved|Deferred)`?',
        [System.Text.RegularExpressions.RegexOptions]::IgnoreCase)
    $bugLogHasStatusTag = $bugStatusMatch.Success
    $bugStatusValue = if ($bugStatusMatch.Success) { $bugStatusMatch.Groups[1].Value } else { "N/A" }

    $singletonAllowlistPass = (
        $singletonUnexpected.Count -eq 0 -and
        $singletonMissing.Count -eq 0 -and
        $singletonDuplicates.Count -eq 0
    )

    $coreSingletonOutsideAllowlistCount = Sum-RecordCounts -Records $coreSingletonOutsideAllowlist
    $friendClassOutsideAllowlistCount = Sum-RecordCounts -Records $friendClassOutsideAllowlist
    $legacyAliasOutsideAtomsCount = Sum-RecordCounts -Records $legacyAliasOutsideAtoms
    $viewerInstanceOutsideShimCount = Sum-RecordCounts -Records $viewerInstanceOutsideShim
    $legacyViewerFacadeOutsideAllowlistCount = Sum-RecordCounts -Records $legacyViewerFacadeOutsideAllowlist
    $infraIncludeOutsideAllowlistCount = Sum-RecordCounts -Records $infraIncludeOutsideAllowlist

    $results = @(
        (New-Result "P12.singleton_decl_allowlist_exact" $singletonAllowlistPass $singletonDeclEntries.Count $expectedSingletonDecls.Count),
        (New-Result "P12.feature_app_domain_core_instance_outside_allowlist_zero" ($coreSingletonOutsideAllowlistCount -eq 0) $coreSingletonOutsideAllowlistCount 0),
        (New-Result "P12.feature_app_domain_core_instance_allowlist_budget" ($coreSingletonAllowlistOverBudget.Count -eq 0) $coreSingletonAllowlistOverBudget.Count 0),
        (New-Result "P12.friend_class_outside_allowlist_zero" ($friendClassOutsideAllowlistCount -eq 0) $friendClassOutsideAllowlistCount 0),
        (New-Result "P12.friend_class_allowlist_budget" ($friendClassAllowlistOverBudget.Count -eq 0) $friendClassAllowlistOverBudget.Count 0),
        (New-Result "P12.legacy_alias_exposure_outside_atoms_zero" ($legacyAliasOutsideAtomsCount -eq 0) $legacyAliasOutsideAtomsCount 0),
        (New-Result "P12.non_shim_vtk_viewer_instance_zero" ($viewerInstanceOutsideShimCount -eq 0) $viewerInstanceOutsideShimCount 0),
        (New-Result "P12.legacy_viewer_facade_outside_allowlist_zero" ($legacyViewerFacadeOutsideAllowlistCount -eq 0) $legacyViewerFacadeOutsideAllowlistCount 0),
        (New-Result "P12.render_infra_include_outside_runtime_zero" ($infraIncludeOutsideAllowlistCount -eq 0) $infraIncludeOutsideAllowlistCount 0),
        (New-Result "P12.module_graph_acyclic" $moduleGraph.IsAcyclic ([int]$moduleGraph.IsAcyclic) 1),
        (New-Result "P12.bug_log_exists" $bugLogExists ([int]$bugLogExists) 1),
        (New-Result "P12.bug_log_has_bug_id" $bugLogHasBugId ([int]$bugLogHasBugId) 1),
        (New-Result "P12.bug_log_has_status_tag" $bugLogHasStatusTag ([int]$bugLogHasStatusTag) 1)
    )

    Write-Host ("Phase 12 Architecture Seal Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host "Phase 12 inventory snapshot:"
    Write-Host (" - singleton declarations found: {0}" -f $singletonDeclEntries.Count)
    Write-Host (" - core singleton calls in feature app/domain (outside allowlist): {0}" -f $coreSingletonOutsideAllowlistCount)
    Write-Host (" - friend class occurrences (outside allowlist): {0}" -f $friendClassOutsideAllowlistCount)
    Write-Host (" - direct legacy alias references outside atoms/*: {0}" -f $legacyAliasOutsideAtomsCount)
    Write-Host (" - VtkViewer::Instance() outside render shim: {0}" -f $viewerInstanceOutsideShimCount)
    Write-Host (" - GetLegacyViewerFacade() outside allowlist: {0}" -f $legacyViewerFacadeOutsideAllowlistCount)
    Write-Host (" - render infrastructure include outside runtime: {0}" -f $infraIncludeOutsideAllowlistCount)
    Write-Host (" - module graph acyclic: {0}" -f $moduleGraph.IsAcyclic)
    Write-Host (" - bug log status: {0}" -f $bugStatusValue)

    Write-Host ""
    Write-Host "Module graph edges:"
    foreach ($module in $moduleTargets) {
        $deps = @($moduleEdges[$module])
        if ($deps.Count -eq 0) {
            Write-Host (" - {0} -> (none)" -f $module)
        } else {
            Write-Host (" - {0} -> {1}" -f $module, ($deps -join ", "))
        }
    }

    if ($singletonUnexpected.Count -gt 0) {
        Write-Host ""
        Write-Host "[P12.singleton_decl_allowlist_exact] Unexpected singleton declarations:"
        foreach ($entry in $singletonUnexpected) {
            Write-Host (" - {0} @ {1}" -f $entry.Class, $entry.Path)
        }
    }
    if ($singletonMissing.Count -gt 0) {
        Write-Host ""
        Write-Host "[P12.singleton_decl_allowlist_exact] Missing singleton declarations:"
        foreach ($entry in $singletonMissing) {
            Write-Host (" - {0} @ {1}" -f $entry.Class, $entry.Path)
        }
    }
    if ($singletonDuplicates.Count -gt 0) {
        Write-Host ""
        Write-Host "[P12.singleton_decl_allowlist_exact] Duplicated singleton declarations:"
        foreach ($entry in $singletonDuplicates) {
            Write-Host (" - {0} (count={1})" -f $entry.Name, $entry.Count)
        }
    }

    if ($coreSingletonOutsideAllowlistCount -gt 0) {
        Write-Host ""
        Write-Host "[P12.feature_app_domain_core_instance_outside_allowlist_zero] Violations:"
        foreach ($entry in $coreSingletonOutsideAllowlist) {
            Write-Host (" - {0} (count={1})" -f $entry.Path, $entry.Count)
        }
    }
    if ($coreSingletonAllowlistOverBudget.Count -gt 0) {
        Write-Host ""
        Write-Host "[P12.feature_app_domain_core_instance_allowlist_budget] Over budget:"
        foreach ($entry in $coreSingletonAllowlistOverBudget) {
            Write-Host (" - {0} (current={1}, budget={2})" -f $entry.Path, $entry.Count, $entry.Budget)
        }
    }

    if ($friendClassOutsideAllowlistCount -gt 0) {
        Write-Host ""
        Write-Host "[P12.friend_class_outside_allowlist_zero] Violations:"
        foreach ($entry in $friendClassOutsideAllowlist) {
            Write-Host (" - {0} (count={1})" -f $entry.Path, $entry.Count)
        }
    }
    if ($friendClassAllowlistOverBudget.Count -gt 0) {
        Write-Host ""
        Write-Host "[P12.friend_class_allowlist_budget] Over budget:"
        foreach ($entry in $friendClassAllowlistOverBudget) {
            Write-Host (" - {0} (current={1}, budget={2})" -f $entry.Path, $entry.Count, $entry.Budget)
        }
    }

    if ($legacyAliasOutsideAtomsCount -gt 0) {
        Write-Host ""
        Write-Host "[P12.legacy_alias_exposure_outside_atoms_zero] Violations:"
        foreach ($entry in $legacyAliasOutsideAtoms) {
            Write-Host (" - {0} (count={1})" -f $entry.Path, $entry.Count)
        }
    }

    if ($viewerInstanceOutsideShimCount -gt 0) {
        Write-Host ""
        Write-Host "[P12.non_shim_vtk_viewer_instance_zero] Violations:"
        foreach ($entry in $viewerInstanceOutsideShim) {
            Write-Host (" - {0} (count={1})" -f $entry.Path, $entry.Count)
        }
    }

    if ($legacyViewerFacadeOutsideAllowlistCount -gt 0) {
        Write-Host ""
        Write-Host "[P12.legacy_viewer_facade_outside_allowlist_zero] Violations:"
        foreach ($entry in $legacyViewerFacadeOutsideAllowlist) {
            Write-Host (" - {0} (count={1})" -f $entry.Path, $entry.Count)
        }
    }

    if ($infraIncludeOutsideAllowlistCount -gt 0) {
        Write-Host ""
        Write-Host "[P12.render_infra_include_outside_runtime_zero] Violations:"
        foreach ($entry in $infraIncludeOutsideAllowlist) {
            Write-Host (" - {0} (count={1})" -f $entry.Path, $entry.Count)
        }
    }

    if (-not $moduleGraph.IsAcyclic) {
        Write-Host ""
        Write-Host "[P12.module_graph_acyclic] Remaining cyclic nodes:"
        foreach ($node in $moduleGraph.Remaining) {
            Write-Host (" - {0}" -f $node)
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
