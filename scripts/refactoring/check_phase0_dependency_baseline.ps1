$ErrorActionPreference = "Stop"

$Root = "webassembly/src"
$IncludePattern = '#include ".*\.\./(app|vtk_viewer|model_tree|mesh_detail|mesh_manager)\.h"|#include ".*\.\./\.\./(app|vtk_viewer|model_tree|mesh_detail|mesh_manager)\.h"'
$GlobalContainerPattern = "atoms::domain::(createdAtoms|createdBonds|surroundingAtoms|surroundingBonds|atomGroups|bondGroups|cellInfo)"

function Count-Matches {
    param(
        [string]$Pattern,
        [string]$Path,
        [string[]]$ExtraArgs = @()
    )

    $results = & rg -n --color never @ExtraArgs -e $Pattern $Path
    if ($LASTEXITCODE -eq 0) {
        return ($results | Measure-Object).Count
    }
    return 0
}

Write-Output "=== Phase 0 Dependency Baseline ==="
Write-Output ("Timestamp: {0}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss zzz"))

$sourceCount = (& rg --files $Root | Measure-Object).Count
$atomsTemplateCount = Count-Matches -Pattern "AtomsTemplate::Instance\(" -Path $Root
$vtkViewerCount = Count-Matches -Pattern "VtkViewer::Instance\(" -Path $Root
$meshManagerCount = Count-Matches -Pattern "MeshManager::Instance\(" -Path $Root
$includeViolationCount = Count-Matches -Pattern $IncludePattern -Path "$Root/atoms"
$globalContainerCount = Count-Matches -Pattern $GlobalContainerPattern -Path $Root

Write-Output ("source_files={0}" -f $sourceCount)
Write-Output ("atoms_template_instance_calls={0}" -f $atomsTemplateCount)
Write-Output ("vtk_viewer_instance_calls={0}" -f $vtkViewerCount)
Write-Output ("mesh_manager_instance_calls={0}" -f $meshManagerCount)
Write-Output ("atoms_upward_include_violations={0}" -f $includeViolationCount)
Write-Output ("global_container_mentions={0}" -f $globalContainerCount)

Write-Output ""
Write-Output "=== P1: atoms upward includes ==="
& rg -n --color never -e $IncludePattern "$Root/atoms"

Write-Output ""
Write-Output "=== P2: singleton calls (top files) ==="
Write-Output "- AtomsTemplate::Instance()"
& rg -n --color never -e "AtomsTemplate::Instance\(" $Root `
| ForEach-Object { ($_ -split ":")[0] } `
| Group-Object `
| Sort-Object Count -Descending `
| Select-Object -First 10 `
| ForEach-Object { "{0}|{1}" -f $_.Name, $_.Count }

Write-Output "- VtkViewer::Instance()"
& rg -n --color never -e "VtkViewer::Instance\(" $Root `
| ForEach-Object { ($_ -split ":")[0] } `
| Group-Object `
| Sort-Object Count -Descending `
| Select-Object -First 10 `
| ForEach-Object { "{0}|{1}" -f $_.Name, $_.Count }

Write-Output "- MeshManager::Instance()"
& rg -n --color never -e "MeshManager::Instance\(" $Root `
| ForEach-Object { ($_ -split ":")[0] } `
| Group-Object `
| Sort-Object Count -Descending `
| Select-Object -First 10 `
| ForEach-Object { "{0}|{1}" -f $_.Name, $_.Count }

Write-Output ""
Write-Output "=== P3: global container references (model_tree/file_loader) ==="
& rg -n --color never -e $GlobalContainerPattern "$Root/model_tree.cpp" "$Root/file_loader.cpp"
