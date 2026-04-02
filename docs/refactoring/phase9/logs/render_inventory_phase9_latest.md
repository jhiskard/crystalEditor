# Phase 9 Render Inventory Snapshot

기준일: `2026-04-02 (KST)`  
기준 브랜치: `refactor/phase8-atoms-template-dismantle` (Phase 9 착수 전 기준선)

## 1. 핵심 파일 볼륨
| 항목 | 값 |
|---|---:|
| `webassembly/src/vtk_viewer.cpp` lines | 1995 |
| `webassembly/src/vtk_viewer.h` lines | 155 |
| `webassembly/src/render/application/render_gateway.h` lines | 38 |

## 2. `VtkViewer::Instance()` 호출 분포
| 항목 | 값 |
|---|---:|
| 전체(`webassembly/src`) | 119 |
| `webassembly/src/render` 내부 | 10 |
| `render` 외부 | 109 |

상위 호출 파일:
- `webassembly/src/atoms/atoms_template.cpp`: 46
- `webassembly/src/mesh.cpp`: 28
- `webassembly/src/toolbar.cpp`: 11
- `webassembly/src/render/infrastructure/vtk_render_gateway.cpp`: 10
- `webassembly/src/atoms/application/measurement_controller.cpp`: 9
- `webassembly/src/atoms/application/visibility_service.cpp`: 5

## 3. Phase 9 추적 이슈
- `P9-BUG-01`
  - 증상: `XSF(Grid) -> VASP -> XSF(Grid) -> VASP` 순서에서 기존 grid 데이터가 잔존할 수 있음
  - 상태: 미해결(2026-04-02 기준), Phase 9 W0/W6에서 재현/판정 로그 유지
