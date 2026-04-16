# Phase 17-R4 세부 작업계획서 (실질 코드 해체/이관 중심)

작성일: `2026-04-13`  
기준 문서: `docs/refactoring/refactoring_plan_root_legacy_dismantle_260408_53Codex.md`  
아키텍처 준수 기준: `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md`의 `3.1~3.7`  
정책 준수 기준: `docs/refactoring/refactoring_plan_architecture_completion_phase13plus_260406.md`의 주석/Git 정책

## 0. 목적

R4의 목표인 `Mesh 파일군 이관`을 실제 코드 해체 단위로 실행한다.

핵심 목표:
1. 루트 파일 `mesh.*`, `mesh_group.*`, `lcrs_tree.*`, `mesh_manager.*`, `mesh_detail.*`, `mesh_group_detail.*`, `model_tree.*`의 책임을 `mesh/domain`, `mesh/application`, `mesh/presentation`으로 분산한다.
2. `MeshManager::Instance()` 중심 경로를 repository/service + runtime 조립 경로로 치환한다.
3. `app.cpp`, `shell/runtime`, `render/presentation`, `atoms/*`에서 루트 mesh 헤더 직접 include를 제거하고 모듈 경계 경로를 고정한다.

전제:
1. R2/R3 이관으로 `shell/presentation`, `render/presentation`, `io/application` 경계는 이미 부분 정렬되었다.
2. `atoms_template_bravais_lattice.cpp`, `atoms_template_periodic_table.cpp`는 설명 자료이며 실행 경로 해체 대상이 아니다.

---

## 1. 현재 기준선 (R4 착수 시점)

### 1.1 대상 파일 규모

| 파일 | 라인 수 | 현 위치 | 주 책임 |
|---|---:|---|---|
| `mesh.cpp` | 1025 | `webassembly/src` 루트 | VTK actor/volume 파이프라인 + 렌더 요청 |
| `mesh.h` | 218 | `webassembly/src` 루트 | Mesh 엔티티 + VTK 핸들 소유 |
| `mesh_group.cpp` | 118 | `webassembly/src` 루트 | Group actor 생성/표시 |
| `mesh_group.h` | 62 | `webassembly/src` 루트 | MeshGroup 엔티티 |
| `lcrs_tree.cpp` | 296 | `webassembly/src` 루트 | 트리 삽입/삭제/순회 |
| `lcrs_tree.h` | 93 | `webassembly/src` 루트 | TreeNode/LcrsTree 타입 |
| `mesh_manager.cpp` | 496 | `webassembly/src` 루트 | repository + command + UI 연동 혼재 |
| `mesh_manager.h` | 103 | `webassembly/src` 루트 | singleton 진입점 + 공유 설정 |
| `mesh_detail.cpp` | 1277 | `webassembly/src` 루트 | Mesh detail UI + volume 제어 |
| `mesh_detail.h` | 164 | `webassembly/src` 루트 | detail panel API + UI 캐시 상태 |
| `mesh_group_detail.cpp` | 299 | `webassembly/src` 루트 | group detail UI |
| `mesh_group_detail.h` | 48 | `webassembly/src` 루트 | group detail panel API |
| `model_tree.cpp` | 44 | `webassembly/src` 루트 | model tree 창 진입 |
| `model_tree.h` | 44 | `webassembly/src` 루트 | model tree panel API |

### 1.2 현재 혼재/위반 포인트

1. 루트 mesh 헤더 include가 다수 경로에 잔존한다.  
`mesh_manager.h`(11), `mesh_detail.h`(10), `model_tree.h`(7), `lcrs_tree.h`(7), `mesh.h`(7)
2. `MeshManager::Instance()` 직접 호출이 `atoms/*`, `render/presentation/viewer_window.cpp`, `workbench_runtime.cpp` 등 다수 경로에 남아 있다.
3. `mesh.cpp`가 `app.h`, `shell/runtime/workbench_runtime.h`, `viewer_toolbar_presenter.h`를 직접 include하여 domain 책임과 shell/render 의존이 결합되어 있다.
4. `mesh_manager.cpp`가 `MeshDetail::Instance()`를 호출해 application 계층이 presentation 상태를 직접 참조한다.
5. `model_tree`/`mesh_detail`/`mesh_group_detail`는 runtime-owned 객체화가 부분 적용됐지만 타입 정의/구현 파일이 루트에 남아 경계가 봉인되지 않았다.

---

