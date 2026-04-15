# Phase 17-R5 실행 보고서 (W4~W6)

- 작성일: `2026-04-14`
- 기준 계획서:
  - `docs/refactoring/refactoring_plan_root_legacy_dismantle_260408_53Codex.md`
  - `docs/refactoring/phase17-root/r5/refactoring_phase17_r5_atoms_dismantle_execution_plan_260414.md`

## 1. 실행 범위

본 보고서는 R5의 `W4~W6` 범위를 다룬다.

1. W4: Density 이관 (서비스 포트/어댑터 정리)
2. W5: Presentation/BZ 분리와 Reset/창 상태 전파 순서 고정
3. W6: Runtime 조립/호출 경로 정렬 (shell/presentation 직접 의존 축소)

## 2. 코드 변경 내역

### 2.1 W4 - Density 포트 전환

적용 파일:
1. `webassembly/src/density/application/density_service_port.h` (신규)
2. `webassembly/src/density/application/density_service.h`
3. `webassembly/src/density/application/density_service.cpp`
4. `webassembly/src/atoms/application/density_service_legacy_port_adapter.h` (신규)
5. `webassembly/src/atoms/application/density_service_legacy_port_adapter.cpp` (신규)
6. `webassembly/src/atoms/atoms_template.h`
7. `webassembly/src/atoms/atoms_template.cpp`
8. `webassembly/cmake/modules/wb_atoms.cmake`

핵심 변경:
1. `DensityService` 생성자 의존성을 `AtomsTemplate*`에서 `DensityServicePort*`로 전환.
2. `atoms` 경계에 `DensityServiceLegacyPortAdapter`를 도입해 기존 동작을 유지하면서 포트 기반 호출로 이관.
3. `atoms_template` 내부에서 density 포트를 소유하고 `DensityService`를 포트로 조립.

완료 기준 확인:
1. `density/application`에서 `atoms_template` 직접 include/직접 타입 의존: `0건`.

### 2.2 W5 - Shell 패널 요청 큐 + BZ 캐시

적용 파일:
1. `webassembly/src/shell/domain/shell_state_store.h`
2. `webassembly/src/shell/application/shell_state_command_service.h`
3. `webassembly/src/shell/application/shell_state_command_service.cpp`
4. `webassembly/src/shell/application/workbench_controller.cpp`
5. `webassembly/src/app.cpp`
6. `webassembly/src/mesh/presentation/model_tree_structure_section.cpp`
7. `webassembly/src/atoms/ui/bz_plot_ui.h`
8. `webassembly/src/atoms/ui/bz_plot_ui.cpp`

핵심 변경:
1. `ShellUiState`에 one-shot 패널 요청(`Editor/Builder/Data`) 큐 필드를 추가.
2. `WorkbenchController`는 창 visibility/focus만 갱신하고, 실제 섹션 요청은 shell 상태 큐에 기록하도록 변경.
3. `App::renderImGuiWindows()` 초입에서 큐를 소비해 `AtomsTemplate::Request*`를 단일 경로로 실행.
4. `ModelTree`의 데이터 패널 열기 경로를 shell controller 경유로 변경.
5. `BZPlotUI`에 격자 타입/특수점 캐시를 추가하여 셀 행렬 변경 시에만 `detectLatticeType()` 재호출.

완료 기준 확인:
1. `workbench_controller.cpp`에서 `RequestEditorSection/RequestBuilderSection/RequestDataMenu` 직접 호출 제거: `PASS`.
2. `renderSpecialPointsTable()`의 프레임 단위 반복 lattice type 감지 제거(캐시 경로 적용): `PASS`.

### 2.3 W6 - Runtime/호출 경로 정렬

적용 파일:
1. `webassembly/src/structure/application/structure_service_port.h`
2. `webassembly/src/structure/application/structure_service.h`
3. `webassembly/src/structure/application/structure_service.cpp`
4. `webassembly/src/atoms/application/structure_service_legacy_port_adapter.h`
5. `webassembly/src/atoms/application/structure_service_legacy_port_adapter.cpp`
6. `webassembly/src/shell/presentation/toolbar/viewer_toolbar_presenter.cpp`
7. `webassembly/src/shell/runtime/workbench_runtime.cpp`
8. `webassembly/src/shell/application/workbench_controller.cpp`

핵심 변경:
1. `StructureService`에 boundary atom 제어 API를 포트 경유로 추가해 controller의 직접 결합을 축소.
2. `Toolbar`에서 charge-density 조회를 `DensityFeature()` 경유로 이관.
3. `Toolbar`의 BZ 모드 판별을 shell state query 기반으로 전환.
4. `WorkbenchRuntime::StructureFeature/MeasurementFeature/DensityFeature`를 facade 재호출 대신 직접 feature 조립 경로로 정렬.

## 3. 검증 결과

자동 검증:
1. `npm run build-wasm:release`: `PASS`
2. `npm run test:cpp`: `PASS`
3. `npm run test:smoke`: `PASS`

추가 확인:
1. 사용자 지시에 따라 기존 삭제 파일(`imgui.ini`, `atoms_template_*` 문서성 파일 등)은 복원하지 않음.

## 4. 잔여 항목 및 리스크

1. `AtomsTemplateFacade()` 직접 호출은 일부 경로(예: node info, viewer interaction 등)에 여전히 잔존.
2. R5 최종 목표(루트 `atoms_template.*` 제거)는 W7 단계에서 CMake 분리/호출 제거를 포함해 추가 진행이 필요.

## 5. 결론

- R5 W4~W6 범위의 실코드 이관(포트 전환, panel request 라우팅, runtime 경로 정렬)을 반영했다.
- 현재 상태 기준으로 빌드/CPP 테스트/스모크 테스트 모두 통과한다.
