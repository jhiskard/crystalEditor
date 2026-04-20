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
    $matches = Invoke-RgLines -Pattern 'AtomsTemplate::Instance\(' -Paths @("webassembly/src", "webassembly/tests") -Globs @("*.cpp", "*.h")

    $results = @(
        (New-Result "P18W7.atoms_instance_call_count_zero" ($matches.Count -eq 0) $matches.Count 0)
    )

    Write-Host ("Phase 18-W7 Atoms Instance Zero Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    if ($matches.Count -gt 0) {
        Write-Host ""
        Write-Host "[P18W7.atoms_instance_call_count_zero] remaining calls:"
        foreach ($line in $matches) {
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
