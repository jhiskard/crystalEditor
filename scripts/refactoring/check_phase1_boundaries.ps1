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
    $r1Lines = Get-RgLines @(
        "-n",
        "using atoms::domain::(createdAtoms|createdBonds|surroundingAtoms|surroundingBonds|bondGroups)",
        "webassembly/src/model_tree.cpp"
    )

    $r2Lines = Get-RgLines @(
        "-n",
        "-P",
        "^(?!\s*//).*VtkViewer::Instance\(",
        "webassembly/src/atoms/domain",
        "webassembly/src/atoms/infrastructure"
    )

    $r3Lines = Get-RgLines @(
        "-n",
        '#include ".*\.\./(app|vtk_viewer|model_tree|mesh_detail|mesh_manager)\.h"|#include ".*\.\./\.\./(app|vtk_viewer|model_tree|mesh_detail|mesh_manager)\.h"',
        "webassembly/src/atoms/domain",
        "webassembly/src/atoms/infrastructure"
    )

    $r4Lines = Get-RgLines @(
        "-n",
        "-P",
        "--glob", "*.h",
        "^(?!\s*//).*extern\s+(std::vector<atoms::domain::AtomInfo>|std::vector<atoms::domain::BondInfo>|std::map<std::string,\s*atoms::domain::AtomGroupInfo>|std::map<std::string,\s*atoms::domain::BondGroupInfo>|CellInfo\s+cellInfo|bool\s+cellVisible|std::vector<vtkSmartPointer<vtkActor>>\s+cellEdgeActors)\b",
        "webassembly/src/atoms/domain"
    )

    $r5Lines = Get-RgLines @(
        "-n",
        "-P",
        "^(?!\s*//)\s*(std::vector<atoms::domain::AtomInfo>\s+createdAtoms|std::vector<atoms::domain::AtomInfo>\s+surroundingAtoms|std::map<std::string,\s*atoms::domain::AtomGroupInfo>\s+atomGroups|std::vector<atoms::domain::BondInfo>\s+createdBonds|std::vector<atoms::domain::BondInfo>\s+surroundingBonds|std::map<std::string,\s*atoms::domain::BondGroupInfo>\s+bondGroups|CellInfo\s+cellInfo|bool\s+cellVisible|std::vector<vtkSmartPointer<vtkActor>>\s+cellEdgeActors|static\s+bool\s+surroundingsVisible|static\s+uint32_t\s+nextAtomId)\b",
        "webassembly/src/atoms/domain/atom_manager.cpp",
        "webassembly/src/atoms/domain/bond_manager.cpp",
        "webassembly/src/atoms/domain/cell_manager.cpp"
    )

    $r6Lines = Get-RgLines @(
        "-n",
        "-P",
        "^(?!\s*//)\s*static\s+(bool\s+surroundingsVisible|uint32_t\s+nextBondId)\b",
        "webassembly/src/atoms/atoms_template.cpp"
    )

    $results = @(
        @{ Name = "R1.model_tree_using"; Count = $r1Lines.Count; Target = 0; Lines = $r1Lines },
        @{ Name = "R2.direct_vtkviewer"; Count = $r2Lines.Count; Target = 0; Lines = $r2Lines },
        @{ Name = "R3.include_violation"; Count = $r3Lines.Count; Target = 0; Lines = $r3Lines },
        @{ Name = "R4.domain_extern_state"; Count = $r4Lines.Count; Target = 0; Lines = $r4Lines },
        @{ Name = "R5.manager_global_definitions"; Count = $r5Lines.Count; Target = 0; Lines = $r5Lines },
        @{ Name = "R6.template_duplicate_static"; Count = $r6Lines.Count; Target = 0; Lines = $r6Lines }
    )

    Write-Host ("Phase Boundary Check (R1~R6) @ {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))
    foreach ($r in $results) {
        $status = if ($r.Count -le $r.Target) { "PASS" } else { "FAIL" }
        Write-Host (" - {0}: {1} (count={2}, target={3})" -f $r.Name, $status, $r.Count, $r.Target)
    }

    $violations = $results | Where-Object { $_.Count -gt $_.Target }
    if ($violations) {
        Write-Host ""
        Write-Host "Violation details:"
        foreach ($r in $violations) {
            Write-Host ("[{0}]" -f $r.Name)
            foreach ($line in $r.Lines) {
                Write-Host $line
            }
        }

        if ($FailOnViolation) {
            exit 1
        }
    }

    exit 0
}
finally {
    Pop-Location
}
