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

function Get-RelativePath {
    param(
        [string]$Root,
        [string]$Path
    )

    $fullRoot = [System.IO.Path]::GetFullPath($Root)
    $fullPath = [System.IO.Path]::GetFullPath($Path)
    if ($fullPath.StartsWith($fullRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
        $rel = $fullPath.Substring($fullRoot.Length).TrimStart('\', '/')
        return ($rel -replace '\\', '/')
    }
    return ($Path -replace '\\', '/')
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    $atomsCodeFiles = @()
    if (Test-Path "webassembly/src/atoms") {
        $atomsCodeFiles = @(
            Get-ChildItem -Path "webassembly/src/atoms" -Recurse -File |
                Where-Object { $_.Extension -in ".cpp", ".h" } |
                ForEach-Object { Get-RelativePath -Root $repoRoot -Path $_.FullName } |
                Sort-Object -Unique
        )
    }

    $legacyFacadeOldPathRefs = @(
        Get-ChildItem -Path "webassembly/src" -Recurse -File |
            Where-Object { $_.Extension -in ".cpp", ".h" } |
            Select-String -SimpleMatch "atoms/legacy/atoms_template_facade.h" |
            ForEach-Object { "{0}:{1}" -f (Get-RelativePath -Root $repoRoot -Path $_.Path), $_.LineNumber } |
            Sort-Object -Unique
    )

    $legacyPortAdapterFiles = @(
        Get-ChildItem -Path "webassembly/src" -Recurse -File |
            Where-Object {
                $_.Extension -in ".cpp", ".h" -and
                $_.Name -like "*legacy_port_adapter*"
            } |
            ForEach-Object { Get-RelativePath -Root $repoRoot -Path $_.FullName } |
            Sort-Object -Unique
    )

    $requiredWorkspaceLegacyFiles = @(
        "webassembly/src/workspace/legacy/atoms_template_facade.h",
        "webassembly/src/workspace/legacy/atoms_template_facade.cpp"
    )
    $missingWorkspaceLegacyFiles = @(
        $requiredWorkspaceLegacyFiles | Where-Object { -not (Test-Path $_) }
    )

    $results = @(
        (New-Result "P17R6.atoms_subtree_code_file_zero" ($atomsCodeFiles.Count -eq 0) $atomsCodeFiles.Count 0),
        (New-Result "P17R6.legacy_facade_old_path_reference_zero" ($legacyFacadeOldPathRefs.Count -eq 0) $legacyFacadeOldPathRefs.Count 0),
        (New-Result "P17R6.legacy_port_adapter_file_zero" ($legacyPortAdapterFiles.Count -eq 0) $legacyPortAdapterFiles.Count 0),
        (New-Result "P17R6.workspace_legacy_facade_files_present" ($missingWorkspaceLegacyFiles.Count -eq 0) $missingWorkspaceLegacyFiles.Count 0)
    )

    Write-Host ("Phase 17-R6 Atoms Legacy Zero Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    if ($atomsCodeFiles.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R6.atoms_subtree_code_file_zero] Residual files:"
        foreach ($entry in $atomsCodeFiles) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($legacyFacadeOldPathRefs.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R6.legacy_facade_old_path_reference_zero] Residual references:"
        foreach ($entry in $legacyFacadeOldPathRefs) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($legacyPortAdapterFiles.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R6.legacy_port_adapter_file_zero] Residual files:"
        foreach ($entry in $legacyPortAdapterFiles) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($missingWorkspaceLegacyFiles.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R6.workspace_legacy_facade_files_present] Missing files:"
        foreach ($entry in $missingWorkspaceLegacyFiles) {
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
