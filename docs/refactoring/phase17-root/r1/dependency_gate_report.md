# Phase 17-R1 의존성 게이트 보고서

작성일시: `2026-04-08 15:23 (KST)`

## 1. 실행한 게이트

1. `check_phase17_facade_contract.ps1`

## 2. 게이트 결과 요약

| 게이트 | 결과 |
|---|---|
| `P17R1.main_local_include_allowlist_exact` | PASS |
| `P17R1.app_h_forbidden_root_include_zero` | PASS |
| `P17R1.app_cpp_legacy_include_not_increased` | PASS |
| `P17R1.app_cpp_unexpected_root_include_zero` | PASS |
| `P17R1.main_app_forbidden_singleton_instance_zero` | PASS |
| `P17R1.main_forbidden_feature_access_zero` | PASS |
| `P17R1.main_apph_forbidden_type_token_zero` | PASS |

## 3. 스냅샷 지표

- `app.cpp` 루트 legacy include 수: `8`
- facade 관련 singleton `::Instance()` 호출 수: `0`
- `main.cpp` feature 직접 접근 수: `0`

## 4. 로그 경로

- `docs/refactoring/phase17-root/r1/logs/check_phase17_facade_contract_latest.txt`
- `docs/refactoring/phase17-root/r1/logs/facade_dependency_inventory_phase17_r1_latest.md`

## 5. 판정

- 게이트 종합 판정: `PASS`
- 다음 단계: `R2(Shell/UI 파일군 이관)` 진행 가능
