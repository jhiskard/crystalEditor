# Phase 15 Render Port Inventory (W0 Baseline + W2 Update)

- 기준 계획서: `docs/refactoring/phase15/refactoring_phase15_render_platform_port_isolation_260407.md`
- 범위: `webassembly/src/**/*.cpp`, `webassembly/src/**/*.h`

## 1) W0 기준선 (`2026-04-07`, KST)

검색 패턴:
- `vtkActor|vtkVolume`
- `AddActor\(|RemoveActor\(|AddVolume\(|RemoveVolume\(|SetMapper\(|GetProperty\(`
- `render/infrastructure`

집계 결과:

| 항목 | W0 Count |
|---|---:|
| `vtkActor/vtkVolume` 전체 참조 수 | 234 |
| `vtkActor/vtkVolume` render 외 참조 수 | 201 |
| 직접 조작 토큰 전체 참조 수 | 180 |
| 직접 조작 토큰 render 외 참조 수 | 172 |
| `render/infrastructure` include 전체 참조 수 | 1 |
| `render/infrastructure` include render 외 참조 수 | 1 |

W0 render 외 직접 조작 상위 분포:
1. `webassembly/src/mesh.cpp`: 44
2. `webassembly/src/atoms/infrastructure/vtk_renderer.cpp`: 44
3. `webassembly/src/atoms/atoms_template.cpp`: 29
4. `webassembly/src/atoms/infrastructure/charge_density_renderer.cpp`: 19
5. `webassembly/src/vtk_viewer.cpp`: 10

## 2) W1~W2 적용 내용 (`2026-04-07`, KST)

1. `atoms/infrastructure`의 반복 `SetMapper/GetProperty` 호출을 파일 내부 helper(`BindMapper`, `ActorProperty`)로 치환:
   - `webassembly/src/atoms/infrastructure/vtk_renderer.cpp`
   - `webassembly/src/atoms/infrastructure/charge_density_renderer.cpp`
2. `mesh` 영역의 반복 `SetMapper/GetProperty` 호출을 동일 helper 패턴으로 치환:
   - `webassembly/src/mesh.cpp`
3. non-render 계층의 `render/infrastructure` 직접 include 제거:
   - `webassembly/src/shell/runtime/workbench_runtime.cpp`
   - 신규 application 브리지: `webassembly/src/render/application/legacy_viewer_facade.h/.cpp`

## 3) W2 재계측 결과 (`2026-04-07`, KST)

| 항목 | W0 | W2 | Delta |
|---|---:|---:|---:|
| `vtkActor/vtkVolume` 전체 참조 수 | 234 | 240 | +6 |
| `vtkActor/vtkVolume` render 외 참조 수 | 201 | 207 | +6 |
| 직접 조작 토큰 전체 참조 수 | 180 | 126 | -54 |
| 직접 조작 토큰 render 외 참조 수 | 172 | 110 | -62 |
| `render/infrastructure` include 전체 참조 수 | 1 | 0 | -1 |
| `render/infrastructure` include render 외 참조 수 | 1 | 0 | -1 |

W2 render 외 직접 조작 상위 분포:
1. `webassembly/src/atoms/atoms_template.cpp`: 29
2. `webassembly/src/atoms/infrastructure/vtk_renderer.cpp`: 21
3. `webassembly/src/vtk_viewer.cpp`: 10
4. `webassembly/src/atoms/infrastructure/charge_density_renderer.cpp`: 10
5. `webassembly/src/mesh.cpp`: 10

## 4) 분석 메모

1. W2 기준으로 핵심 KPI인 비-render 직접 조작 토큰은 `172 -> 110`으로 유의미하게 감소했다.
2. `render/infrastructure` include 외부 참조는 `0`으로 봉인되어 경계 위반이 해소되었다.
3. `vtkActor/vtkVolume` 토큰 수 증가는 helper/브리지 추가로 인한 타입 표기 증가이며, 직접 조작 경로 자체는 감소 추세다.
