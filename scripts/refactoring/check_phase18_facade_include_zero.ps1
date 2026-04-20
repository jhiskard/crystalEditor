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

function Invoke-RgLines {
    param(
        [string]$Pattern,
        [string[]]$Paths,
        [string[]]$Globs
    )

    $args = @("--line-number", $Pattern) + $Paths
    foreach ($glob in $Globs) {
        $args += @("-g", $glob)
    }

    $lines = @(& rg @args 2>$null)
    if ($LASTEXITCODE -gt 1) {
        throw "rg failed for pattern: $Pattern"
    }

    if ($LASTEXITCODE -eq 1) {
        return @()
    }

    return $lines
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    $pattern = '#include\s+\".*workspace/legacy/atoms_template_facade.h\"'
    $allMatches = Invoke-RgLines -Pattern $pattern -Paths @("webassembly/src") -Globs @("*.cpp", "*.h")
    $externalMatches = @(
        $allMatches |
            Where-Object { $_ -notmatch "^webassembly/src/workspace/legacy/" }
    )

    $results = @(
        (New-Result "P18W7.facade_include_total_count_zero" ($allMatches.Count -eq 0) $allMatches.Count 0),
        (New-Result "P18W7.facade_include_external_count_zero" ($externalMatches.Count -eq 0) $externalMatches.Count 0)
    )

    Write-Host ("Phase 18-W7 Facade Include Zero Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    if ($externalMatches.Count -gt 0) {
        Write-Host ""
        Write-Host "[P18W7.facade_include_external_count_zero] remaining includes:"
        foreach ($line in $externalMatches) {
            Write-Host (" - {0}" -f $line)
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
