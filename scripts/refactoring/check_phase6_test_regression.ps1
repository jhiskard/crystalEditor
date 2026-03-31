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

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    $expectedTestFiles = @(
        "webassembly/tests/io/chgcar_parser_test.cpp",
        "webassembly/tests/io/xsf_parser_test.cpp",
        "webassembly/tests/atoms/cell_transform_test.cpp",
        "webassembly/tests/atoms/measurement_geometry_test.cpp"
    )
    $expectedFixtures = @(
        "webassembly/tests/fixtures/io/sample_chgcar.vasp",
        "webassembly/tests/fixtures/io/sample_structure.xsf",
        "webassembly/tests/fixtures/io/sample_grid.xsf"
    )
    $expectedPureModules = @(
        "webassembly/src/atoms/domain/cell_transform.cpp",
        "webassembly/src/atoms/domain/measurement_geometry.cpp",
        "webassembly/src/io/infrastructure/xsf_parser.cpp"
    )

    $packageJson = Get-Content -Raw -Encoding utf8 "package.json" | ConvertFrom-Json
    $pageRaw = Get-Content -Raw -Encoding utf8 "app/workbench/page.tsx"
    $bindRaw = Get-Content -Raw -Encoding utf8 "webassembly/src/bind_function.cpp"
    $testsCMakeRaw = Get-Content -Raw -Encoding utf8 "webassembly/tests/CMakeLists.txt"

    $unitTestFiles = Get-ChildItem "webassembly/tests" -Recurse -Filter "*_test.cpp" | ForEach-Object {
        $_.FullName.Substring($repoRoot.Path.Length + 1).Replace('\', '/')
    }
    $fixtureFiles = Get-ChildItem "webassembly/tests/fixtures/io" -File | ForEach-Object {
        $_.FullName.Substring($repoRoot.Path.Length + 1).Replace('\', '/')
    }

    $missingTestFiles = @($expectedTestFiles | Where-Object { -not (Test-Path $_) })
    $missingFixtures = @($expectedFixtures | Where-Object { -not (Test-Path $_) })
    $missingPureModules = @($expectedPureModules | Where-Object { -not (Test-Path $_) })

    $scriptNames = @("test:cpp", "test:smoke", "test:phase6")
    $scriptPresence = 0
    foreach ($name in $scriptNames) {
        if ($packageJson.scripts.PSObject.Properties.Name -contains $name) {
            $scriptPresence += 1
        }
    }

    $pageReadySignal =
        $pageRaw.Contains('data-testid="workbench-root"') -and
        $pageRaw.Contains('data-ready={ready ? "true" : "false"}')
    $pageTestApi =
        $pageRaw.Contains("__VTK_WORKBENCH_TEST__") -and
        $pageRaw.Contains("importTextIntoModule")
    $bindingCoverage =
        $bindRaw.Contains('function("getStructureCount"') -and
        $bindRaw.Contains('function("setStructureVisible"') -and
        $bindRaw.Contains('function("hasChargeDensity"')
    $testsCMakeCoverage =
        $testsCMakeRaw.Contains("wb_native_tests") -and
        $testsCMakeRaw.Contains("measurement_geometry.cpp") -and
        $testsCMakeRaw.Contains("NODERAWFS=1")

    $results = @(
        (New-Result "P6.tests_cmake_exists" (Test-Path "webassembly/tests/CMakeLists.txt") ([int](Test-Path "webassembly/tests/CMakeLists.txt")) 1),
        (New-Result "P6.unit_test_files_present" ($missingTestFiles.Count -eq 0) ($unitTestFiles.Count) ($expectedTestFiles.Count)),
        (New-Result "P6.fixture_files_present" ($missingFixtures.Count -eq 0) ($fixtureFiles.Count) ($expectedFixtures.Count)),
        (New-Result "P6.package_test_scripts_present" ($scriptPresence -eq $scriptNames.Count) $scriptPresence $scriptNames.Count),
        (New-Result "P6.playwright_config_exists" (Test-Path "playwright.config.ts") ([int](Test-Path "playwright.config.ts")) 1),
        (New-Result "P6.smoke_spec_exists" (Test-Path "tests/e2e/workbench-smoke.spec.ts") ([int](Test-Path "tests/e2e/workbench-smoke.spec.ts")) 1),
        (New-Result "P6.page_ready_signal" $pageReadySignal ([int]$pageReadySignal) 1),
        (New-Result "P6.page_test_api" $pageTestApi ([int]$pageTestApi) 1),
        (New-Result "P6.wasm_state_bindings" $bindingCoverage ([int]$bindingCoverage) 1),
        (New-Result "P6.pure_test_modules_present" ($missingPureModules.Count -eq 0) (($expectedPureModules.Count - $missingPureModules.Count)) $expectedPureModules.Count),
        (New-Result "P6.test_runner_emsdk_fallback" $testsCMakeCoverage ([int]$testsCMakeCoverage) 1)
    )

    Write-Host ("Phase 6 Test Regression Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host "Phase 6 inventory snapshot:"
    Write-Host (" - unit test files: {0}" -f $unitTestFiles.Count)
    Write-Host (" - fixture files: {0}" -f $fixtureFiles.Count)
    Write-Host (" - test scripts: {0}" -f $scriptPresence)

    if ($missingTestFiles.Count -gt 0) {
        Write-Host ""
        Write-Host "[P6.unit_test_files_present] Missing test files:"
        foreach ($path in $missingTestFiles) {
            Write-Host (" - {0}" -f $path)
        }
    }

    if ($missingFixtures.Count -gt 0) {
        Write-Host ""
        Write-Host "[P6.fixture_files_present] Missing fixture files:"
        foreach ($path in $missingFixtures) {
            Write-Host (" - {0}" -f $path)
        }
    }

    if ($missingPureModules.Count -gt 0) {
        Write-Host ""
        Write-Host "[P6.pure_test_modules_present] Missing pure modules:"
        foreach ($path in $missingPureModules) {
            Write-Host (" - {0}" -f $path)
        }
    }

    $failed = @($results | Where-Object { -not $_.Pass })
    if ($failed.Count -gt 0 -and $FailOnViolation) {
        exit 1
    }
}
finally {
    Pop-Location
}
