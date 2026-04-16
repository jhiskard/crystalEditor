# Phase 17-R5 세부 작업계획서 (실질 코드 해체/이관 중심)

작성일: `2026-04-14`  
기준 문서: `docs/refactoring/refactoring_plan_root_legacy_dismantle_260408_53Codex.md`  
아키텍처 준수 기준: `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md`의 `3.1~3.7`  
정책 준수 기준: `docs/refactoring/refactoring_plan_architecture_completion_phase13plus_260406.md`의 주석/Git 정책

## 0. 목적

R5의 목표인 `atoms 원본 해체`를 실제 코드 이관 단위로 실행한다.

핵심 목표:
1. `webassembly/src/atoms/atoms_template.cpp`, `webassembly/src/atoms/atoms_template.h`의 책임을 `structure/measurement/density/shell/render` 경계로 분산하고 루트 파일을 제거한다.
2. `StructureService`, `MeasurementService`, `DensityService`의 `AtomsTemplate` 위임 구조를 제거하고 각 모듈이 자체 저장소/서비스를 직접 소유하도록 전환한다.
3. R4 수행 중 발생했던 Reset/창 관련 오류 수정 내역을 R5 선행 안정화 조건과 회귀 게이트에 반영해 재발을 차단한다.

범위 제외:
1. `webassembly/src/atoms_template_bravais_lattice.cpp`
2. `webassembly/src/atoms_template_periodic_table.cpp`  
위 2개 파일은 실행 경로가 아닌 코드 설명 자료로 분류되어 R5 해체 대상에서 제외한다.

---

## 1. 현재 기준선 (R5 착수 시점)

### 1.1 대상 파일/의존 규모

| 항목 | 현재 값 | 비고 |
|---|---:|---|
| `webassembly/src/atoms/atoms_template.cpp` | 5209 lines | 루트 atoms 구현 핵심 |
| `webassembly/src/atoms/atoms_template.h` | 1340 lines | API/상태/VTK 핸들 집약 |
| `webassembly/src/atoms` 루트 `*.cpp` 수 | 1 | 목표 0 |
| `webassembly/src/atoms` 루트 `*.h` 수 | 1 | 목표 0 |
| `atoms_template.h` include 참조 | 40 | 다중 모듈 경계 침범 |
| `AtomsTemplate::Instance()` 호출 | 17 | composition root 외 잔존 |
| `AtomsTemplateFacade()` 참조 | 39 | runtime 경계 과도 결합 |

### 1.2 R4 Reset/창 오류 수정 반영 기준

R5는 아래 R4 안정화 변경을 **필수 유지 조건**으로 둔다.

1. `shouldApplyInitialLayout` 소모 규칙:
- `workbench_controller.cpp`의 `OpenEditorPanel/OpenBuilderPanel/OpenDataPanel/RequestLayoutPreset`에서 초기 레이아웃 bootstrap 신호를 소모한다.

2. `pendingLayoutPreset` one-shot 규칙:
- `app.cpp`에서 레이아웃 적용 후 `pendingLayoutPreset`를 즉시 `None`으로 소비해 프레임 간 재적용을 막는다.

3. 메뉴 액션 직후 shell/app 동기화 규칙:
- `Edit/Build/Data` 메뉴 액션 직후 `syncShellStateFromStore()`로 동일 프레임 동기화를 수행한다.

4. 회귀 검증 대상:
- `Layout 1/2/3`, `Reset`, `Edit/Build/Data` 창 열기 경로에서 “즉시 초기화/즉시 닫힘/반복 적용” 증상 재발 여부.

### 1.3 현재 경계 위반(해체 필요 포인트)

1. `structure/application/structure_service.*`가 `AtomsTemplate*`에 직접 위임한다.
2. `measurement/application/measurement_service.*`가 `AtomsTemplate::MeasurementMode` 변환 브리지에 직접 의존한다.
3. `density/application/density_service.*`가 `AtomsTemplate` API를 그대로 전달한다.
4. `render/presentation/viewer_window.cpp`, `mesh/presentation/model_tree_*`에서 `AtomsTemplate::Instance()` 직접 호출이 남아 있다.
5. `shell/runtime/workbench_runtime.*`가 `AtomsTemplateFacade()`를 중심으로 feature 서비스를 역참조한다.

---

## 2. 파일 단위 해체/이관 매핑

