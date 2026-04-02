# Phase 9 Render Inventory Snapshot (W0 Restart Baseline)

기준일: `2026-04-02 (KST)`  
기준 브랜치: `refactor/phase9-render-boundary-complete`  
기준 커밋: `a20401f`  
기준 계획서: `docs/refactoring/phase9/refactoring_phase9_render_boundary_complete_260402.md`

## 1. 핵심 파일 볼륨
| 항목 | 값 |
|---|---:|
| `webassembly/src/vtk_viewer.cpp` lines | 2190 |
| `webassembly/src/vtk_viewer.h` lines | 177 |
| `webassembly/src/render/application/render_gateway.h` lines | 51 |
| `webassembly/src/render/infrastructure/vtk_render_gateway.cpp` lines | 79 |

## 2. `VtkViewer::Instance()` 호출 분포
| 항목 | 값 |
|---|---:|
| 전체(`webassembly/src`, `*.cpp/*.h`) | 119 |
| `webassembly/src/render` 내부 | 10 |
| `render` 외부 | 109 |

상위 호출 파일:
- `webassembly/src/atoms/atoms_template.cpp`: 46
- `webassembly/src/mesh.cpp`: 28
- `webassembly/src/toolbar.cpp`: 11
- `webassembly/src/render/infrastructure/vtk_render_gateway.cpp`: 10
- `webassembly/src/atoms/application/measurement_controller.cpp`: 9
- `webassembly/src/atoms/application/visibility_service.cpp`: 5
- `webassembly/src/file_loader.cpp`: 2
- `webassembly/src/shell/runtime/workbench_runtime.cpp`: 1
- `webassembly/src/mesh_manager.cpp`: 1

## 3. W0 우선 전환 대상(Phase 9 W3 기준)
1. `atoms/atoms_template.cpp`
2. `mesh.cpp`
3. `toolbar.cpp`
4. `atoms/application/measurement_controller.cpp`
5. `atoms/application/visibility_service.cpp`
6. `file_loader.cpp`
7. `mesh_manager.cpp`
8. `shell/runtime/workbench_runtime.cpp`

## 4. 추적 이슈 연계
- `P9-BUG-01`
  - 증상: `XSF(Grid) -> VASP -> XSF(Grid) -> VASP` 순서에서 기존 grid 데이터가 잔존할 수 있음
  - 상태: `Open`
  - 연계 로그: `docs/refactoring/phase9/logs/bug_p9_vasp_grid_sequence_latest.md`

