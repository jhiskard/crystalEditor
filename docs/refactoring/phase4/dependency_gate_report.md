# Phase 4 의존성 게이트 리포트

작성일: `2026-03-30 (KST)`  
기준 계획: `docs/refactoring/phase4/refactoring_phase4_ui_renderer_split_260330.md`  
작업 브랜치: `refactor/phase4-ui-render-split`

근거 로그:
- 정적 분리 게이트: `docs/refactoring/phase4/logs/dependency_check_phase4_latest.txt`
- 경계 규칙 게이트: `docs/refactoring/phase4/logs/boundary_check_phase4_latest.txt`
- 빌드 게이트: `docs/refactoring/phase4/logs/build_phase4_gate_latest.txt`
- 수동 스모크: `docs/refactoring/phase4/logs/manual_smoke_phase4_latest.md`

## 1. 필수 게이트 결과 (Must Pass)
| 항목 | 현재값 | 목표값 | 결과 |
|---|---:|---:|---|
| `P4.required_files_exist` | missing 0건 | 0 | PASS |
| `P4.atoms_moved_defs_removed_from_origin` | 0 | 0 | PASS |
| `P4.atoms_moved_defs_present_in_new_files` | 20 | 20 | PASS |
| `P4.charge_moved_defs_removed_from_origin` | 0 | 0 | PASS |
| `P4.charge_moved_defs_present_in_new_files` | 6 | 6 | PASS |
| `P4.model_tree_moved_defs_removed_from_origin` | 0 | 0 | PASS |
| `P4.model_tree_moved_defs_present_in_new_files` | 5 | 5 | PASS |
| `P4.viewer_moved_defs_removed_from_origin` | 0 | 0 | PASS |
| `P4.viewer_moved_defs_present_in_new_files` | 3 | 3 | PASS |
| `P4.cmake_wiring_entries` | 12 | 12 | PASS |
| `R1~R6 boundary checks` | 위반 0건 | 0 | PASS |

## 2. 권고 지표 (Recommended)
| 항목 | 기준선 | 목표 | 현재 | 결과 |
|---|---:|---:|---:|---|
| `atoms_template.cpp` lines | 6,130 | <= 4,291 | 5,913 | FAIL (권고) |
| `model_tree.cpp` lines | 1,782 | <= 1,336 | 42 | PASS (권고) |
| `charge_density_ui.cpp` lines | 2,260 | <= 1,695 | 1,764 | FAIL (권고) |
| `vtk_viewer.cpp` lines | 2,449 | <= 1,959 | 2,190 | FAIL (권고) |

## 3. 런타임 게이트
- Release 빌드 검증: `PASS`
  - 명령: `cmd /d /c "set EMSDK_QUIET=1&& call ..\emsdk\emsdk_env.bat && npm.cmd run build-wasm:release"`
  - 근거: `docs/refactoring/phase4/logs/build_phase4_gate_latest.txt`
- 수동 스모크 검증: `PASS`
  - 범위: import/measurement/visibility/charge density/camera align
  - 결과: Phase 4 변경 이후 이상 없음
  - 근거: `docs/refactoring/phase4/logs/manual_smoke_phase4_latest.md`

## 4. 요약
- Phase 4 필수 정적 게이트는 `PASS`이다.
- 런타임 게이트(빌드/수동 스모크)도 `PASS`이다.
- Phase 종료 판정은 `GO` 근거를 충족한다.