### 2.1 필수 해체군 (R5 완료 기준에 반드시 포함)

| 원본 파일/책임 | 목표 경로(1차) | 실질 이관 단위 | 완료 기준 |
|---|---|---|---|
| `webassembly/src/atoms/atoms_template.h` | `structure/*`, `measurement/*`, `density/*`, `shell/*` | enum/API/상태 정의 분산 | 루트 `atoms_template.h` 삭제 |
| `webassembly/src/atoms/atoms_template.cpp` | `structure/*`, `measurement/*`, `density/*`, `render/*`, `shell/presentation/*` | 5209 라인 함수군 분해 | 루트 `atoms_template.cpp` 삭제 |
| `atoms/application/structure_lifecycle_service.cpp` | `structure/application/*` | 구조 등록/삭제/정리 use-case 이관 | `AtomsTemplate::` 메서드 정의 제거 |
| `atoms/application/visibility_service.cpp` | `structure/application/*`, `density/application/*` | 구조/원자/결합 가시성 분리 | AtomsTemplate 가시성 캐시 제거 |
| `atoms/application/measurement_controller.cpp` | `measurement/application/*`, `measurement/presentation/*` | 측정 모드/선택/오버레이 제어 분리 | 측정 로직의 AtomsTemplate 의존 제거 |
| `atoms/presentation/builder_windows_controller.cpp`, `editor_windows_controller.cpp` | `shell/presentation/*`, `structure/presentation/*`, `density/presentation/*` | Builder/Editor/Data 창 엔트리 분리 | 창 렌더 함수에서 AtomsTemplate 중심 제어 제거 |

### 2.2 필수 보강군 (R5에서 신규 도입)

| 신규 항목 | 위치 | 목적 |
|---|---|---|
| `StructureCommandService`/`StructureQueryService` | `structure/application` | 등록/삭제/가시성/선택 책임의 모듈 소유화 |
| `MeasurementRepository` 구현체(`.cpp`) + overlay 모델 빌더 | `measurement/domain`, `measurement/application` | 측정 상태/기하 계산/표시 descriptor 분리 |
| `DensityRepository` 구현체(`.cpp`) + grid visibility 정책 | `density/domain`, `density/application` | charge-density 상태/고급 grid 상태 단일 소유 |
| `AtomsPanelRouter`(가칭) | `shell/presentation` | Builder/Editor/Data 메뉴 요청을 feature 서비스 호출로 중재 |
| `check_phase17_r5_atoms_migration.ps1` | `scripts/refactoring` | R5 해체/의존 게이트 자동검사 |
| `check_phase17_r5_shell_layout_reset_contract.ps1` | `scripts/refactoring` | Reset/창 회귀 금지 규칙 검사 |

---

## 3. 함수/책임 해체 내역 (실질 이관 단위)

### 3.1 Shell/창 요청 계열 (R4 안정화 연계)

현재 위치:
1. `atoms_template.cpp:1328~1906` (`RequestBuilderSection`, `RequestEditorSection`, `RequestDataMenu`, `Render*Window`, `Render`, `RenderAdvancedView`)
2. `atoms/presentation/builder_windows_controller.cpp`
3. `atoms/presentation/editor_windows_controller.cpp`

이관:
1. 메뉴 요청 enum(`BuilderSectionRequest`, `EditorSectionRequest`, `DataMenuRequest`)은 `shell/domain` 또는 각 feature application command로 분리.
2. 창 라우팅은 `shell/presentation` 전용 presenter로 이동.
3. R4 안정화 규칙(`shouldApplyInitialLayout`, `pendingLayoutPreset` one-shot, 메뉴 후 즉시 동기화)을 R5 UI 라우터 기본 계약으로 고정.

### 3.2 Structure lifecycle/visibility 계열

현재 위치:
1. `atoms/application/structure_lifecycle_service.cpp` (`RegisterStructure`, `RemoveStructure`, `RemoveUnassignedData`)
2. `atoms/application/visibility_service.cpp` (`SetStructureVisible`, `SetUnitCellVisible`, atom/bond visibility)
3. `atoms_template.cpp:2089~2903` (`LoadXSFFile`, `LoadXSFParsedData`, `resetStructure`, `setBravaisLattice`, `applyAtomChanges`, `applyCellChanges`)

