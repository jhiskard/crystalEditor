# Phase17-R6 Dependency Gate Report

작성일: `2026-04-16`

## 1. 실행 개요

R6 최종 구조 전환 이후, Phase17 관련 게이트를 재실행하여 경계/의존성/모듈 그래프 상태를 검증하였다.

## 2. 실행 게이트 목록 및 결과

| 게이트 스크립트 | 결과 |
|---|---|
| `check_phase17_root_allowlist.ps1` | PASS |
| `check_phase17_root_ownership_map.ps1` | PASS |
| `check_phase17_facade_contract.ps1` | PASS |
| `check_phase17_r2_shell_ui_migration.ps1` | PASS |
| `check_phase17_r3_render_platform_io_migration.ps1` | PASS |
| `check_phase17_r4_mesh_migration.ps1` | PASS |
| `check_phase17_r5_atoms_migration.ps1` | PASS |
| `check_phase17_r5_shell_layout_reset_contract.ps1` | PASS |
| `check_phase17_r6_atoms_legacy_zero.ps1` | PASS |
| `check_phase17_target_graph_final.ps1` | PASS |

## 3. 핵심 확인사항

1. `webassembly/src/atoms` 하위 코드 파일(`*.cpp/*.h`) 0건 유지
2. legacy facade 경로(`atoms/legacy/atoms_template_facade.h`) 참조 0건
3. 최종 모듈 10개(`wb_common`, `wb_platform`, `wb_shell`, `wb_workspace`, `wb_render`, `wb_mesh`, `wb_structure`, `wb_measurement`, `wb_density`, `wb_io`) 구성 확인
4. `wb_core`, `wb_ui`, `wb_atoms` 모듈 파일 및 `CMakeLists.txt` 참조 제거 확인
5. root allowlist/ownership map/facade contract 기준선 PASS 유지

## 4. 결론

R6 의존성/경계 게이트는 모두 PASS이며, 최종 타깃 그래프 전환 상태가 유효함을 확인하였다.
