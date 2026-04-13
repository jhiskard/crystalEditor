# Phase 17-R3 인벤토리 스냅샷 (W0)

- 작성일: `2026-04-13`
- 기준 계획서:
  - `docs/refactoring/refactoring_plan_root_legacy_dismantle_260408_53Codex.md`
  - `docs/refactoring/phase17-root/r3/refactoring_phase17_r3_render_platform_io_dismantle_execution_plan_260413.md`

## 1. W0 점검 결과

### 1.1 루트 코드 파일 스냅샷
- `src root code files`: `23`
- `atoms root code files`: `2`
- 출처 로그: `docs/refactoring/phase17-root/r3/logs/check_phase17_root_allowlist_r3_w0_w3_latest.txt`

### 1.2 루트 소유권 맵 스냅샷
- `current root files`: `25`
- `dismantle_target`: `20`
- `documentation_material_excluded`: `2`
- `facade_keep`: `3`
- 출처 로그: `docs/refactoring/phase17-root/r3/logs/check_phase17_root_ownership_map_r3_w0_w3_latest.txt`

### 1.3 facade 계약 기준선
- `app.cpp legacy includes`: `7` (`<= 8` 유지)
- 출처 로그: `docs/refactoring/phase17-root/r3/logs/check_phase17_facade_contract_r3_w0_w3_latest.txt`

### 1.4 R2 회귀 기준선
- `check_phase17_r2_shell_ui_migration.ps1`: `PASS`
- 출처 로그: `docs/refactoring/phase17-root/r3/logs/check_phase17_r2_shell_ui_migration_r3_w0_w3_latest.txt`

## 2. W1~W3 착수 판정

- 판정: `GO`
- 근거:
  - R3 착수 전 기준선(allowlist/ownership/facade/R2 회귀) 모두 PASS
  - R3 W1~W3 해체 대상 루트 파일 상태 추적 가능