이관:
1. 구조 CRUD/정리 책임을 `structure::application`으로 이동.
2. 구조 가시성/unit-cell 가시성/원자·결합 가시성은 `structure`와 `density` 경계로 분리.
3. 구조 import 적용은 `io/application` -> `structure::application` 호출로 단일화.

### 3.3 Measurement 계열

현재 위치:
1. `atoms/application/measurement_controller.cpp`
2. `atoms_template.cpp:3390~5370` (측정 모드, drag selection, overlay 스타일, 거리/각도/이면각/중심 생성/삭제/가시성)

이관:
1. 측정 상태/목록은 `measurement::domain::MeasurementRepository` 단일 소유.
2. 계산/생성/삭제 use-case는 `measurement::application::MeasurementService`로 이동.
3. overlay actor 직접 제어는 render 포트 호출로 바꾸고, measurement는 descriptor 생성만 수행.

### 3.4 Density 계열

현재 위치:
1. `atoms_template.cpp:1356~2060`, `5373~5557` (advanced grid visibility, view type, CHGCAR load, clear/visible/state sync)
2. `atoms/ui/charge_density_ui.cpp`, `atoms/presentation/isosurface_panel.cpp`, `atoms/presentation/slice_view_panel.cpp`

이관:
1. grid/isosurface/slice 상태는 `density::domain::DensityRepository`로 단일화.
2. `DensityService`의 위임형 구현을 제거하고 자체 상태/규칙 구현으로 교체.
3. UI는 `density/presentation` read model 기반으로 재구성하고 feature 상태 직접 수정 금지.

### 3.5 BZ/격자 타입 감지 계열 (Reset 이슈 재발 방지 연계)

현재 위치:
1. `atoms/ui/bz_plot_ui.cpp` (`renderSpecialPointsTable`에서 `detectLatticeType` 호출)
2. `atoms/domain/special_points.h` (`detectLatticeType` 로그 출력)

이관/정리:
1. 격자 타입 판정은 셀 변경 이벤트 기반 캐시 경로로 이동하고, 프레임 렌더 루프 재계산을 제거한다.
2. BZ 관련 상태는 structure/density 분리 이후에도 shell layout/창 상태와 느슨하게 결합되도록 분리한다.
3. Reset 버튼 동작은 도킹 리셋과 feature 창 상태를 분리해 “초기 상태 Reset 후 반복 초기화” 재발을 방지한다.

### 3.6 Cross-module singleton 제거 계열

현재 위치:
1. `render/presentation/viewer_window.cpp`
2. `mesh/presentation/model_tree_dialogs.cpp`
3. `mesh/presentation/model_tree_structure_section.cpp`
4. `shell/runtime/workbench_runtime.*`

이관:
1. `AtomsTemplate::Instance()` 호출을 feature 서비스 호출로 치환.
2. `WorkbenchRuntime::AtomsTemplateFacade()` 중심 경로를 `StructureFeature/MeasurementFeature/DensityFeature` 직접 조립 경로로 치환.
3. R5 종료 시 `atoms_template.h` include가 `shell/runtime` 외 임시 경로에서도 0이 되도록 정리.

---

## 4. 단계별 WBS (실행 순서 고정)

### W0. 기준선 동결 + R4 안정화 회귀 시나리오 고정

작업:
1. `atoms_template.*` 함수/호출/의존 인벤토리 로그 생성.
2. Reset/창 회귀 시나리오 고정:
- 초기 진입 후 `Reset`
- `Layout 1/2/3` 각각 적용
- `Edit/Build/Data` 메뉴로 창 열기
3. R4 안정화 규칙을 R5 게이트 조건으로 문서화.

완료 기준:
1. `docs/refactoring/phase17-root/r5/logs/r5_inventory_snapshot_latest.md` 생성.
2. `docs/refactoring/phase17-root/r5/logs/r5_shell_layout_regression_baseline_latest.md` 생성.

### W1. Shell 요청 계약 분리

작업:
1. `AtomsTemplate` 전용 요청 enum/함수를 shell/application 계약으로 분리.
2. `WorkbenchController`의 Builder/Editor/Data 요청을 feature service 호출로 치환.
3. `app.cpp` 메뉴 계층에서 `AtomsTemplate` 직접 참조 제거.

완료 기준:
1. `shell/*`에서 `AtomsTemplate::BuilderSectionRequest` 등 타입 의존 0.

