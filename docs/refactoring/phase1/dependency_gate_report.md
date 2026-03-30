# Phase 1 Dependency Gate Report

기준 시점: `2026-03-30 (KST)`  
기준 비교: `docs/refactoring/phase0/dependency_baseline_report.md`  
빌드 로그: `docs/refactoring/phase1/logs/build_20260330_1116_phase1_gate.txt`

## 1. 정량 결과(Phase 1 게이트)
| 항목 | Baseline (Phase 0) | Current (Phase 1) | 결과 |
|---|---:|---:|---|
| `model_tree.cpp`의 `using atoms::domain::*` | 5 | 0 | PASS |
| `atoms/domain + atoms/infrastructure`의 `VtkViewer::Instance()` (주석 제외) | 50 | 0 | PASS |
| `atoms/domain + atoms/infrastructure` 상위 include 위반 | 6 | 0 | PASS |
| `atoms` 전체 상위 include 위반 | 15 | 9 | 감소(참고 지표) |

## 2. 검증 커맨드
```bash
rg -n 'using atoms::domain::(createdAtoms|createdBonds|surroundingAtoms|surroundingBonds|bondGroups)' webassembly/src/model_tree.cpp
rg -n -P '^(?!\s*//).*VtkViewer::Instance\(' webassembly/src/atoms/domain webassembly/src/atoms/infrastructure
rg -n '#include ".*\.\./(app|vtk_viewer|model_tree|mesh_detail|mesh_manager)\.h"|#include ".*\.\./\.\./(app|vtk_viewer|model_tree|mesh_detail|mesh_manager)\.h"' webassembly/src/atoms/domain webassembly/src/atoms/infrastructure
```

## 3. 빌드 결과
- 명령: `npm.cmd run build-wasm:release` (emsdk 환경 로드 후)
- 결과: PASS
- 산출물:
  - `public/wasm/VTK-Workbench.js`
  - `public/wasm/VTK-Workbench.wasm`
  - `public/wasm/VTK-Workbench.data`

## 4. 스모크 검증

### 4.1 스모크 앵커 점검(정적)
| ID | 상태 | 근거 |
|---|---|---|
| S1 | PASS | `FileLoader::OpenStructureFileBrowser`, `FileLoader::ApplyXSFParseResult` |
| S2 | PASS | `FileLoader::OpenXSFGridFileBrowser`, `FileLoader::ApplyXSFGridParseResult` |
| S3 | PASS | `FileLoader::OpenChgcarFileBrowser`, `FileLoader::ApplyChgcarParseResult` |
| S4 | PASS | `ModelTree::Render`, `ModelTree::renderXsfStructureTable` |
| S5 | PASS | `AtomsTemplate::SetStructureVisible`, `SetAtomVisibleById`, `SetBondVisibleById`, `SetBondsVisible` |
| S6 | PASS | `AtomsTemplate::SetUnitCellVisible` |
| S7 | PASS | `AtomsTemplate::EnterMeasurementMode`, `ExitMeasurementMode`, `RemoveMeasurementsByStructure` |
| S8 | PASS | `AtomsTemplate::RenderChargeDensityViewerWindow`, `RenderSliceViewerWindow` |

### 4.2 수동 런타임 스모크(실사용 UI 조작)
- 실행일시: `2026-03-30 (KST)`
- 실행자: 사용자
- 결과: **PASS**
- 판정: 이상 증상 없음(구조 import/트리 토글/가시성/Charge Density/측정 흐름)

## 5. 요약
- Phase 1 경계 게이트 3개 항목은 모두 목표값(`0`) 달성
- 빌드 성공 및 wasm 산출물 갱신 확인
- 수동 런타임 스모크(실사용 UI 조작) PASS 확인
- `atoms` 전체 include 위반은 9건 잔여이며, 계획서 기준 Phase 4 백로그(주로 `atoms_template.cpp`, `atoms/ui/*`)로 이관
