# Phase Boundary Rules

기준일: `2026-03-30 (KST)`  
적용 범위: `webassembly/src/model_tree.cpp`, `webassembly/src/atoms/domain`, `webassembly/src/atoms/infrastructure`, `webassembly/src/atoms/atoms_template.cpp`

## 1. Phase 1 규칙 (R1~R3)
- `R1.model_tree_using`: `model_tree.cpp`에서 `using atoms::domain::{createdAtoms, createdBonds, surroundingAtoms, surroundingBonds, bondGroups}` 사용 금지
- `R2.direct_vtkviewer`: `atoms/domain`, `atoms/infrastructure`에서 `VtkViewer::Instance()` 직접 호출 금지
- `R3.include_violation`: `atoms/domain`, `atoms/infrastructure`에서 상위 모듈 직접 include 금지
  - 금지 헤더: `app.h`, `vtk_viewer.h`, `model_tree.h`, `mesh_detail.h`, `mesh_manager.h`

## 2. Phase 2 확장 규칙 (R4~R6)
- `R4.domain_extern_state`: `atoms/domain/*.h`에서 상태 컨테이너 `extern` 선언 금지
  - 대상: `createdAtoms`, `surroundingAtoms`, `atomGroups`, `createdBonds`, `surroundingBonds`, `bondGroups`, `cellInfo`, `cellVisible`, `cellEdgeActors`
- `R5.manager_global_definitions`: `atom_manager.cpp`, `bond_manager.cpp`, `cell_manager.cpp`에서 전역 상태 실체 정의 금지
  - 대상: Atom/Bond/Cell 상태 컨테이너 및 `surroundingsVisible`, `nextAtomId`
- `R6.template_duplicate_static`: `atoms_template.cpp`에서 중복 상태 static 금지
  - 대상: `static bool surroundingsVisible`, `static uint32_t nextBondId`

## 3. 검증 커맨드
```bash
rg -n 'using atoms::domain::(createdAtoms|createdBonds|surroundingAtoms|surroundingBonds|bondGroups)' webassembly/src/model_tree.cpp
rg -n -P '^(?!\s*//).*VtkViewer::Instance\(' webassembly/src/atoms/domain webassembly/src/atoms/infrastructure
rg -n '#include ".*\.\./(app|vtk_viewer|model_tree|mesh_detail|mesh_manager)\.h"|#include ".*\.\./\.\./(app|vtk_viewer|model_tree|mesh_detail|mesh_manager)\.h"' webassembly/src/atoms/domain webassembly/src/atoms/infrastructure
rg -n -P --glob '*.h' '^(?!\s*//).*extern\s+(std::vector<atoms::domain::AtomInfo>|std::vector<atoms::domain::BondInfo>|std::map<std::string,\s*atoms::domain::AtomGroupInfo>|std::map<std::string,\s*atoms::domain::BondGroupInfo>|CellInfo\s+cellInfo|bool\s+cellVisible|std::vector<vtkSmartPointer<vtkActor>>\s+cellEdgeActors)\b' webassembly/src/atoms/domain
rg -n -P '^(?!\s*//)\s*(std::vector<atoms::domain::AtomInfo>\s+createdAtoms|std::vector<atoms::domain::AtomInfo>\s+surroundingAtoms|std::map<std::string,\s*atoms::domain::AtomGroupInfo>\s+atomGroups|std::vector<atoms::domain::BondInfo>\s+createdBonds|std::vector<atoms::domain::BondInfo>\s+surroundingBonds|std::map<std::string,\s*atoms::domain::BondGroupInfo>\s+bondGroups|CellInfo\s+cellInfo|bool\s+cellVisible|std::vector<vtkSmartPointer<vtkActor>>\s+cellEdgeActors|static\s+bool\s+surroundingsVisible|static\s+uint32_t\s+nextAtomId)\b' webassembly/src/atoms/domain/atom_manager.cpp webassembly/src/atoms/domain/bond_manager.cpp webassembly/src/atoms/domain/cell_manager.cpp
rg -n -P '^(?!\s*//)\s*static\s+(bool\s+surroundingsVisible|uint32_t\s+nextBondId)\b' webassembly/src/atoms/atoms_template.cpp
```

## 4. 자동 검사 스크립트
- `scripts/refactoring/check_phase1_boundaries.ps1`

실행:
```powershell
powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase1_boundaries.ps1
```

## 5. 판정 기준
- R1~R6 모두 `count = 0`이어야 `PASS`
- 스크립트는 위반이 1개 이상이면 exit code `1`을 반환
