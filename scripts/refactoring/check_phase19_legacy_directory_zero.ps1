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

function To-RepoRelativePath {
    param(
        [string]$RepoRootPath,
        [string]$Path
    )

    if ($Path.StartsWith($RepoRootPath, [System.StringComparison]::OrdinalIgnoreCase)) {
        return $Path.Substring($RepoRootPath.Length).TrimStart('\', '/')
    }
    return $Path
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$repoRootPath = $repoRoot.Path
Push-Location $repoRoot

try {
    $legacyDirectories = @(
        Get-ChildItem -Path "webassembly/src" -Directory -Recurse |
            Where-Object { $_.Name -eq "legacy" } |
            ForEach-Object { To-RepoRelativePath -RepoRootPath $repoRootPath -Path $_.FullName }
    )

    $legacyNamedFiles = @(
        Get-ChildItem -Path "webassembly/src" -File -Recurse |
            Where-Object { $_.Name -match "legacy" } |
            ForEach-Object { To-RepoRelativePath -RepoRootPath $repoRootPath -Path $_.FullName }
    )

    $results = @(
        (New-Result "P19W9.legacy_directory_count_zero" ($legacyDirectories.Count -eq 0) $legacyDirectories.Count 0),
        (New-Result "P19W9.legacy_named_file_count_zero" ($legacyNamedFiles.Count -eq 0) $legacyNamedFiles.Count 0)
    )

    Write-Host ("Phase 19 Legacy Directory Zero Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    if ($legacyDirectories.Count -gt 0) {
        Write-Host ""
        Write-Host "[P19W9.legacy_directory_count_zero] remaining legacy directories:"
        foreach ($entry in $legacyDirectories) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($legacyNamedFiles.Count -gt 0) {
        Write-Host ""
        Write-Host "[P19W9.legacy_named_file_count_zero] remaining files:"
        foreach ($entry in $legacyNamedFiles) {
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
