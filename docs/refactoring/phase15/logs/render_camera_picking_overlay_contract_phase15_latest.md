# Phase 15 W4 Render 계약 문서 (Camera / Picking / Overlay)

- 작성 시각: `2026-04-07 (KST)`
- 기준 계획서: `docs/refactoring/phase15/refactoring_phase15_render_platform_port_isolation_260407.md`
- 계약 인터페이스: `webassembly/src/render/application/render_gateway.h`

## 1) 계약 목적

1. non-render 계층(`atoms/mesh/io/shell`)이 카메라/오버레이/캡처 기능을 사용할 때 `RenderGateway` 포트만 경유하도록 강제한다.
2. `render/infrastructure` 직접 의존 없이도 기능 동작을 유지하고, 향후 플랫폼/렌더 백엔드 교체 시 영향 범위를 render 모듈 내부로 제한한다.

## 2) Camera/Picking/Overlay 계약 매트릭스

| 범주 | 허용 API (`RenderGateway`) | 주 소비 계층(예시) | 금지 접근 |
|---|---|---|---|
| Camera | `SetProjectionMode`, `AlignCameraToCellAxis`, `AlignCameraToIcellAxis`, `SetArrowRotateStepDeg`, `GetArrowRotateStepDeg`, `GetActiveCamera` | `shell/application/workbench_controller.cpp`, `toolbar.cpp` | non-render에서 `VtkViewer::Instance()` 직접 호출 |
| Picking/Capture | `CaptureActorImage` | `atoms/infrastructure/charge_density_renderer.cpp` | non-render에서 renderer/camera 객체 직접 조작 |
| Overlay | `AddMeasurementOverlayActor`, `RemoveMeasurementOverlayActor`, `AddActor2D`, `RemoveActor2D`, `IsPerformanceOverlayEnabled`, `SetPerformanceOverlayEnabled` | `atoms/atoms_template.cpp`, `atoms/infrastructure/vtk_renderer.cpp`, `shell/application/workbench_controller.cpp` | non-render에서 `vtkRenderer::AddActor2D` 등 직접 호출 |

## 3) 코드 반영 포인트

1. `render_gateway.h`에 계약 범주 주석(Camera/Picking/Overlay)을 추가해 포트 사용 목적을 명시했다.
2. `vtk_render_gateway.h`에 infra 전용 어댑터임을 명시해 non-render에서의 직접 참조를 금지했다.
3. `workbench_runtime.cpp`는 `render/application/legacy_viewer_facade.h` 브리지 경유로 정리되어 `render/infrastructure` include가 제거됐다.

## 4) 검증 규칙

1. 정적 게이트(`check_phase15_render_platform_isolation.ps1`)에서 non-render의 `render/infrastructure` include는 `0`이어야 한다.
2. 메뉴 오픈 회귀 경로(`Edit/Build/Data/Utilities`)는 코드 패턴 기준으로 유지되어야 한다.
3. 본 계약 문서는 W5 게이트에서 존재 여부를 확인한다.
