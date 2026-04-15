# Phase 17-R5 의존성 게이트 보고서

작성일시: `2026-04-14 (KST)`

## 1. 실행한 게이트

1. `check_phase17_root_allowlist.ps1`
2. `check_phase17_root_ownership_map.ps1`
3. `check_phase17_facade_contract.ps1`
4. `check_phase17_r2_shell_ui_migration.ps1`
5. `check_phase17_r3_render_platform_io_migration.ps1`
6. `check_phase17_r4_mesh_migration.ps1`
7. `check_phase17_mesh_singleton_usage.ps1`
8. `check_phase17_r5_atoms_migration.ps1`
9. `check_phase17_r5_shell_layout_reset_contract.ps1`

## 2. 게이트 결과 요약

| 게이트 | 결과 |
|---|---|
| `R0 Root Allowlist` | PASS |
| `R0 Root Ownership Map` | PASS |
| `R1 Facade Contract` | PASS |
| `R2 Shell/UI Migration` | PASS |
| `R3 Render/Platform/IO Migration` | PASS |
| `R4 Mesh Migration` | PASS |
| `R4 Mesh Singleton Usage` | PASS |
| `R5 Atoms Migration` | PASS |
| `R5 Shell Layout/Reset Contract` | PASS |

## 3. 핵심 스냅샷 지표

1. 루트 `webassembly/src/atoms` 코드 파일(`*.cpp`, `*.h`) 잔존: `0`
2. `#include "...atoms_template.h"` 참조: `0`
3. `atoms_template_facade.h` include 참조: `36`
4. `wb_atoms.cmake`의 `atoms_template_facade.cpp` 등록: `PASS`
5. `wb_atoms.cmake`의 `atoms_template.cpp` 잔존: `0`
6. `AtomsTemplate::Instance()` allowlist 외 호출: `0`
7. Layout/Reset 계약(`consumeInitialLayoutBootstrap`, one-shot `pendingLayoutPreset` 소모, 메뉴 후 즉시 sync): `PASS`

## 4. 실행 로그 경로

1. `docs/refactoring/phase17-root/r5/logs/check_phase17_root_allowlist_r5_w7_w8_latest.txt`
2. `docs/refactoring/phase17-root/r5/logs/check_phase17_root_ownership_map_r5_w7_w8_latest.txt`
3. `docs/refactoring/phase17-root/r5/logs/check_phase17_facade_contract_r5_w7_w8_latest.txt`
4. `docs/refactoring/phase17-root/r5/logs/check_phase17_r2_shell_ui_migration_r5_w7_w8_latest.txt`
5. `docs/refactoring/phase17-root/r5/logs/check_phase17_r3_render_platform_io_migration_r5_w7_w8_latest.txt`
6. `docs/refactoring/phase17-root/r5/logs/check_phase17_r4_mesh_migration_r5_w7_w8_latest.txt`
7. `docs/refactoring/phase17-root/r5/logs/check_phase17_mesh_singleton_usage_r5_w7_w8_latest.txt`
8. `docs/refactoring/phase17-root/r5/logs/check_phase17_r5_atoms_migration_r5_w7_w8_latest.txt`
9. `docs/refactoring/phase17-root/r5/logs/check_phase17_r5_shell_layout_reset_contract_r5_w7_w8_latest.txt`

## 5. 빌드/테스트 결과

1. `npm run build-wasm:release`: PASS
2. `npm run test:cpp`: PASS
3. `npm run test:smoke`: PASS

## 6. 판정

- 게이트 종합 판정: `PASS`
- 다음 단계: `R6` 착수 가능
