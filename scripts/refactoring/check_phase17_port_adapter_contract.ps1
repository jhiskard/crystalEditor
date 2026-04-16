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

function Get-LocalIncludes {
    param(
        [string]$Path
    )

    if (-not (Test-Path $Path)) {
        return @()
    }

    $lines = Get-Content -Encoding utf8 $Path
    $includes = @()
    foreach ($line in $lines) {
        $match = [regex]::Match($line, '^\s*#include\s+"([^"]+)"')
        if ($match.Success) {
            $includes += $match.Groups[1].Value
        }
    }

    return @($includes)
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    $ioApplicationFiles = @(
        Get-ChildItem -Path "webassembly/src/io/application/*.h", "webassembly/src/io/application/*.cpp" -File |
            Sort-Object FullName
    )

    $forbiddenIoIncludePattern =
        '^(.*emscripten.*|.*imgui\.h|.*platform/browser/browser_file_picker\.h|.*platform/browser/browser_file_dialog_adapter\.h|.*platform/worker/emscripten_worker_port\.h|.*platform/worker/runtime_progress_port\.h)$'
    $forbiddenIoSymbolPattern = '(BrowserFilePicker::|emscripten::|emscripten_|EM_ASM\s*\()'
    $forbiddenConcreteAdapterTokenPattern = '\b(BrowserFileDialogAdapter|EmscriptenWorkerPort|RuntimeProgressPort)\b'

    $ioForbiddenIncludes = @()
    $ioForbiddenSymbols = @()
    $ioConcreteAdapterTokens = @()

    foreach ($file in $ioApplicationFiles) {
        $includes = Get-LocalIncludes -Path $file.FullName
        foreach ($include in $includes) {
            if ($include -match $forbiddenIoIncludePattern) {
                $ioForbiddenIncludes += ("{0}: {1}" -f $file.FullName, $include)
            }
        }

        $text = Get-TextWithoutComments -Path $file.FullName
        if ([regex]::IsMatch($text, $forbiddenIoSymbolPattern)) {
            $ioForbiddenSymbols += $file.FullName
        }
        if ([regex]::IsMatch($text, $forbiddenConcreteAdapterTokenPattern)) {
            $ioConcreteAdapterTokens += $file.FullName
        }
    }

    $ioForbiddenSymbols = @($ioForbiddenSymbols | Sort-Object -Unique)
    $ioConcreteAdapterTokens = @($ioConcreteAdapterTokens | Sort-Object -Unique)

    $importEntryHeaderPath = "webassembly/src/io/application/import_entry_service.h"
    $parserWorkerHeaderPath = "webassembly/src/io/application/parser_worker_service.h"

    $importIncludes = Get-LocalIncludes -Path $importEntryHeaderPath
    $parserIncludes = Get-LocalIncludes -Path $parserWorkerHeaderPath

    $requiredImportPortIncludes = @("file_dialog_port.h", "worker_port.h", "progress_port.h")
    $requiredParserPortIncludes = @("worker_port.h", "progress_port.h")

    $missingImportPortIncludes = @($requiredImportPortIncludes | Where-Object { $_ -notin $importIncludes })
    $missingParserPortIncludes = @($requiredParserPortIncludes | Where-Object { $_ -notin $parserIncludes })

    $runtimeCppPath = "webassembly/src/shell/runtime/workbench_runtime.cpp"
    $runtimeIncludes = Get-LocalIncludes -Path $runtimeCppPath
    $runtimeText = Get-TextWithoutComments -Path $runtimeCppPath

    $requiredRuntimeAdapterIncludes = @(
        "../../platform/browser/browser_file_dialog_adapter.h",
        "../../platform/worker/emscripten_worker_port.h",
        "../../platform/worker/runtime_progress_port.h"
    )
    $missingRuntimeAdapterIncludes = @(
        $requiredRuntimeAdapterIncludes | Where-Object { $_ -notin $runtimeIncludes }
    )

    $runtimeAdapterWiringPass =
        [regex]::IsMatch($runtimeText, 'static\s+platform::browser::BrowserFileDialogAdapter\s+\w+\s*;') -and
        [regex]::IsMatch($runtimeText, 'static\s+platform::worker::EmscriptenWorkerPort\s+\w+\s*;') -and
        [regex]::IsMatch($runtimeText, 'static\s+platform::worker::RuntimeProgressPort\s+\w+\s*;') -and
        [regex]::IsMatch($runtimeText, 'static\s+FileLoader\s+\w+\s*\(\s*\w+\s*,\s*\w+\s*,\s*\w+\s*\)\s*;')

    $runtimeLegacyRootIncludePattern = '^\s*#include\s+"(\.\./\.\./file_loader\.h|\.\./\.\./unv_reader\.h|\.\./\.\./vtk_viewer\.h|\.\./\.\./mouse_interactor_style\.h)"'
    $runtimeLegacyRootIncludes = @(
        Get-Content -Encoding utf8 $runtimeCppPath |
            Where-Object { $_ -match $runtimeLegacyRootIncludePattern }
    )

    $renderAppEmAsmRaw = & rg -n 'EM_ASM\s*\(' webassembly/src/render/application 2>$null
    if ($LASTEXITCODE -notin @(0, 1)) {
        throw "Failed to execute EM_ASM scan on render/application."
    }
    $renderAppEmAsmMatches = @($renderAppEmAsmRaw)

    $renderAppEmscriptenIncludeRaw = & rg -n '#include\s+[<"]emscripten/' webassembly/src/render/application 2>$null
    if ($LASTEXITCODE -notin @(0, 1)) {
        throw "Failed to execute emscripten include scan on render/application."
    }
    $renderAppEmscriptenIncludeMatches = @($renderAppEmscriptenIncludeRaw)

    $runtimeAdapterWiringCurrent = if ($runtimeAdapterWiringPass) { 0 } else { 1 }

    $results = @(
        (New-Result "P17R3.io_app_forbidden_include_zero" ($ioForbiddenIncludes.Count -eq 0) $ioForbiddenIncludes.Count 0),
        (New-Result "P17R3.io_app_forbidden_symbol_zero" ($ioForbiddenSymbols.Count -eq 0) $ioForbiddenSymbols.Count 0),
        (New-Result "P17R3.io_app_concrete_adapter_token_zero" ($ioConcreteAdapterTokens.Count -eq 0) $ioConcreteAdapterTokens.Count 0),
        (New-Result "P17R3.io_app_port_headers_wired" ($missingImportPortIncludes.Count -eq 0 -and $missingParserPortIncludes.Count -eq 0) ($missingImportPortIncludes.Count + $missingParserPortIncludes.Count) 0),
        (New-Result "P17R3.runtime_adapter_includes_present" ($missingRuntimeAdapterIncludes.Count -eq 0) $missingRuntimeAdapterIncludes.Count 0),
        (New-Result "P17R3.runtime_adapter_wiring_present" $runtimeAdapterWiringPass $runtimeAdapterWiringCurrent 0),
        (New-Result "P17R3.runtime_legacy_root_include_zero" ($runtimeLegacyRootIncludes.Count -eq 0) $runtimeLegacyRootIncludes.Count 0),
        (New-Result "P17R3.render_app_em_asm_zero" ($renderAppEmAsmMatches.Count -eq 0) $renderAppEmAsmMatches.Count 0),
        (New-Result "P17R3.render_app_emscripten_include_zero" ($renderAppEmscriptenIncludeMatches.Count -eq 0) $renderAppEmscriptenIncludeMatches.Count 0)
    )

    Write-Host ("Phase 17-R3 Port/Adapter Contract Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($result in $results) {
        $status = if ($result.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $result.Name, $status, $result.Current, $result.Target)
    }

    Write-Host ""
    Write-Host "Phase 17-R3 port/adapter snapshot:"
    Write-Host (" - io/application files scanned: {0}" -f $ioApplicationFiles.Count)
    Write-Host (" - io forbidden includes: {0}" -f $ioForbiddenIncludes.Count)
    Write-Host (" - io forbidden symbols: {0}" -f $ioForbiddenSymbols.Count)
    Write-Host (" - runtime adapter wiring pass: {0}" -f $runtimeAdapterWiringPass)

    if ($ioForbiddenIncludes.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R3.io_app_forbidden_include_zero] Violations:"
        foreach ($entry in $ioForbiddenIncludes) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($ioForbiddenSymbols.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R3.io_app_forbidden_symbol_zero] Violations:"
        foreach ($entry in $ioForbiddenSymbols) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($ioConcreteAdapterTokens.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R3.io_app_concrete_adapter_token_zero] Violations:"
        foreach ($entry in $ioConcreteAdapterTokens) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($missingImportPortIncludes.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R3.io_app_port_headers_wired] Missing import_entry_service ports:"
        foreach ($entry in $missingImportPortIncludes) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($missingParserPortIncludes.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R3.io_app_port_headers_wired] Missing parser_worker_service ports:"
        foreach ($entry in $missingParserPortIncludes) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($missingRuntimeAdapterIncludes.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R3.runtime_adapter_includes_present] Missing runtime adapter includes:"
        foreach ($entry in $missingRuntimeAdapterIncludes) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($runtimeLegacyRootIncludes.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R3.runtime_legacy_root_include_zero] Violations:"
        foreach ($entry in $runtimeLegacyRootIncludes) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($renderAppEmAsmMatches.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R3.render_app_em_asm_zero] Violations:"
        foreach ($entry in $renderAppEmAsmMatches) {
            Write-Host (" - {0}" -f $entry)
        }
    }

    if ($renderAppEmscriptenIncludeMatches.Count -gt 0) {
        Write-Host ""
        Write-Host "[P17R3.render_app_emscripten_include_zero] Violations:"
        foreach ($entry in $renderAppEmscriptenIncludeMatches) {
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
