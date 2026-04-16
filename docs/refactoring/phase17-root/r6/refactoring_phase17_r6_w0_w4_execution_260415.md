# Phase17-R6 W0~W4 실행 결과 보고서

작성일: `2026-04-15`
기준 계획서: `docs/refactoring/phase17-root/r6/refactoring_phase17_r6_final_target_graph_legacy_seal_execution_plan_260415.md`

## 0. 실행 범위

본 문서는 R6 세부계획서의 W0~W4 실행 결과를 정리한다.

- W0: 기준선 동결 및 인벤토리 산출
- W1: runtime 조립 경로 전환
- W2: structure/measurement/density 직접 소유 완결
- W3: atoms UI/Presentation 최종 분산
- W4: atoms domain/infrastructure/legacy 잔여 해체

## 1. W0 결과

- 산출물 갱신:
  - `docs/refactoring/phase17-root/r6/logs/r6_inventory_snapshot_latest.md`
- 핵심 지표:
  - `webassembly/src/atoms` 하위 코드 파일: `0`
  - `AtomsTemplateFacade()` 호출: `0`
  - legacy adapter 파일: `0`

## 2. W1 결과 (runtime 경로)

- `WorkbenchRuntime::AtomsTemplateFacade()` 경로 제거 상태 유지.
- runtime/mesh/render/io/shell 경로의 facade accessor 호출을 제거하고 `AtomsTemplate::Instance()` 기반 경로로 정리.
- facade accessor 0건 확인.

## 3. W2 결과 (직접 소유)

- legacy port adapter 3종 제거 완료:
  - `atoms/application/*legacy_port_adapter*`
- `StructureService`, `MeasurementService`, `DensityService`는 legacy adapter 없이 직접 동작하도록 유지.

## 4. W3 결과 (UI/Presentation 이관)

다음 경로로 이관 완료:

- `atoms/presentation/*` -> `shell/presentation/atoms/*`, `density/presentation/*`
- `atoms/ui/*` -> `shell/presentation/atoms/*`, `density/presentation/charge_density_ui.*`
- `atoms/application/*` -> `structure/application/*`, `measurement/application/*`

## 5. W4 결과 (domain/infrastructure/legacy 이관)

다음 경로로 잔여 해체 및 이관 완료:

- `atoms/domain/*` -> `structure/domain/atoms/*`
- `atoms/infrastructure/*` -> `render/infrastructure/atoms/*`
- `atoms/legacy/atoms_template_facade.*` -> `workspace/legacy/atoms_template_facade.*`
- density/measurement 전용 파일은 선행 이관 유지:
  - `density/domain/charge_density.*`
  - `density/infrastructure/charge_density_renderer.*`, `slice_renderer.h`, `rho_file_parser.h`
  - `measurement/domain/measurement_geometry.*`
- IO parser bridge 이관 유지:
  - `io/infrastructure/chgcar_parser.*`, `file_io_manager.*`

결과적으로 `webassembly/src/atoms` 하위 `.cpp/.h` 파일은 0건이다.

## 6. 빌드/테스트 검증

- `npm run build-wasm:release` : PASS
- `npm run test:cpp` : PASS

## 7. 현재 잔여 사항 (W5 이후)

W0~W4 범위 기준으로 코드 파일 해체/이관은 완료되었고,
다음 단계(W5)에서 타깃 그래프(`wb_core/wb_ui/wb_atoms` 해체 및 최종 10개 타깃 전환)를 수행하면 R6 목표 그래프에 수렴한다.
