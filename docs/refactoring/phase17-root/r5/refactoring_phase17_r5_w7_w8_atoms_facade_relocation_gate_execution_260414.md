# Phase 17-R5 W7~W8 실행 보고서

작성일: `2026-04-14`  
기준 계획서: `docs/refactoring/phase17-root/r5/refactoring_phase17_r5_atoms_dismantle_execution_plan_260414.md`

## 0. 실행 범위

1. W7: `atoms_template.*` 제거 및 CMake 정렬
2. W8: R5 전용 게이트 도입/실행 및 보고서 패키징

## 1. W7 수행 내역

### 1.1 파일 해체/이동

1. 삭제:
   - `webassembly/src/atoms/atoms_template.cpp`
   - `webassembly/src/atoms/atoms_template.h`
2. 신규(legacy facade 경로로 이동):
   - `webassembly/src/atoms/legacy/atoms_template_facade.cpp`
   - `webassembly/src/atoms/legacy/atoms_template_facade.h`

### 1.2 include 경로 정렬

1. 전체 소스에서 `atoms_template.h` include를 `atoms_template_facade.h` 경로로 치환.
2. `webassembly/src/atoms/legacy/atoms_template_facade.*` 내부 상대 include 경로를 `legacy` 하위 구조 기준으로 재정렬.
3. `app.cpp`의 legacy include 주석 정리 및 facade include 경로 확정.

### 1.3 CMake 정렬

1. `webassembly/cmake/modules/wb_atoms.cmake`
   - `webassembly/src/atoms/atoms_template.cpp` 제거
   - `webassembly/src/atoms/legacy/atoms_template_facade.cpp` 등록

### 1.4 singleton 호출 경계 정리

1. `mesh/presentation/model_tree_dialogs.cpp`
2. `mesh/presentation/model_tree_mesh_section.cpp`
3. `mesh/presentation/model_tree_structure_section.cpp`
4. `render/presentation/viewer_window.cpp`

위 파일에서 `AtomsTemplate::Instance()` 직접 호출을 `GetWorkbenchRuntime().AtomsTemplateFacade()` 경유로 치환하여 composition root 중심 경계로 정렬.

## 2. W8 수행 내역

### 2.1 신규 게이트 스크립트 도입

1. `scripts/refactoring/check_phase17_r5_atoms_migration.ps1`
   - legacy 파일 제거
   - 루트 atoms 코드 파일 수
   - legacy include 참조
   - `wb_atoms.cmake` 엔트리
   - singleton allowlist 위반
2. `scripts/refactoring/check_phase17_r5_shell_layout_reset_contract.ps1`
   - `consumeInitialLayoutBootstrap` 규약
   - Layout one-shot 소모 규약
   - 메뉴 액션 직후 sync 규약
   - Reset geometry pass 규약

### 2.2 기존 게이트 경로 업데이트

1. `scripts/refactoring/check_phase17_facade_contract.ps1`
   - app.cpp 허용 legacy include 경로를 `atoms/legacy/atoms_template_facade.h`로 업데이트.

### 2.3 게이트 실행 로그 생성

1. `check_phase17_root_allowlist_r5_w7_w8_latest.txt`
2. `check_phase17_root_ownership_map_r5_w7_w8_latest.txt`
3. `check_phase17_facade_contract_r5_w7_w8_latest.txt`
4. `check_phase17_r2_shell_ui_migration_r5_w7_w8_latest.txt`
5. `check_phase17_r3_render_platform_io_migration_r5_w7_w8_latest.txt`
6. `check_phase17_r4_mesh_migration_r5_w7_w8_latest.txt`
7. `check_phase17_mesh_singleton_usage_r5_w7_w8_latest.txt`
8. `check_phase17_r5_atoms_migration_r5_w7_w8_latest.txt`
9. `check_phase17_r5_shell_layout_reset_contract_r5_w7_w8_latest.txt`

## 3. 검증 결과

1. 게이트 9종: 모두 `PASS`
2. `npm run build-wasm:release`: `PASS`
3. `npm run test:cpp`: `PASS`
4. `npm run test:smoke`: `PASS`

## 4. 완료 기준 대비

1. 루트 `webassembly/src/atoms` 코드 파일(`*.cpp`, `*.h`) 0개: 달성
2. `atoms_template.h` include 참조 0건: 달성
3. `wb_atoms`의 facade 경로 정렬: 달성
4. R5 전용 게이트 2종 도입 및 PASS: 달성
5. R6 착수 문서(`dependency_gate_report`, `go_no_go_r6`) 작성: 달성

## 5. 후속 권고

1. R6에서 `atoms/legacy`의 facade 내부 책임을 `structure/measurement/density/render/shell`로 추가 분해.
2. facade 내부 `AtomsTemplate::Instance()` self-call 제거 여부를 별도 기술부채 항목으로 관리.