## 2. 파일 단위 해체/이관 매핑

## 2.1 필수 해체군 (R4 완료 기준에 반드시 포함)

| 원본 파일 | 목표 경로(1차) | 실질 해체 단위 | 완료 기준 |
|---|---|---|---|
| `webassembly/src/mesh.h/.cpp` | `webassembly/src/mesh/domain/mesh_entity.h/.cpp` | Mesh 데이터/렌더 상태 모델, 볼륨 설정, mesh-group 연결 | 루트 `mesh.*` 삭제 |
| `webassembly/src/mesh_group.h/.cpp` | `webassembly/src/mesh/domain/mesh_group_entity.h/.cpp` | Group 엔티티 + group actor 설정 | 루트 `mesh_group.*` 삭제 |
| `webassembly/src/lcrs_tree.h/.cpp` | `webassembly/src/mesh/domain/lcrs_tree.h/.cpp` | TreeNode/LCRS 트리 자료구조 | 루트 `lcrs_tree.*` 삭제 |
| `webassembly/src/mesh_manager.h/.cpp` | `webassembly/src/mesh/domain/mesh_repository_core.*` + `webassembly/src/mesh/application/mesh_*_service.*` | repository/command/query/volume-shared 설정 분해 | 루트 `mesh_manager.*` 삭제 |
| `webassembly/src/model_tree.h/.cpp` | `webassembly/src/mesh/presentation/model_tree_panel.h/.cpp` | model tree 창 엔트리 + 하위 section 연결 | 루트 `model_tree.*` 삭제 |
| `webassembly/src/mesh_detail.h/.cpp` | `webassembly/src/mesh/presentation/mesh_detail_panel.h/.cpp` | mesh detail 렌더 + volume UI 제어 | 루트 `mesh_detail.*` 삭제 |
| `webassembly/src/mesh_group_detail.h/.cpp` | `webassembly/src/mesh/presentation/mesh_group_detail_panel.h/.cpp` | mesh group detail 렌더/편집 | 루트 `mesh_group_detail.*` 삭제 |

## 2.2 필수 보강군 (R4에서 신규 도입)

| 신규 항목 | 위치 | 목적 |
|---|---|---|
| `MeshRepositoryCore` | `mesh/domain` | `id->mesh map`, `LcrsTree`, XSF 구조 ID 소유를 domain으로 고정 |
| `MeshVisibilityCommandService` | `mesh/application` | show/hide/display mode 제어를 presentation에서 분리 |
| `MeshVolumeSettingsService` | `mesh/application` | 공유 volume 설정 생성/동기화 책임 분리 |
| `MeshSelectionSyncService` | `mesh/application` | model tree 선택 상태와 workspace 상태 동기화 |
| `mesh/presentation/*_panel` 타입 | `mesh/presentation` | 루트 panel 타입 제거 및 runtime 생성자 주입 기준 고정 |
| `check_phase17_r4_mesh_migration.ps1` | `scripts/refactoring` | 루트 mesh 파일 잔존/의존/싱글턴 호출 검증 자동화 |

---

## 3. 함수/책임 해체 내역 (실질 이관 단위)

### 3.1 `mesh.h/.cpp` 해체 단위

1. 엔티티 생성/ID/이름/표시 상태:
`Mesh::New`, `GenerateNewId`, `SetDisplayMode`, visibility setter/getter  
-> `mesh/domain/mesh_entity.*`

2. 볼륨 렌더 설정/데이터 범위:
`SetVolumeRenderMode`, `SetVolumeColorPreset`, `SetVolumeWindowLevel`, `SetVolumeQuality`, `GetVolumeDataRange` 계열  
-> `mesh/domain/mesh_entity.*` + `mesh/application/mesh_volume_settings_service.*`

3. VTK actor/mapper 파이프라인:
`createMeshMapper`, `createMeshActor`, `ensureVolumeMeshPipeline`, `ensureVolumeRenderPipeline`  
-> `mesh/domain`에 두지 않고 `render` 경계 호출 지점을 분리한다.  
R4에서는 `RenderGateway` 호출을 application 경유로 이동하고, R6 이전까지 VTK 타입 잔존 최소화.

4. shell/runtime 의존 제거:
`GetWorkbenchRuntime().ToolbarPanel().GetMeshDisplayMode()`, `App::DevicePixelRatio()` 직접 호출  
-> `mesh/application` 설정 조회 서비스/주입값으로 치환.

### 3.2 `mesh_group.h/.cpp` 해체 단위

