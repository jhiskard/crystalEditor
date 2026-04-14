# Phase 17-R4 실행 보고서 (W0~W3)

- 작성일: `2026-04-13`
- 기준 계획서
  - `docs/refactoring/refactoring_plan_root_legacy_dismantle_260408_53Codex.md`
  - `docs/refactoring/phase17-root/r4/refactoring_phase17_r4_mesh_dismantle_execution_plan_260413.md`

## 1. 실행 범위

본 보고서는 R4 세부계획의 `W0~W3` 범위만 다룬다.

1. W0: 인벤토리/의존 스냅샷 고정
2. W1: Domain 파일 이동 (`mesh.*`, `mesh_group.*`, `lcrs_tree.*`)
3. W2: Repository core 경로 정렬 (`mesh_manager.*` -> `mesh_repository_core.*`)
4. W3: Presentation 패널 파일 이동 (`model_tree.*`, `mesh_detail.*`, `mesh_group_detail.*`)

## 2. 코드 해체/이관 내역

### 2.1 W0 인벤토리 스냅샷

- 생성:
  - `docs/refactoring/phase17-root/r4/logs/r4_inventory_snapshot_latest.md`
- 포함 데이터:
  - 14개 대상 파일의 착수 시점(HEAD) 라인 수
  - 루트 mesh 헤더 참조 건수
  - `MeshManager::Instance`, `ModelTree::Instance`, `MeshDetail::Instance`, `MeshGroupDetail::Instance` 호출 위치 로그
  - `wb_core.cmake`, `wb_mesh.cmake`의 착수/현재 소스 등록 비교

### 2.2 W1 Domain 이동

- 이동 완료:
  - `webassembly/src/lcrs_tree.*` -> `webassembly/src/mesh/domain/lcrs_tree.*`
  - `webassembly/src/mesh.*` -> `webassembly/src/mesh/domain/mesh_entity.*`
  - `webassembly/src/mesh_group.*` -> `webassembly/src/mesh/domain/mesh_group_entity.*`
- 반영:
  - 연관 include 경로를 모듈 경로 기준으로 정렬
  - `wb_core.cmake`, `wb_mesh.cmake`의 소스 등록 경로 갱신

### 2.3 W2 Repository core 정렬

- 이동 완료:
  - `webassembly/src/mesh_manager.*` -> `webassembly/src/mesh/domain/mesh_repository_core.*`
- 정렬:
  - `mesh/domain/mesh_repository.*`
  - `mesh/application/mesh_command_service.h`
  - `mesh/application/mesh_query_service.h`
- 결과:
  - `mesh/domain|application`에서 루트 mesh 헤더 직접 include 제거

### 2.4 W3 Presentation 패널 이동

- 이동 완료:
  - `webassembly/src/model_tree.*` -> `webassembly/src/mesh/presentation/model_tree_panel.*`
  - `webassembly/src/mesh_detail.*` -> `webassembly/src/mesh/presentation/mesh_detail_panel.*`
  - `webassembly/src/mesh_group_detail.*` -> `webassembly/src/mesh/presentation/mesh_group_detail_panel.*`
- 정렬:
  - `mesh/presentation/model_tree_dialogs.cpp`
  - `mesh/presentation/model_tree_mesh_section.cpp`
  - `mesh/presentation/model_tree_structure_section.cpp`
  - `app.cpp`, `shell/runtime/workbench_runtime.cpp` 포함 참조 경로 전환

## 3. W0~W3 완료 조건 점검

1. 루트 `mesh.*`, `mesh_group.*`, `lcrs_tree.*` 0개: `PASS`
2. 루트 `mesh_manager.*` 0개: `PASS`
3. 루트 `model_tree.*`, `mesh_detail.*`, `mesh_group_detail.*` 0개: `PASS`
4. `wb_core/wb_mesh`에서 legacy root mesh source 제거: `PASS`
5. `mesh/domain|application`의 루트 mesh 헤더 include 0: `PASS`
6. `app.cpp`, `workbench_runtime.cpp`의 루트 mesh 헤더 include 0: `PASS`

## 4. 검증 결과

### 4.1 게이트

1. `check_phase17_root_allowlist.ps1`: PASS
2. `check_phase17_root_ownership_map.ps1`: PASS
3. `check_phase17_facade_contract.ps1`: PASS
4. `check_phase17_r2_shell_ui_migration.ps1`: PASS
5. `check_phase17_r3_render_platform_io_migration.ps1`: PASS

로그:
1. `docs/refactoring/phase17-root/r4/logs/check_phase17_root_allowlist_r4_w0_w3_latest.txt`
2. `docs/refactoring/phase17-root/r4/logs/check_phase17_root_ownership_map_r4_w0_w3_latest.txt`
3. `docs/refactoring/phase17-root/r4/logs/check_phase17_facade_contract_r4_w0_w3_latest.txt`
4. `docs/refactoring/phase17-root/r4/logs/check_phase17_r2_shell_ui_migration_r4_w0_w3_latest.txt`
5. `docs/refactoring/phase17-root/r4/logs/check_phase17_r3_render_platform_io_migration_r4_w0_w3_latest.txt`

### 4.2 빌드/테스트

1. `npm run build-wasm:release`: PASS
2. `npm run test:cpp`: PASS
3. `npm run test:smoke`: PASS

로그:
1. `docs/refactoring/phase17-root/r4/logs/build_phase17_r4_w0_w3_latest.txt`
2. `docs/refactoring/phase17-root/r4/logs/test_cpp_phase17_r4_w0_w3_latest.txt`
3. `docs/refactoring/phase17-root/r4/logs/test_smoke_phase17_r4_w0_w3_latest.txt`

## 5. 결론

- R4 `W0~W3` 범위는 계획서 기준으로 실행 완료.
- 판정: `GO (W4 진행 가능)`