### W2. Structure 이관 (구조/셀/가시성)

작업:
1. `RegisterStructure/RemoveStructure/RemoveUnassignedData`를 `structure/application`으로 이동.
2. 구조/셀/원자/결합 가시성 API를 `structure` 모듈 API로 치환.
3. `io/application/import_apply_service.cpp`와 연동해 구조 적용 경로를 `structure` 서비스 호출로 고정.

완료 기준:
1. 구조 lifecycle/visibility가 `AtomsTemplate` 멤버 상태를 직접 수정하지 않음.

### W3. Measurement 이관

작업:
1. `measurement/domain/measurement_repository.cpp` 구현 추가.
2. `measurement_service`에서 mode/목록/가시성/삭제 API를 자체 구현으로 전환.
3. drag selection/overlay descriptor 경로를 `render` 포트 계약으로 분리.

완료 기준:
1. `measurement/application/measurement_service.*`의 `#include "../../atoms/atoms_template.h"` 제거.

### W4. Density 이관

작업:
1. `density/domain/density_repository.cpp` 구현 추가.
2. `DensityService` 위임형 구현 제거 및 advanced-grid 정책 내재화.
3. charge-density UI/패널을 `density/presentation` read model 중심으로 재배치.

완료 기준:
1. `density/application/density_service.*`의 `AtomsTemplate` 위임 코드 0.

### W5. Presentation/BZ 분리 + Reset 안정화 반영

작업:
1. `atoms/presentation/*`, `atoms/ui/*` 중 feature 창 렌더 엔트리를 `shell/presentation`, `structure/density/measurement/presentation`으로 분산.
2. BZ 특수점 테이블의 lattice 타입 판정은 캐시 기반으로 전환.
3. Reset 동작에서 창 열림/도킹 재구성을 분리하고, 메뉴로 연 창이 즉시 닫히지 않도록 상태 전파 순서를 고정.

완료 기준:
1. Reset/창 회귀 시나리오 PASS.
2. `bz_plot_ui` 렌더 루프에서 중복 판정/중복 로그 경로 제거.

### W6. Runtime 조립/호출 경로 정렬

작업:
1. `workbench_runtime.*`에서 `AtomsTemplateFacade()` 의존 경로를 구조/측정/밀도 서비스 직접 조립 경로로 전환.
2. `render/*`, `mesh/presentation/*`, `io/*`의 `AtomsTemplate::Instance()` 호출 제거.
3. 임시 호환 경로가 필요할 경우 Doxygen 임시 경로 주석(추적 ID/제거 phase) 부여.

완료 기준:
1. composition root 외 `AtomsTemplate::Instance()` 호출 0.

### W7. `atoms_template.*` 제거 + CMake 정렬

작업:
1. `webassembly/src/atoms/atoms_template.cpp`, `webassembly/src/atoms/atoms_template.h` 삭제.
2. `wb_atoms.cmake` 소스 목록 재편(해당 구현을 `wb_structure/wb_measurement/wb_density/wb_shell/wb_render`로 분산).
3. 루트 `webassembly/src/atoms` 코드 파일(직계층) 0개 달성.

완료 기준:
1. 루트 atoms 코드 파일 0.
2. `atoms_template.h` include 참조 0.

### W8. 게이트/문서 패키징

작업:
1. 신규 게이트 추가:
- `scripts/refactoring/check_phase17_r5_atoms_migration.ps1`
- `scripts/refactoring/check_phase17_r5_shell_layout_reset_contract.ps1`
2. R5 결과 보고서/의존성 게이트 보고서/`go_no_go_r6.md` 작성.

완료 기준:
1. R5 게이트 PASS.
2. R6 착수 문서 완료.

---

## 5. 코드 변경 파일 목록(예상)

필수 변경:
1. `webassembly/src/app.cpp`
2. `webassembly/src/shell/application/workbench_controller.cpp`
3. `webassembly/src/shell/runtime/workbench_runtime.h`
4. `webassembly/src/shell/runtime/workbench_runtime.cpp`
5. `webassembly/src/structure/application/structure_service.h`
6. `webassembly/src/structure/application/structure_service.cpp`
7. `webassembly/src/measurement/application/measurement_service.h`
8. `webassembly/src/measurement/application/measurement_service.cpp`
9. `webassembly/src/density/application/density_service.h`
10. `webassembly/src/density/application/density_service.cpp`
11. `webassembly/cmake/modules/wb_atoms.cmake`
12. `webassembly/cmake/modules/wb_structure.cmake`
13. `webassembly/cmake/modules/wb_measurement.cmake`
14. `webassembly/cmake/modules/wb_density.cmake`
15. `webassembly/cmake/modules/wb_shell.cmake`
16. `webassembly/cmake/modules/wb_render.cmake`

