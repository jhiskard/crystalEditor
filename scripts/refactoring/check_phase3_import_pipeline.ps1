param(
    [switch]$FailOnViolation = $true
)

$ErrorActionPreference = "Stop"

function Get-RgLines {
    param(
        [string[]]$Arguments
    )

    $lines = & rg @Arguments 2>$null
    if ($LASTEXITCODE -eq 1) {
        return @()
    }
    if ($LASTEXITCODE -ne 0) {
        throw "rg failed (exit=$LASTEXITCODE): $($Arguments -join ' ')"
    }
    return @($lines | Where-Object { $_ -and $_.Trim() -ne "" })
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Push-Location $repoRoot

try {
    $baselineFileLoaderLines = 1449
    $targetMaxFileLoaderLines = [int][Math]::Floor($baselineFileLoaderLines * 0.7)
    $currentFileLoaderLines = (Get-Content "webassembly/src/file_loader.cpp" | Measure-Object -Line).Lines

    $emAsmLines = Get-RgLines @(
        "-n",
        "EM_ASM|EMSCRIPTEN",
        "webassembly/src/file_loader.cpp"
    )

    $entrySignatureLines = Get-RgLines @(
        "-n",
        "void FileLoader::(OnParserWorkerResult|ApplyXSFParseResult|ApplyXSFGridParseResult|ApplyChgcarParseResult)\(",
        "webassembly/src/file_loader.cpp"
    )

    $entryNames = @(
        $entrySignatureLines |
            ForEach-Object {
                if ($_ -match 'FileLoader::([A-Za-z0-9_]+)\(') {
                    $matches[1]
                }
            } |
            Where-Object { $_ } |
            Select-Object -Unique
    )
    $entryCount = $entryNames.Count

    $parserDelegationLines = Get-RgLines @(
        "-n",
        "m_ParserWorkerService\.Process(StructureFile|XsfFile|XsfGridFile|ChgcarFile)\(",
        "webassembly/src/file_loader.cpp"
    )

    $browserAdapterLines = Get-RgLines @(
        "-n",
        "BrowserFilePicker::Open(MeshFileBrowser|MeshFileStreamingBrowser|StructureFileBrowser|XsfFileBrowser|XsfGridFileBrowser|ChgcarFileBrowser)\(",
        "webassembly/src/file_loader.cpp"
    )

    $applyServiceLines = Get-RgLines @(
        "-n",
        "m_ImportApplyService\.Apply\(",
        "webassembly/src/file_loader.cpp"
    )

    $orchestratorLines = Get-RgLines @(
        "-n",
        "m_ImportOrchestrator\.(HasSceneDataForStructureImport|BeginReplaceSceneImportTransaction|FinalizeReplaceSceneImportSuccess|RollbackFailedStructureImport)\(",
        "webassembly/src/file_loader.cpp"
    )

    $requiredFiles = @(
        "webassembly/src/io/platform/browser_file_picker.h",
        "webassembly/src/io/platform/browser_file_picker.cpp",
        "webassembly/src/io/application/parser_worker_service.h",
        "webassembly/src/io/application/parser_worker_service.cpp",
        "webassembly/src/io/application/import_apply_service.h",
        "webassembly/src/io/application/import_apply_service.cpp",
        "webassembly/src/io/application/import_orchestrator.h",
        "webassembly/src/io/application/import_orchestrator.cpp"
    )
    $missingFiles = @($requiredFiles | Where-Object { -not (Test-Path $_) })

    $results = @(
        @{
            Name = "P3.file_loader_lines"
            Pass = $currentFileLoaderLines -le $targetMaxFileLoaderLines
            Current = $currentFileLoaderLines
            Target = "<= $targetMaxFileLoaderLines"
        },
        @{
            Name = "P3.file_loader_em_asm"
            Pass = $emAsmLines.Count -eq 0
            Current = $emAsmLines.Count
            Target = 0
        },
        @{
            Name = "P3.import_callback_entrypoints"
            Pass = $entryCount -ge 1 -and $entryCount -le 2
            Current = $entryCount
            Target = "1~2"
        },
        @{
            Name = "P3.parser_worker_delegate_calls"
            Pass = $parserDelegationLines.Count -eq 4
            Current = $parserDelegationLines.Count
            Target = 4
        },
        @{
            Name = "P3.browser_adapter_delegate_calls"
            Pass = $browserAdapterLines.Count -ge 6
            Current = $browserAdapterLines.Count
            Target = ">= 6"
        },
        @{
            Name = "P3.import_apply_delegate_calls"
            Pass = $applyServiceLines.Count -eq 1
            Current = $applyServiceLines.Count
            Target = 1
        },
        @{
            Name = "P3.import_orchestrator_delegate_calls"
            Pass = $orchestratorLines.Count -ge 4
            Current = $orchestratorLines.Count
            Target = ">= 4"
        },
        @{
            Name = "P3.required_files_exist"
            Pass = $missingFiles.Count -eq 0
            Current = if ($missingFiles.Count -eq 0) { 0 } else { $missingFiles.Count }
            Target = 0
        }
    )

    Write-Host ("Phase 3 Import Pipeline Check @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($r in $results) {
        $status = if ($r.Pass) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (current={2}, target={3})" -f $r.Name, $status, $r.Current, $r.Target)
    }

    if ($entryNames.Count -gt 0) {
        Write-Host (" - entrypoints: {0}" -f ($entryNames -join ", "))
    }

    if ($missingFiles.Count -gt 0) {
        Write-Host ""
        Write-Host "[P3.required_files_exist] Missing files:"
        foreach ($file in $missingFiles) {
            Write-Host (" - {0}" -f $file)
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
