# Phase 17-R2 실행 보고서 (W6: CMake/게이트 정렬 및 문서화)

작성일: `2026-04-13`  
기준 계획서: `docs/refactoring/refactoring_plan_root_legacy_dismantle_260408_53Codex.md`  
실행 계획서: `docs/refactoring/phase17-root/r2/refactoring_phase17_r2_shell_ui_dismantle_execution_plan_260409.md`

## 1. 실행 범위

W6 목표 항목을 다음 순서로 실행했다.

1. `wb_ui.cmake` 이관 경로 정합성 재검증
2. 신규 게이트 스크립트 도입
3. R2 게이트/빌드/테스트 실행
4. R2 종료 문서(게이트 보고서, GO/NO-GO) 작성

## 2. W6 코드 및 스크립트 작업

## 2.1 CMake 경로 정렬 상태

확인 파일:
1. `webassembly/cmake/modules/wb_ui.cmake`

확인 결과:
1. `icon_button.cpp`, `font_registry.cpp`, `test_window_panel.cpp`, `viewer_toolbar_presenter.cpp`가 신규 경로로 등록됨
2. W5 이관군 `image_loader.cpp`, `texture_resource.cpp`가 신규 경로로 등록됨
3. 레거시 루트 경로(`custom_ui.cpp`, `font_manager.cpp`, `test_window.cpp`, `toolbar.cpp`, `image.cpp`, `texture.cpp`) 직접 등록 0

## 2.2 신규 게이트 스크립트 도입

신규 파일:
1. `scripts/refactoring/check_phase17_r2_shell_ui_migration.ps1`

검증 항목:
1. 필수 루트 레거시 파일 제거 여부
2. 필수 이관 파일 존재 여부
3. 선택 이관군(`image/texture`) 상태 일관성(legacy 또는 migrated 단일 상태)
4. `wb_ui.cmake` 소스 등록 정합성
5. `toolbar` 내 `AtomsTemplate::Instance()` 직접 호출 0
6. `app.cpp` 레이아웃 command 경로(`SetWindowVisible`) 사용 여부
7. `app.cpp` 메뉴 open action 이후 `syncShellStateFromStore()` 동기화 여부
8. 레이아웃 적용 구간의 `m_bShow*` 직접 대입 0

## 3. 실행 결과

## 3.1 R2 게이트 실행

실행 명령:
1. `check_phase17_r2_shell_ui_migration.ps1`
2. `check_phase17_facade_contract.ps1`
3. `check_phase17_root_allowlist.ps1`
4. `check_phase17_root_ownership_map.ps1`

결과:
1. 전 항목 PASS
2. W5 상태: `migrated`
3. `app.cpp` legacy include: `7` (`<= 8` 유지)
4. src/atoms root 코드 파일 수: `28 / 2`

로그:
1. `docs/refactoring/phase17-root/r2/logs/check_phase17_r2_shell_ui_migration_w6_latest.txt`
2. `docs/refactoring/phase17-root/r2/logs/check_phase17_facade_contract_r2_w6_latest.txt`
3. `docs/refactoring/phase17-root/r2/logs/check_phase17_root_allowlist_r2_w6_latest.txt`
4. `docs/refactoring/phase17-root/r2/logs/check_phase17_root_ownership_map_r2_w6_latest.txt`

## 3.2 빌드/테스트 실행

실행 명령:
1. `npm run build-wasm:release`
2. `npm run test:cpp`
3. `npm run test:smoke`

결과:
1. build PASS (`VTK-Workbench.js/.wasm/.data` up-to-date install)
2. C++ test PASS (`1/1`, 100%)
3. smoke test PASS (`1 passed`)

로그:
1. `docs/refactoring/phase17-root/r2/logs/build_phase17_r2_w6_latest.txt`
2. `docs/refactoring/phase17-root/r2/logs/test_cpp_phase17_r2_w6_latest.txt`
3. `docs/refactoring/phase17-root/r2/logs/test_smoke_phase17_r2_w6_latest.txt`

## 4. R2 완료 기준 판정

`refactoring_phase17_r2_shell_ui_dismantle_execution_plan_260409.md`의 완료 기준 대비 판정:

1. R2 게이트 PASS: 충족
2. `app.cpp` 직접 상태 조작 축소 및 facade 계약 유지: 충족
3. 선택 이관군(`image/texture`) 수행 여부 명시: 충족 (`migrated`)
4. R3 착수 문서 완료: 충족 (`go_no_go_r3.md`)

최종 판정: `R2 완료 / R3 착수 가능`
