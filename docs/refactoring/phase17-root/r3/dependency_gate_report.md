# Phase 17-R3 의존성 게이트 보고서 (W7)

작성일시: `2026-04-13 16:20 (KST)`

## 1. 실행 게이트

1. `scripts/refactoring/check_phase17_root_allowlist.ps1`
2. `scripts/refactoring/check_phase17_root_ownership_map.ps1`
3. `scripts/refactoring/check_phase17_facade_contract.ps1`
4. `scripts/refactoring/check_phase17_r2_shell_ui_migration.ps1`
5. `scripts/refactoring/check_phase17_r3_render_platform_io_migration.ps1`
6. `scripts/refactoring/check_phase17_port_adapter_contract.ps1`

## 2. 결과 요약

| 게이트 | 결과 |
|---|---|
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
| `P17R1.main_local_include_allowlist_exact` | PASS |
| `P17R1.app_h_forbidden_root_include_zero` | PASS |
| `P17R1.app_cpp_legacy_include_not_increased` | PASS |
| `P17R1.app_cpp_unexpected_root_include_zero` | PASS |
| `P17R1.main_app_forbidden_singleton_instance_zero` | PASS |
| `P17R1.main_forbidden_feature_access_zero` | PASS |
| `P17R1.main_apph_forbidden_type_token_zero` | PASS |
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
| `P17R3.legacy_root_files_removed` | PASS |
| `P17R3.migrated_files_present` | PASS |
| `P17R3.wb_io_sources_registered` | PASS |
| `P17R3.wb_io_legacy_sources_zero` | PASS |
| `P17R3.wb_render_sources_registered` | PASS |
| `P17R3.wb_render_legacy_sources_zero` | PASS |
| `P17R3.legacy_include_reference_zero` | PASS |
| `P17R3.io_app_forbidden_include_zero` | PASS |
| `P17R3.io_app_forbidden_symbol_zero` | PASS |
| `P17R3.io_app_concrete_adapter_token_zero` | PASS |
| `P17R3.io_app_port_headers_wired` | PASS |
| `P17R3.runtime_adapter_includes_present` | PASS |
| `P17R3.runtime_adapter_wiring_present` | PASS |
| `P17R3.runtime_legacy_root_include_zero` | PASS |
| `P17R3.render_app_em_asm_zero` | PASS |
| `P17R3.render_app_emscripten_include_zero` | PASS |

## 3. 스냅샷 지표

1. src root 코드 파일 수: `19`
2. atoms root 코드 파일 수: `2`
3. 현재 root ownership 분류:
4. `dismantle_target`: `16`
5. `documentation_material_excluded`: `2`
6. `facade_keep`: `3`
7. `app.cpp` legacy include 수: `6` (`<= 8`)
8. `io/application` 스캔 파일 수: `13`
9. `io/application` forbidden include/symbol: `0 / 0`

## 4. 빌드/테스트 결과

1. `npm run build-wasm:release`: `PASS`
2. `npm run test:cpp`: `PASS`
3. `npm run test:smoke`: `PASS`

## 5. 로그 경로

1. `docs/refactoring/phase17-root/r3/logs/check_phase17_root_allowlist_r3_w7_latest.txt`
2. `docs/refactoring/phase17-root/r3/logs/check_phase17_root_ownership_map_r3_w7_latest.txt`
3. `docs/refactoring/phase17-root/r3/logs/check_phase17_facade_contract_r3_w7_latest.txt`
4. `docs/refactoring/phase17-root/r3/logs/check_phase17_r2_shell_ui_migration_r3_w7_latest.txt`
5. `docs/refactoring/phase17-root/r3/logs/check_phase17_r3_render_platform_io_migration_r3_w7_latest.txt`
6. `docs/refactoring/phase17-root/r3/logs/check_phase17_port_adapter_contract_r3_w7_latest.txt`
7. `docs/refactoring/phase17-root/r3/logs/build_phase17_r3_w7_latest.txt`
8. `docs/refactoring/phase17-root/r3/logs/test_cpp_phase17_r3_w7_latest.txt`
9. `docs/refactoring/phase17-root/r3/logs/test_smoke_phase17_r3_w7_latest.txt`

## 6. 판정

- W7 게이트 종합: `PASS`
- R3 종료 판정: `PASS`
- 다음 단계: `R4 착수 가능`

