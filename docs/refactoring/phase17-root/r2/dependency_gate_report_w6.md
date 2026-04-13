# Phase 17-R2 의존성 게이트 보고서 (W6)

작성일시: `2026-04-13 11:00 (KST)`

## 1. 실행 게이트

1. `scripts/refactoring/check_phase17_r2_shell_ui_migration.ps1`
2. `scripts/refactoring/check_phase17_facade_contract.ps1`
3. `scripts/refactoring/check_phase17_root_allowlist.ps1`
4. `scripts/refactoring/check_phase17_root_ownership_map.ps1`

## 2. 결과 요약

| 게이트 | 결과 |
|---|---|
| `P17R2.required_legacy_root_files_removed` | PASS |
| `P17R2.required_migrated_files_present` | PASS |
| `P17R2.optional_image_texture_state_consistent` | PASS |
| `P17R2.wb_ui_sources_registered` | PASS |
| `P17R2.wb_ui_legacy_sources_zero` | PASS |
| `P17R2.toolbar_singleton_direct_call_zero` | PASS |
| `P17R2.app_layout_command_helper_present` | PASS |
| `P17R2.app_menu_open_actions_synced` | PASS |
| `P17R2.app_layout_region_found` | PASS |
| `P17R2.app_layout_direct_show_assign_zero` | PASS |
| `P17R1.main_local_include_allowlist_exact` | PASS |
| `P17R1.app_h_forbidden_root_include_zero` | PASS |
| `P17R1.app_cpp_legacy_include_not_increased` | PASS |
| `P17R1.app_cpp_unexpected_root_include_zero` | PASS |
| `P17R1.main_app_forbidden_singleton_instance_zero` | PASS |
| `P17R1.main_forbidden_feature_access_zero` | PASS |
| `P17R1.main_apph_forbidden_type_token_zero` | PASS |
| `P17R0.src_root_set_within_baseline_snapshot` | PASS |
| `P17R0.atoms_root_set_within_baseline_snapshot` | PASS |
| `P17R0.src_root_file_count_not_increased` | PASS |
| `P17R0.atoms_root_file_count_not_increased` | PASS |
| `P17R0.facade_files_present` | PASS |
| `P17R0.documentation_only_files_classified` | PASS |
| `P17R0.ownership_map_covers_current_root_files` | PASS |
| `P17R0.ownership_map_entries_classified` | PASS |
| `P17R0.facade_entries_defined_in_map` | PASS |
| `P17R0.required_facade_files_present` | PASS |
| `P17R0.doc_material_exclusion_classified` | PASS |

## 3. 스냅샷 지표

1. 선택 이관군 상태: `migrated`
2. `app.cpp` `SetWindowVisible` 호출 수: `1`
3. 레이아웃 블록 `m_bShow*` 직접 대입 수: `0`
4. src root 코드 파일 수: `28`
5. atoms root 코드 파일 수: `2`
6. root ownership 분류:
7. `dismantle_target`: `25`
8. `documentation_material_excluded`: `2`
9. `facade_keep`: `3`
10. `app.cpp` legacy include 수: `7` (`<= 8`)

## 4. 로그 경로

1. `docs/refactoring/phase17-root/r2/logs/check_phase17_r2_shell_ui_migration_w6_latest.txt`
2. `docs/refactoring/phase17-root/r2/logs/check_phase17_facade_contract_r2_w6_latest.txt`
3. `docs/refactoring/phase17-root/r2/logs/check_phase17_root_allowlist_r2_w6_latest.txt`
4. `docs/refactoring/phase17-root/r2/logs/check_phase17_root_ownership_map_r2_w6_latest.txt`
5. `docs/refactoring/phase17-root/r2/logs/build_phase17_r2_w6_latest.txt`
6. `docs/refactoring/phase17-root/r2/logs/test_cpp_phase17_r2_w6_latest.txt`
7. `docs/refactoring/phase17-root/r2/logs/test_smoke_phase17_r2_w6_latest.txt`

## 5. 판정

- W6 게이트 종합: `PASS`
- R2 종료 판정: `PASS`
- 다음 단계: `R3 착수 가능`
