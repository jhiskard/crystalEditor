# Phase 2 Dependency Gate Report

기준 시점: `2026-03-30 (KST)`  
기준 문서: `refactoring_phase2_state_store_integration_260330.md`  
규칙 문서: `docs/refactoring/phase1_boundary_rules.md`  
경계 검사 로그: `docs/refactoring/phase2/logs/dependency_check_20260330_1417_phase2.txt`  
빌드 로그: `docs/refactoring/phase2/logs/build_20260330_1414_phase2_gate.txt`

## 1. 정량 결과 (R1~R6)
| 항목 | Baseline | Target | Current | 결과 |
|---|---:|---:|---:|---|
| `R1.model_tree_using` | 0 | 0 | 0 | PASS |
| `R2.direct_vtkviewer` | 0 | 0 | 0 | PASS |
| `R3.include_violation` | 0 | 0 | 0 | PASS |
| `R4.domain_extern_state` | 8 | 0 | 0 | PASS |
| `R5.manager_global_definitions` | 8 | 0 | 0 | PASS |
| `R6.template_duplicate_static` | 2 | 0 | 0 | PASS |

## 2. 검증 명령
```powershell
powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase1_boundaries.ps1
```

## 3. 빌드 결과
- 명령: `npm.cmd run build-wasm:release` (emsdk 환경 로드 포함)
- 결과: `PASS`
- 산출물
  - `public/wasm/VTK-Workbench.js`
  - `public/wasm/VTK-Workbench.wasm`
  - `public/wasm/VTK-Workbench.data`

## 4. 스모크 근거
- 정적 앵커 체크: PASS  
  - Import 경로/핵심 진입점(`OpenStructureFileBrowser`, `ApplyXSFParseResult`, `OpenChgcarFileBrowser`, `ApplyChgcarParseResult`, `SetStructureVisible`, `SetUnitCellVisible`, `EnterMeasurementMode`, `RenderChargeDensityViewerWindow`, `RenderSliceViewerWindow`) 존재 확인
- 런타임 수동 스모크(UI 실조작): `PASS` (`2026-03-30 (KST)`, 사용자 확인: 이상 없음)

## 5. 요약
- Phase 2의 핵심 경계 규칙(R1~R6)은 모두 목표값(`0`)을 달성했다.
- Atom -> Bond -> Cell 순서의 상태 소유권 이전과 `StructureStateStore` 기반 통합이 빌드/검사 기준으로 검증됐다.
- 최종 Go/No-Go는 수동 스모크 PASS 반영으로 확정했다.
