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

    # W5-1) App/Toolbar/FileLoader singleton declaration check
    # Allowlist is intentionally empty for Phase 13 W5 target.
    $singletonAllowlist = @{}
    $singletonTargetClasses = @("App", "Toolbar", "FileLoader")
    $singletonEntries = @()

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
            $className = $match.Groups[1].Value
            if ($className -in $singletonTargetClasses) {
                $singletonEntries += [pscustomobject]@{
                    Class = $className
                    Path = $relativePath
                }
            }
        }
    }

    $singletonOutsideAllowlist = @()
    foreach ($entry in $singletonEntries) {
        if (-not $singletonAllowlist.ContainsKey($entry.Class)) {
            $singletonOutsideAllowlist += $entry
            continue
        }

        if ($singletonAllowlist[$entry.Class] -ne $entry.Path) {
            $singletonOutsideAllowlist += $entry
        }
    }

    $singletonMissingAllowlist = @()
    foreach ($allowClass in $singletonAllowlist.Keys) {
        $allowPath = $singletonAllowlist[$allowClass]
        $found = @($singletonEntries | Where-Object {
            $_.Class -eq $allowClass -and $_.Path -eq $allowPath
        })
        if ($found.Count -eq 0) {
            $singletonMissingAllowlist += [pscustomobject]@{
                Class = $allowClass
                Path = $allowPath
            }
        }
    }

    $singletonDeclPass = ($singletonOutsideAllowlist.Count -eq 0 -and $singletonMissingAllowlist.Count -eq 0)

    # W5-2) App/Toolbar/FileLoader::Instance() calls must be zero
    $instanceCallRecords = Get-MatchRecordsWithoutComments `
        -Paths $allCppFiles `
        -Pattern "(App|Toolbar|FileLoader)::Instance\s*\(" `
        -RepoRootPath $repoRootPath
    $instanceCallCount = Sum-RecordCounts -Records $instanceCallRecords

    # W5-3) Feature app/domain core singleton usage outside allowlist must be zero
    $appDomainCppFiles = @(
        $allCppFiles |
            Where-Object {
                $relative = To-RelativePath -FullPath $_ -RepoRootPath $repoRootPath
                $relative -match "^webassembly/src/.+/(application|domain)/.+\.cpp$"
            }
    )
    $featureCoreSingletonRecords = Get-MatchRecordsWithoutComments `
        -Paths $appDomainCppFiles `
        -Pattern "(App|Toolbar|FileLoader|MeshManager|AtomsTemplate|VtkViewer)::Instance\s*\(" `
        -RepoRootPath $repoRootPath
    $coreSingletonAllowlistBudgets = [ordered]@{
        "webassembly/src/mesh/domain/mesh_repository.cpp" = 2
    }
    $featureCoreSingletonOutsideAllowlist = @(
        $featureCoreSingletonRecords |
            Where-Object { -not $coreSingletonAllowlistBudgets.Contains($_.Path) }
    )
    $featureCoreSingletonAllowlistOverBudget = @()
    foreach ($allowPath in $coreSingletonAllowlistBudgets.Keys) {
        $budget = $coreSingletonAllowlistBudgets[$allowPath]
        $record = @($featureCoreSingletonRecords | Where-Object { $_.Path -eq $allowPath } | Select-Object -First 1)
        $current = if ($record.Count -gt 0) { $record[0].Count } else { 0 }
        if ($current -gt $budget) {
            $featureCoreSingletonAllowlistOverBudget += [pscustomobject]@{
                Path = $allowPath
                Count = $current
                Budget = $budget
            }
        }
    }
    $featureCoreSingletonOutsideAllowlistCount = Sum-RecordCounts -Records $featureCoreSingletonOutsideAllowlist

    # W5-4) Phase 7~12 protection rules
    $mainCppPath = "webassembly/src/main.cpp"
    $mainCppText = Get-TextWithoutComments -Path $mainCppPath
    $idxCreateContext = $mainCppText.IndexOf("ImGui::CreateContext(")
    $idxPrimeSingletons = $mainCppText.IndexOf("runtime.PrimeLegacySingletons(")
    $idxFontRegistry = $mainCppText.IndexOf("runtime.FontRegistry(")
    $imguiInitOrderPass = (
        $idxCreateContext -ge 0 -and
        $idxPrimeSingletons -gt $idxCreateContext -and
        $idxFontRegistry -gt $idxPrimeSingletons
    )

    $runtimeCppPath = "webassembly/src/shell/runtime/workbench_runtime.cpp"
    $runtimeCppText = Get-TextWithoutComments -Path $runtimeCppPath
    $primeMatch = [regex]::Match(
        $runtimeCppText,
        "void\s+WorkbenchRuntime::PrimeLegacySingletons\s*\(\)\s*\{(?<body>.*?)\}",
        [System.Text.RegularExpressions.RegexOptions]::Singleline)
    $primeNoFontRegistryPass = $primeMatch.Success -and -not [regex]::IsMatch($primeMatch.Groups["body"].Value, "FontRegistry\s*\(")

    $chargeDensityPath = "webassembly/src/atoms/ui/charge_density_ui.cpp"
    $chargeDensityRaw = if (Test-Path $chargeDensityPath) { Get-Content -Raw -Encoding utf8 $chargeDensityPath } else { "" }
    $chargeBootstrapGuardPass = (
        $chargeDensityRaw.Contains("Always bootstrap the first grid") -and
        [regex]::IsMatch($chargeDensityRaw, "loadFromGridEntry\s*\(\s*0\s*\)") -and
        $chargeDensityRaw.Contains("Failed to bootstrap first XSF grid entry after import.")
    )

    # W5-5) Bug log status tag check
    $bugLogPath = "docs/refactoring/phase13/logs/bug_p13_vasp_grid_sequence_latest.md"
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
        (New-Result "P13.singleton_decl_app_toolbar_fileloader_allowlist" $singletonDeclPass $singletonEntries.Count $singletonAllowlist.Count),
        (New-Result "P13.instance_calls_app_toolbar_fileloader_zero" ($instanceCallCount -eq 0) $instanceCallCount 0),
        (New-Result "P13.feature_app_domain_core_instance_outside_allowlist_zero" ($featureCoreSingletonOutsideAllowlistCount -eq 0) $featureCoreSingletonOutsideAllowlistCount 0),
        (New-Result "P13.feature_app_domain_core_instance_allowlist_budget" ($featureCoreSingletonAllowlistOverBudget.Count -eq 0) $featureCoreSingletonAllowlistOverBudget.Count 0),
        (New-Result "P13.main_imgui_init_order_guard" $imguiInitOrderPass ([int]$imguiInitOrderPass) 1),
        (New-Result "P13.prime_singletons_no_font_registry_call" $primeNoFontRegistryPass ([int]$primeNoFontRegistryPass) 1),
        (New-Result "P13.charge_density_first_grid_bootstrap_guard" $chargeBootstrapGuardPass ([int]$chargeBootstrapGuardPass) 1),
        (New-Result "P13.bug_log_exists" $bugLogExists ([int]$bugLogExists) 1),
        (New-Result "P13.bug_log_has_bug_id" $bugLogHasBugId ([int]$bugLogHasBugId) 1),
        (New-Result "P13.bug_log_has_status_tag" $bugLogHasStatusTag ([int]$bugLogHasStatusTag) 1)
    )

    Write-Host ("Phase 13 Runtime Hard Seal Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host "Phase 13 guard snapshot:"
    Write-Host (" - App/Toolbar/FileLoader singleton decl count: {0}" -f $singletonEntries.Count)
    Write-Host (" - App/Toolbar/FileLoader ::Instance() calls: {0}" -f $instanceCallCount)
    Write-Host (" - feature app/domain core singleton calls outside allowlist: {0}" -f $featureCoreSingletonOutsideAllowlistCount)
    Write-Host (" - bug log status: {0}" -f $bugStatusValue)

    if ($singletonOutsideAllowlist.Count -gt 0) {
        Write-Host ""
        Write-Host "[P13.singleton_decl_app_toolbar_fileloader_allowlist] Violations:"
        foreach ($entry in $singletonOutsideAllowlist) {
            Write-Host (" - {0} @ {1}" -f $entry.Class, $entry.Path)
        }
    }

    if ($singletonMissingAllowlist.Count -gt 0) {
        Write-Host ""
        Write-Host "[P13.singleton_decl_app_toolbar_fileloader_allowlist] Missing allowlist entries:"
        foreach ($entry in $singletonMissingAllowlist) {
            Write-Host (" - {0} @ {1}" -f $entry.Class, $entry.Path)
        }
    }

    if ($instanceCallCount -gt 0) {
        Write-Host ""
        Write-Host "[P13.instance_calls_app_toolbar_fileloader_zero] Violations:"
        foreach ($entry in $instanceCallRecords) {
            Write-Host (" - {0} (count={1})" -f $entry.Path, $entry.Count)
        }
    }

    if ($featureCoreSingletonOutsideAllowlistCount -gt 0) {
        Write-Host ""
        Write-Host "[P13.feature_app_domain_core_instance_outside_allowlist_zero] Violations:"
        foreach ($entry in $featureCoreSingletonOutsideAllowlist) {
            Write-Host (" - {0} (count={1})" -f $entry.Path, $entry.Count)
        }
    }

    if ($featureCoreSingletonAllowlistOverBudget.Count -gt 0) {
        Write-Host ""
        Write-Host "[P13.feature_app_domain_core_instance_allowlist_budget] Over budget:"
        foreach ($entry in $featureCoreSingletonAllowlistOverBudget) {
            Write-Host (" - {0} (current={1}, budget={2})" -f $entry.Path, $entry.Count, $entry.Budget)
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
