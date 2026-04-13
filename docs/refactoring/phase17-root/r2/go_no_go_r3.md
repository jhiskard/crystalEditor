# Phase 17-R2 GO/NO-GO 판정서 (for R3)

작성일: `2026-04-13`  
판정 대상: `R3. 루트 레거시 해체 후속 단계`

## 1. 체크리스트

1. `check_phase17_r2_shell_ui_migration.ps1` 도입 및 PASS: `PASS`
2. R0/R1/R2 게이트 동시 PASS 유지: `PASS`
3. `build-wasm:release` PASS: `PASS`
4. `test:cpp` PASS: `PASS`
5. `test:smoke` PASS: `PASS`
6. 선택 이관군(`image/texture`) 상태 명시: `PASS` (`migrated`)

## 2. 근거 문서

1. `docs/refactoring/phase17-root/r2/refactoring_phase17_r2_w0_w3_shell_ui_migration_execution_260413.md`
2. `docs/refactoring/phase17-root/r2/refactoring_phase17_r2_w4_w5_shell_ui_state_resource_migration_execution_260413.md`
3. `docs/refactoring/phase17-root/r2/refactoring_phase17_r2_w6_cmake_gate_documentation_execution_260413.md`
4. `docs/refactoring/phase17-root/r2/dependency_gate_report_w6.md`
5. `docs/refactoring/phase17-root/r2/logs/check_phase17_r2_shell_ui_migration_w6_latest.txt`
6. `docs/refactoring/phase17-root/r2/logs/build_phase17_r2_w6_latest.txt`
7. `docs/refactoring/phase17-root/r2/logs/test_cpp_phase17_r2_w6_latest.txt`
8. `docs/refactoring/phase17-root/r2/logs/test_smoke_phase17_r2_w6_latest.txt`

## 3. 판정

- 결론: `GO`
- 조건:
1. R3 진행 중 `app.cpp` legacy include 수가 `8`을 초과하면 즉시 FAIL로 되돌린다.
2. `shell/presentation/toolbar`에서 `AtomsTemplate::Instance()` 직접 호출 재유입을 금지한다.
3. `wb_ui.cmake`에 레거시 루트 소스 경로 재등록을 금지한다.
