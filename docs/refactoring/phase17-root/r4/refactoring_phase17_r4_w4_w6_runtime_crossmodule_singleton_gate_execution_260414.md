# Phase 17-R4 실행 보고서 (W4~W6)

- 작성일: `2026-04-14`
- 기준 계획서:
  - `docs/refactoring/refactoring_plan_root_legacy_dismantle_260408_53Codex.md`
  - `docs/refactoring/phase17-root/r4/refactoring_phase17_r4_mesh_dismantle_execution_plan_260413.md`

## 1. 실행 범위

본 보고서는 R4의 `W4~W6` 범위를 다룬다.

1. W4: Runtime 조립/호출 경로 정렬
2. W5: Cross-module 참조 정리 (atoms/render/workspace)
3. W6: CMake/게이트/문서 패키징

## 2. 코드 변경 내역

### 2.1 W4 (Runtime 조립/호출 경로 정렬)

적용 파일:
1. `webassembly/src/shell/runtime/workbench_runtime.h`
2. `webassembly/src/shell/runtime/workbench_runtime.cpp`
3. `webassembly/src/shell/application/workbench_controller.cpp`
4. `webassembly/src/app.cpp`

핵심 변경:
1. `WorkbenchRuntime::MeshRepository()` 반환 타입을 `MeshManager`에서 `mesh::domain::MeshRepository`로 전환
2. `workbench_controller`의 표시모드 변경 경로를 `mesh::application::MeshCommandService` 계약으로 이관
3. `app.cpp`의 선택 mesh 조회를 `mesh::application::MeshQueryService`로 이관
4. `workbench_runtime.cpp`에서 legacy 정적 호출(`MeshManager::PrintMeshTree`) 제거

### 2.2 W5 (Cross-module 참조 정리)

적용 파일:
1. `webassembly/src/render/presentation/viewer_window.cpp`
2. `webassembly/src/atoms/atoms_template.cpp`
3. `webassembly/src/atoms/ui/charge_density_ui.cpp`
4. `webassembly/src/atoms/presentation/isosurface_panel.cpp`
5. `webassembly/src/mesh/presentation/mesh_detail_panel.h`
6. `webassembly/src/mesh/presentation/mesh_detail_panel.cpp`
7. `webassembly/src/mesh/domain/mesh_repository.h`
8. `webassembly/src/mesh/domain/mesh_repository.cpp`
9. `webassembly/src/mesh/domain/mesh_repository_core.cpp`
10. `webassembly/src/mesh/application/mesh_query_service.h`

핵심 변경:
1. `viewer_window`의 interaction LOD 경로를 `MeshQueryService` 기반 조회로 치환
2. `atoms_template`, `charge_density_ui`, `isosurface_panel`의 `MeshManager::Instance()` 직접 호출 제거
3. mesh detail 패널의 shared-volume 동기화 경로를 repository facade로 전환
4. repository facade에 호환 API를 보강해 외부 모듈이 mesh core singleton에 직접 접근하지 않도록 경계 고정
5. `MeshManager::Instance()` 호출은 allowlist 파일(`mesh_repository.cpp`) 1건만 유지

### 2.3 W6 (게이트/문서 패키징)

신규 게이트:
1. `scripts/refactoring/check_phase17_r4_mesh_migration.ps1`
2. `scripts/refactoring/check_phase17_mesh_singleton_usage.ps1`

문서 산출:
1. `docs/refactoring/phase17-root/r4/dependency_gate_report.md`
2. `docs/refactoring/phase17-root/r4/go_no_go_r5.md`

## 3. 완료 기준 점검

1. `workbench_runtime.cpp`에서 루트 mesh 헤더 include 0: `PASS`
2. `viewer_window.cpp`의 interaction LOD `MeshManager::Instance()` 직접 호출 제거: `PASS`
3. `atoms/*`의 mesh singleton 직접 호출 제거: `PASS`
4. 전체 코드베이스 `MeshManager::Instance()` 호출 allowlist 1건 유지: `PASS`
5. 신규 R4 게이트 2종 도입 및 PASS: `PASS`

## 4. 검증 결과

게이트:
1. `check_phase17_root_allowlist.ps1`: PASS
2. `check_phase17_root_ownership_map.ps1`: PASS
3. `check_phase17_facade_contract.ps1`: PASS
4. `check_phase17_r2_shell_ui_migration.ps1`: PASS
5. `check_phase17_r3_render_platform_io_migration.ps1`: PASS
6. `check_phase17_r4_mesh_migration.ps1`: PASS
7. `check_phase17_mesh_singleton_usage.ps1`: PASS

로그:
1. `docs/refactoring/phase17-root/r4/logs/check_phase17_root_allowlist_r4_w4_w6_latest.txt`
2. `docs/refactoring/phase17-root/r4/logs/check_phase17_root_ownership_map_r4_w4_w6_latest.txt`
3. `docs/refactoring/phase17-root/r4/logs/check_phase17_facade_contract_r4_w4_w6_latest.txt`
4. `docs/refactoring/phase17-root/r4/logs/check_phase17_r2_shell_ui_migration_r4_w4_w6_latest.txt`
5. `docs/refactoring/phase17-root/r4/logs/check_phase17_r3_render_platform_io_migration_r4_w4_w6_latest.txt`
6. `docs/refactoring/phase17-root/r4/logs/check_phase17_r4_mesh_migration_r4_w4_w6_latest.txt`
7. `docs/refactoring/phase17-root/r4/logs/check_phase17_mesh_singleton_usage_r4_w4_w6_latest.txt`

빌드/테스트:
1. `npm run build-wasm:release`: PASS
2. `npm run test:cpp`: PASS
3. `npm run test:smoke`: PASS

로그:
1. `docs/refactoring/phase17-root/r4/logs/build_phase17_r4_w4_w6_latest.txt`
2. `docs/refactoring/phase17-root/r4/logs/test_cpp_phase17_r4_w4_w6_latest.txt`
3. `docs/refactoring/phase17-root/r4/logs/test_smoke_phase17_r4_w4_w6_latest.txt`

## 5. 결론

- R4 `W4~W6` 범위는 계획서 기준으로 실행 완료.
- 판정: `GO (R5 착수 가능)`
