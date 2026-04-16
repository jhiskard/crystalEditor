# Phase 17-R2 의존성 게이트 보고서 (W4~W5)

작성일시: `2026-04-13 10:36 (KST)`

## 1. 실행 게이트

1. `scripts/refactoring/check_phase17_root_allowlist.ps1`
2. `scripts/refactoring/check_phase17_root_ownership_map.ps1`
3. `scripts/refactoring/check_phase17_facade_contract.ps1`

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

## 3. 스냅샷 지표

1. src root 코드 파일 수: `28`
2. atoms root 코드 파일 수: `2`
3. root ownership 분류:
4. `dismantle_target`: `25`
5. `documentation_material_excluded`: `2`
6. `facade_keep`: `3`
7. `app.cpp legacy include`: `7` (`<= 8` 유지)

## 4. 로그 경로

1. `docs/refactoring/phase17-root/r2/logs/check_phase17_root_allowlist_r2_w4_w5_latest.txt`
2. `docs/refactoring/phase17-root/r2/logs/check_phase17_root_ownership_map_r2_w4_w5_latest.txt`
3. `docs/refactoring/phase17-root/r2/logs/check_phase17_facade_contract_r2_w4_w5_latest.txt`

## 5. 결론

- W4~W5 구간 게이트 판정: `PASS`
- 다음 단계: `R2-W6` 진행 가능