1. 그룹 엔티티 기본 API:
`MeshGroup::New`, `GetMeshGroupTypeStr`, `SetVisibility`, `SetPointSize`, `SetGroupColor`  
-> `mesh/domain/mesh_group_entity.*`

2. group actor 준비:
`createGroupMapper`, `createGroupActor`  
-> domain 보관 상태와 render 반영 책임을 분리한다(생성/반영 경계 명시).

### 3.3 `lcrs_tree.h/.cpp` 해체 단위

1. 트리 구조/순회:
`InsertItem`, `DeleteItem`, `TraverseTree`, `TraverseTreeMutable`  
-> `mesh/domain/lcrs_tree.*`

2. ID 조회/무결성:
`GetTreeNodeById`, `GetTreeNodeByIdMutable`, `isExistingId`  
-> repository core에서만 사용하도록 접근 경로를 제한한다.

### 3.4 `mesh_manager.h/.cpp` 해체 단위

1. 저장소 소유 책임:
`m_MeshTree`, `m_IdToMeshMap`, `m_XsfStructureIds`, `GetMeshById*`  
-> `mesh/domain/mesh_repository_core.*`

2. command 책임:
`InsertMesh`, `DeleteMesh`, `ShowMesh`, `HideMesh`, `SetDisplayMode`, `SetAllDisplayMode`  
-> `mesh/application/mesh_command_service.*` 및 `mesh/application/mesh_visibility_command_service.*`

3. volume shared 설정 책임:
`SetSharedVolumeDisplaySettings`, `EnsureSharedVolumeDisplaySettingsFromMesh`, `ApplySharedVolumeDisplaySettingsToAllMeshes`, `GetGlobalVolumeDataRange`  
-> `mesh/application/mesh_volume_settings_service.*`

4. XSF 구조 등록 책임:
`RegisterXsfStructure`, `DeleteXsfStructure`, `DeleteAllXsfStructures`  
-> `mesh/application/mesh_structure_link_service.*` (또는 기존 command service 확장)

5. presentation 역참조 제거:
`ShowMesh` 내부 `MeshDetail::Instance()` 직접 참조 제거  
-> UI state는 panel 내부 상태로 한정하고 command 서비스는 순수 명령만 수행.

### 3.5 `model_tree.h/.cpp` + `mesh/presentation/model_tree_*.cpp` 해체 단위

1. 패널 엔트리:
`ModelTree::Render`  
-> `mesh/presentation/model_tree_panel.*`

2. mesh 섹션 렌더:
`renderMeshTable`, `renderMeshTree`  
-> `mesh/presentation/model_tree_mesh_section.*` 유지, 타입 참조를 신규 panel 헤더로 전환

3. 구조/밀도 섹션 렌더:
`renderXsfStructureTable`  
-> R4에서는 파일 경로/타입 종속성만 정렬하고, atoms 세부 분해는 R5에서 완료한다.

4. 확인 팝업:
`renderDeleteConfirmPopup`, `renderClearMeasurementsConfirmPopup`  
-> `mesh/presentation/model_tree_dialogs.*`

### 3.6 `mesh_detail.h/.cpp` 해체 단위

1. 패널 엔트리:
`Render`, `RenderVolumeControls`  
-> `mesh/presentation/mesh_detail_panel.*`

2. volume 제어 UI:
`renderTableRowVolume*`, `syncVolumeUiFromMesh`, `syncSharedVolumeUiFromSettings`  
-> panel 내부로 유지하되 상태 적용은 application service를 통해 반영

3. static changed flag:
`s_Has*Changed` 계열  
-> panel 인스턴스 상태로 축소하거나 workspace context로 이관해 static 전역 의존 축소

### 3.7 `mesh_group_detail.h/.cpp` 해체 단위

1. 패널 엔트리:
`Render`  
-> `mesh/presentation/mesh_group_detail_panel.*`

2. group 편집 UI:
`renderTableRowPointSize`, `renderTableRowGroupColor`  
-> group command service를 경유하도록 교체

3. singleton accessor 제거:
`MeshGroupDetail::Instance()`  
-> runtime 소유 객체 직접 참조(`WorkbenchRuntime`)로 고정

---

## 4. 단계별 WBS (실행 순서 고정)

### W0. 인벤토리/의존 스냅샷 고정

