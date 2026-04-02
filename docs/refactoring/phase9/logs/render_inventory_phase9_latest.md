# Phase 9 Render Inventory Snapshot (W0 Restart Baseline)

기준일: `2026-04-02 (KST)`  
기준 브랜치: `refactor/phase9-render-boundary-complete`  
기준 계획: `docs/refactoring/phase9/refactoring_phase9_render_boundary_complete_260402.md`

## 1. 대상 파일 라인 수
| 항목 | 값 |
|---|---:|
| `webassembly/src/vtk_viewer.cpp` lines | 2190 |
| `webassembly/src/vtk_viewer.h` lines | 177 |
| `webassembly/src/render/application/render_gateway.h` lines | 82 |
| `webassembly/src/render/infrastructure/vtk_render_gateway.cpp` lines | 129 |

## 2. `VtkViewer::Instance()` 호출 분포 (W0)
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

## 3. W0 우선 전환 대상 (Phase 9 W3 기준)
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
  - 증상: `XSF(Grid) -> VASP -> XSF(Grid) -> VASP` 순서에서 기존 grid 데이터가 잔존
  - 상태: `Open`
  - 로그: `docs/refactoring/phase9/logs/bug_p9_vasp_grid_sequence_latest.md`

## 5. W3 Update (2026-04-02 KST)
| 항목 | 값 |
|---|---:|
| `VtkViewer::Instance()` 전체(`*.cpp/*.h`) | 27 |
| `render` 내부 호출 | 21 |
| `render` 외부 호출 | 6 |

W3 전환 대상 파일 결과:
- `atoms/atoms_template.cpp`: 0
- `atoms/application/measurement_controller.cpp`: 0
- `atoms/application/visibility_service.cpp`: 0
- `mesh.cpp`: 0
- `mesh_manager.cpp`: 0
- `toolbar.cpp`: 0
- `file_loader.cpp`: 0
- `shell/runtime/workbench_runtime.cpp`: 0

W3 이후 잔여 `render` 외부 호출(정리 대상):
- `webassembly/src/mouse_interactor_style.cpp`: 2
- `webassembly/src/test_window.cpp`: 2
- `webassembly/src/atoms/domain/atom_manager.h`: 2 (주석)

## 6. W4 Update (2026-04-02 KST)
| 항목 | 값 |
|---|---:|
| `VtkViewer::Instance()` 전체(`*.cpp/*.h`) | 24 |
| `render` 내부 호출 | 24 |
| `render` 외부 호출 | 0 |

W4 반영 사항:
- `mouse_interactor_style.cpp`의 wheel interaction LOD 제어를 `RenderGateway` 경유로 전환
- `test_window.cpp`의 actor 추가 경로를 `RenderGateway` 경유로 전환
- `atom_manager.h`의 legacy 주석에서 `VtkViewer::Instance()` 문자열 제거
- `P9-BUG-01` 원인 귀속/수정 반영:
  - `ChargeDensityUI`에서 비-grid 로드 및 clear 경로의 grid 엔트리 정리 누락 보완
