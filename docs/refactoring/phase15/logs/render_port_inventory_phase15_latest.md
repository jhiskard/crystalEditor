# Phase 15 W0 Render Port Inventory Baseline

- 측정 시각: `2026-04-07 (KST)`
- 범위: `webassembly/src/**/*.cpp`, `webassembly/src/**/*.h`
- 기준 계획서: `docs/refactoring/phase15/refactoring_phase15_render_platform_port_isolation_260407.md`

## 1) VTK actor/volume 토큰 기준선

검색 패턴:
- `vtkActor`
- `vtkVolume`

집계 결과:

| 항목 | Count |
|---|---:|
| 전체 참조 수 | 234 |
| render 모듈 외 참조 수 (`webassembly/src/render/*` 제외) | 201 |

render 외 상위 분포:
1. `webassembly/src/atoms/atoms_template.cpp`: 32
2. `webassembly/src/atoms/atoms_template.h`: 30
3. `webassembly/src/atoms/infrastructure/vtk_renderer.cpp`: 23
4. `webassembly/src/atoms/infrastructure/vtk_renderer.h`: 15
5. `webassembly/src/mesh.h`: 14

## 2) 직접 조작 토큰 기준선

검색 패턴:
- `AddActor(`
- `RemoveActor(`
- `AddVolume(`
- `RemoveVolume(`
- `SetMapper(`
- `GetProperty(`

집계 결과:

| 항목 | Count |
|---|---:|
| 전체 참조 수 | 180 |
| render 모듈 외 참조 수 (`webassembly/src/render/*` 제외) | 172 |

render 외 상위 분포:
1. `webassembly/src/mesh.cpp`: 44
2. `webassembly/src/atoms/infrastructure/vtk_renderer.cpp`: 44
3. `webassembly/src/atoms/atoms_template.cpp`: 29
4. `webassembly/src/atoms/infrastructure/charge_density_renderer.cpp`: 19
5. `webassembly/src/vtk_viewer.cpp`: 10

## 3) 경계 include 기준선

검색 패턴:
- `render/infrastructure`

집계 결과:

| 항목 | Count |
|---|---:|
| 전체 include 참조 수 | 1 |
| render 모듈 외 include 참조 수 | 1 |

render 외 참조 파일:
1. `webassembly/src/shell/runtime/workbench_runtime.cpp` (1)

## 4) W0 메모

1. Phase 15 핵심은 render 책임을 `render/application` 포트 계약 중심으로 수렴시키는 것이다.
2. W1~W3에서 비-render 직접 조작 경로와 경계 include를 단계적으로 감소시킨다.
