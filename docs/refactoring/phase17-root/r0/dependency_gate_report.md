# Phase 17-R0 의존성 게이트 보고서

작성일시: `2026-04-08 15:16 (KST)`

## 1. 실행한 게이트

1. `check_phase17_root_allowlist.ps1`
2. `check_phase17_root_ownership_map.ps1`

## 2. 게이트 결과 요약

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

## 3. 스냅샷 지표

- src root 코드 파일 수: `40`
- atoms root 코드 파일 수: `2`
- 현재 루트 코드 파일 총수: `42`
- 분류:
  - `facade_keep`: `3`
  - `documentation_material_excluded`: `2`
  - `dismantle_target`: `37`

## 4. 로그 경로

- `docs/refactoring/phase17-root/r0/logs/check_phase17_root_allowlist_latest.txt`
- `docs/refactoring/phase17-root/r0/logs/check_phase17_root_ownership_map_latest.txt`
- `docs/refactoring/phase17-root/r0/logs/root_inventory_phase17_r0_latest.md`

## 5. 판정

- 게이트 종합 판정: `PASS`
- 다음 단계: `R1(Facade 축소 선행)` 진행 가능