작업:
1. 14개 대상 파일 라인 수 및 include 참조 스냅샷 작성
2. `MeshManager::Instance`, `ModelTree::Instance`, `MeshDetail::Instance`, `MeshGroupDetail::Instance` 호출 위치 로그 고정
3. CMake(`wb_core.cmake`, `wb_mesh.cmake`)의 루트 소스 등록 목록 스냅샷 생성

완료 기준:
1. `docs/refactoring/phase17-root/r4/logs/r4_inventory_snapshot_latest.md` 생성

### W1. Domain 파일 이동 (tree/entity)

작업:
1. `lcrs_tree.*` -> `mesh/domain/lcrs_tree.*` 이동
2. `mesh.*` -> `mesh/domain/mesh_entity.*` 이동
3. `mesh_group.*` -> `mesh/domain/mesh_group_entity.*` 이동
4. include 경로를 루트 기준에서 모듈 기준으로 전환

완료 기준:
1. 루트 `mesh.*`, `mesh_group.*`, `lcrs_tree.*` 0개
2. `wb_core/wb_mesh`가 신규 경로를 빌드에 사용

### W2. MeshManager 분해 (repository + service)

작업:
1. `mesh_manager.*`를 `mesh/domain/mesh_repository_core.*` + application 서비스군으로 분해
2. `mesh/domain/mesh_repository.cpp`의 `MeshManager` wrapper를 제거하고 repository core 직결로 전환
3. `MeshDetail` 역참조 제거(Show/Hide 경로에서 UI 상태 직접 접근 금지)

완료 기준:
1. 루트 `mesh_manager.*` 0개
2. `mesh/domain|application`에서 루트 mesh 헤더 include 0

### W3. Presentation 패널 이동

작업:
1. `model_tree.*` -> `mesh/presentation/model_tree_panel.*`
2. `mesh_detail.*` -> `mesh/presentation/mesh_detail_panel.*`
3. `mesh_group_detail.*` -> `mesh/presentation/mesh_group_detail_panel.*`
4. 기존 `mesh/presentation/model_tree_*_section.cpp`의 include를 신규 panel 경로로 정렬

완료 기준:
1. 루트 `model_tree.*`, `mesh_detail.*`, `mesh_group_detail.*` 0개
2. `app.cpp`/`workbench_runtime.cpp` include가 `mesh/presentation/*` 경로만 참조

### W4. Runtime 조립/호출 경로 정렬

작업:
1. `WorkbenchRuntime`의 panel/repository 반환 타입을 신규 mesh 모듈 경로로 전환
2. `app.cpp`의 mesh 패널 렌더 호출 경로를 신규 타입으로 치환
3. `shell/application/workbench_controller.cpp`의 `MeshRepository()` 경로를 application service 계약으로 교체

완료 기준:
1. `shell/runtime/workbench_runtime.cpp`에서 `../../mesh_*.h`, `../../model_tree.h` include 0
2. composition root 외에서 루트 mesh 타입 직접 생성/lookup 0

### W5. Cross-module 참조 정리 (atoms/render/workspace)

작업:
1. `atoms/*`, `render/presentation/viewer_window.cpp` 등에서 루트 mesh 헤더 include를 신규 경로로 치환
2. `viewer_window.cpp`의 interaction LOD에서 `MeshManager::Instance()` 직접 호출을 query/command service 경유로 교체
3. XSF/charge-density 연계 경로의 mesh ID 선택/가시성 동기화를 service API로 통합

완료 기준:
1. 전체 코드베이스에서 `#include "mesh_manager.h"` 등 루트 mesh 헤더 include 0
2. `MeshManager::Instance()` 호출 0 또는 runtime 내부 임시 호환 1개 이하(호환 ID/제거 시점 명시)

### W6. CMake/게이트/문서 패키징

작업:
1. `wb_core.cmake`, `wb_mesh.cmake`의 source 경로를 신규 모듈 경로로 완전 정렬
2. 신규 게이트 도입:
   - `scripts/refactoring/check_phase17_r4_mesh_migration.ps1`
   - `scripts/refactoring/check_phase17_mesh_singleton_usage.ps1`
3. R4 결과보고서/의존 게이트 보고서/`go_no_go_r5.md` 작성

완료 기준:
1. R4 게이트 PASS
2. R5 착수 문서 완료

---

## 5. 코드 변경 파일 목록(예상)