신규(예상):
1. `webassembly/src/structure/application/structure_command_service.*`
2. `webassembly/src/structure/application/structure_query_service.*`
3. `webassembly/src/measurement/domain/measurement_repository.cpp`
4. `webassembly/src/density/domain/density_repository.cpp`
5. `webassembly/src/shell/presentation/atoms_panel_router.*`
6. `scripts/refactoring/check_phase17_r5_atoms_migration.ps1`
7. `scripts/refactoring/check_phase17_r5_shell_layout_reset_contract.ps1`

삭제(목표):
1. `webassembly/src/atoms/atoms_template.cpp`
2. `webassembly/src/atoms/atoms_template.h`

---

## 6. 검증 계획

### 6.1 자동 검증

1. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_root_allowlist.ps1`
2. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_root_ownership_map.ps1`
3. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_facade_contract.ps1`
4. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_r2_shell_ui_migration.ps1`
5. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_r3_render_platform_io_migration.ps1`
6. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_r4_mesh_migration.ps1`
7. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_mesh_singleton_usage.ps1`
8. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_r5_atoms_migration.ps1` (신규)
9. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_r5_shell_layout_reset_contract.ps1` (신규)
10. `npm run build-wasm:release`
11. `npm run test:cpp`
12. `npm run test:smoke`

### 6.2 수동 회귀 포인트

1. 초기 진입 직후 `Reset` 클릭 시 레이아웃 반복 적용/창 즉시 초기화가 없어야 한다.
2. `Layout 1/2/3`이 서로 다른 도킹 구성을 정확히 적용해야 한다.
3. `Edit`, `Build`, `Data` 하위 메뉴로 연 창이 즉시 닫히지 않아야 한다.
4. `Utilities > Brillouin Zone` 경로에서 BZ 창 동작과 구조 표시 전환이 정상이어야 한다.
5. Model Tree에서 구조/측정/밀도 관련 항목 조작 시 구조/측정/밀도 상태 소유권이 유지되어야 한다.

---

## 7. 리스크 및 대응

| 리스크 | 수준 | 대응 |
|---|---|---|
| `atoms_template` 대형 상태 분해 중 기능 누락 | High | W2/W3/W4를 독립 커밋으로 분리하고 기능 체크리스트 고정 |
| 측정 overlay의 render 책임 분리 실패 | High | measurement는 descriptor만 소유, actor 생성은 render 포트로 강제 |
| Reset/창 상태 회귀 재발 | High | R5 전용 shell-layout 게이트 + 수동 회귀 시나리오 필수화 |
| `AtomsTemplate` 제거 후 runtime 조립 파손 | High | W6에서 composition root 조립 순서 고정 후 W7 삭제 진행 |
| CMake 타깃 경계 붕괴(`wb_atoms` 과대 유지) | Medium | W7에서 소스 소유권 재배치 + 모듈별 target 검증 |

---

## 8. R5 완료 기준 (실행판)

1. `webassembly/src/atoms` 루트의 코드 파일(`atoms_template.cpp/.h`)이 0개다.
2. `structure/measurement/density` application 서비스에서 `AtomsTemplate` 직접 위임이 0건이다.
3. composition root 외 `AtomsTemplate::Instance()` 호출이 0건이다.
4. `atoms_template.h` include 참조가 0건이다.
5. Reset/창 회귀 시나리오(`Layout 1/2/3`, `Reset`, `Edit/Build/Data`)가 모두 PASS다.
6. `check_phase17_r5_atoms_migration.ps1` PASS.
7. `check_phase17_r5_shell_layout_reset_contract.ps1` PASS.
8. `build-wasm`, `test:cpp`, `test:smoke` PASS.
9. `docs/refactoring/phase17-root/r5/dependency_gate_report.md`, `docs/refactoring/phase17-root/r5/go_no_go_r6.md`가 작성되어 있다.

