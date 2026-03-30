# Phase 0 Dependency Baseline Report

기준 시점: `2026-03-27 (KST)`  
기준 커밋: `763ad6417a831fad0b4840d470541ab4e39c841d`

## 1. 위반 패턴 정의
- P1: `atoms` 내부에서 상위 모듈 헤더 직접 include
  - 패턴: `../app.h`, `../vtk_viewer.h`, `../model_tree.h`, `../mesh_detail.h`, `../mesh_manager.h` 및 `../../*` 변형
- P2: direct singleton 호출
  - 패턴: `AtomsTemplate::Instance()`, `VtkViewer::Instance()`, `MeshManager::Instance()`
- P3: 전역 컨테이너 직접 참조
  - 패턴: `atoms::domain::{createdAtoms, createdBonds, surroundingAtoms, surroundingBonds, atomGroups, bondGroups, cellInfo}`

## 2. 기준선 요약
| 항목 | 수치 | 비고 |
|---|---:|---|
| P1 상위 include 위반 | 15 | `webassembly/src/atoms` 기준 |
| P2 `AtomsTemplate::Instance()` | 72 | 전체 파일 기준 |
| P2 `VtkViewer::Instance()` | 216 | 전체 파일 기준 |
| P2 `MeshManager::Instance()` | 84 | 전체 파일 기준 |
| P2 코드 파일(`*.cpp`, `*.h`)만 `AtomsTemplate::Instance()` | 67 | 문서 파일 제외 |
| P2 코드 파일(`*.cpp`, `*.h`)만 `VtkViewer::Instance()` | 199 | 문서 파일 제외 |
| P3 전역 컨테이너 직접 참조(전체) | 52 | `atoms/domain` 포함 전체 검색 |
| P3 전역 컨테이너 직접 참조(핵심 경로) | 6 | `model_tree.cpp`/`file_loader.cpp` |

## 3. 상세 스냅샷

### 3.1 P1: 상위 include 위반 목록(15)
```text
webassembly/src/atoms/atoms_template.cpp:2:#include "../app.h"
webassembly/src/atoms/atoms_template.cpp:3:#include "../vtk_viewer.h"
webassembly/src/atoms/atoms_template.cpp:4:#include "../model_tree.h"
webassembly/src/atoms/atoms_template.cpp:5:#include "../mesh_detail.h"
webassembly/src/atoms/atoms_template.cpp:6:#include "../mesh_manager.h"
webassembly/src/atoms/ui/bz_plot_ui.cpp:5:#include "../../app.h"
webassembly/src/atoms/ui/bravais_lattice_ui.cpp:7:#include "../../app.h"
webassembly/src/atoms/infrastructure/batch_update_system.cpp:4:#include "../../vtk_viewer.h"
webassembly/src/atoms/ui/charge_density_ui.cpp:6:#include "../../mesh_manager.h"
webassembly/src/atoms/ui/charge_density_ui.cpp:7:#include "../../mesh_detail.h"
webassembly/src/atoms/infrastructure/bz_plot_layer.cpp:3:#include "../../vtk_viewer.h"
webassembly/src/atoms/domain/bond_manager.cpp:3:#include "../../vtk_viewer.h"
webassembly/src/atoms/infrastructure/charge_density_renderer.cpp:5:#include "../../vtk_viewer.h"
webassembly/src/atoms/domain/bz_plot.cpp:4:#include "../../vtk_viewer.h"
webassembly/src/atoms/infrastructure/vtk_renderer.h:4:#include "../../vtk_viewer.h"
```

### 3.2 P2: singleton 호출 상위 파일(Top)
#### `AtomsTemplate::Instance()`
```text
webassembly/src/file_loader.cpp|26
webassembly/src/app.cpp|16
webassembly/src/vtk_viewer.cpp|8
webassembly/src/toolbar.cpp|8
webassembly/src/atoms/atoms_template.cpp|5
```

#### `VtkViewer::Instance()`
```text
webassembly/src/atoms/atoms_template.cpp|89
webassembly/src/mesh.cpp|28
webassembly/src/atoms/infrastructure/vtk_renderer.cpp|27
webassembly/src/toolbar.cpp|11
webassembly/src/atoms/infrastructure/charge_density_renderer.cpp|9
```

#### `MeshManager::Instance()`
```text
webassembly/src/mesh_detail.cpp|32
webassembly/src/atoms/atoms_template.cpp|16
webassembly/src/file_loader.cpp|14
webassembly/src/model_tree.cpp|9
webassembly/src/atoms/ui/charge_density_ui.cpp|5
```

### 3.3 P3: 전역 컨테이너 직접 참조(핵심)
```text
webassembly/src/model_tree.cpp:14:using atoms::domain::bondGroups;
webassembly/src/model_tree.cpp:15:using atoms::domain::createdAtoms;
webassembly/src/model_tree.cpp:16:using atoms::domain::createdBonds;
webassembly/src/model_tree.cpp:17:using atoms::domain::surroundingAtoms;
webassembly/src/model_tree.cpp:18:using atoms::domain::surroundingBonds;
webassembly/src/file_loader.cpp:228:if (!atoms::domain::createdAtoms.empty() || !atoms::domain::surroundingAtoms.empty()) {
```

## 4. 재실행 방법
- 스크립트: `scripts/refactoring/check_phase0_dependency_baseline.ps1`
- 실행:
```powershell
powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase0_dependency_baseline.ps1
```

## 5. Phase 1 진입 시 감시 지표
- P1(상위 include) 15 -> 감소 추세 확인
- P3(`model_tree.cpp`의 `using atoms::domain::*`) 제거
- P2 중 `VtkViewer::Instance()` 호출 상위 3개 파일 우선 축소