필수 변경:
1. `webassembly/src/app.cpp`
2. `webassembly/src/shell/runtime/workbench_runtime.h`
3. `webassembly/src/shell/runtime/workbench_runtime.cpp`
4. `webassembly/src/shell/application/workbench_controller.cpp`
5. `webassembly/cmake/modules/wb_core.cmake`
6. `webassembly/cmake/modules/wb_mesh.cmake`

신규(예상):
1. `webassembly/src/mesh/domain/mesh_entity.*`
2. `webassembly/src/mesh/domain/mesh_group_entity.*`
3. `webassembly/src/mesh/domain/lcrs_tree.*`
4. `webassembly/src/mesh/domain/mesh_repository_core.*`
5. `webassembly/src/mesh/application/mesh_visibility_command_service.*`
6. `webassembly/src/mesh/application/mesh_volume_settings_service.*`
7. `webassembly/src/mesh/presentation/model_tree_panel.*`
8. `webassembly/src/mesh/presentation/mesh_detail_panel.*`
9. `webassembly/src/mesh/presentation/mesh_group_detail_panel.*`

삭제(목표):
1. `webassembly/src/mesh.cpp`
2. `webassembly/src/mesh.h`
3. `webassembly/src/mesh_group.cpp`
4. `webassembly/src/mesh_group.h`
5. `webassembly/src/lcrs_tree.cpp`
6. `webassembly/src/lcrs_tree.h`
7. `webassembly/src/mesh_manager.cpp`
8. `webassembly/src/mesh_manager.h`
9. `webassembly/src/model_tree.cpp`
10. `webassembly/src/model_tree.h`
11. `webassembly/src/mesh_detail.cpp`
12. `webassembly/src/mesh_detail.h`
13. `webassembly/src/mesh_group_detail.cpp`
14. `webassembly/src/mesh_group_detail.h`

---

## 6. 검증 계획

## 6.1 자동 검증

1. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_root_allowlist.ps1`
2. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_root_ownership_map.ps1`
3. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_facade_contract.ps1`
4. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_r2_shell_ui_migration.ps1`
5. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_r4_mesh_migration.ps1` (신규)
6. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_mesh_singleton_usage.ps1` (신규)
7. `npm run build-wasm:release`
8. `npm run test:cpp`
9. `npm run test:smoke`

## 6.2 수동 회귀 포인트

1. Model Tree에서 mesh 선택/가시성 토글/삭제
2. Mesh Detail의 edge/face/volume 제어 + volume quality/sample distance
3. Mesh Group Detail의 그룹 표시/삭제/point size/color 적용
4. Viewer 상호작용 중 interaction LOD 진입/복귀
5. XSF 구조 항목과 mesh 트리 동기화(삭제/표시/density 뷰 연동)

---

## 7. 리스크 및 대응

| 리스크 | 수준 | 대응 |
|---|---|---|
| `mesh.cpp`의 VTK 로직 분리 중 렌더 회귀 | High | W1/W2를 분리 커밋하고 볼륨 모드 회귀 시나리오를 매 단계 검증 |
| `mesh_manager.cpp` 분해 시 호출 경로 광범위 수정 | High | repository core 우선 이관 후 service API를 단계적으로 치환 |
| `model_tree_structure_section.cpp`의 atoms 결합으로 인한 경계 교착 | High | R4에서 타입/경로 이관 우선, atoms 기능 분해는 R5와 연동 계획으로 고정 |
| `MeshDetail` static flag 상태 누락/오작동 | Medium | panel 인스턴스 상태화 후 기존 동작 비교 로그 추가 |
| CMake 소스 경로 이관 누락 | Medium | W6에서 `wb_core/wb_mesh` source diff 게이트 강제 |

---

## 8. R4 완료 기준 (실행판)

1. 루트 `mesh.*`, `mesh_group.*`, `lcrs_tree.*`, `mesh_manager.*`, `mesh_detail.*`, `mesh_group_detail.*`, `model_tree.*` 파일이 0개다.
2. `app.cpp`, `workbench_runtime.*`, `atoms/*`, `render/presentation/*`에서 루트 mesh 헤더 include가 0개다.
3. `MeshManager` 루트 singleton 경로가 제거되고 repository/service 경로로 치환된다.
4. `check_phase17_r4_mesh_migration.ps1` PASS.
5. `check_phase17_mesh_singleton_usage.ps1` PASS.
6. `build-wasm`, `test:cpp`, `test:smoke` PASS.
7. `docs/refactoring/phase17-root/r4/dependency_gate_report.md`, `docs/refactoring/phase17-root/r4/go_no_go_r5.md`가 작성되어 있다.

