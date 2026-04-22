# Phase 19 / W9 — 게이트 패키지 및 메타 게이트 구성

작성일: `2026-04-22 (KST)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_complete_dismantle_codex_260421_v2.md`
검증보고서 근거: 재유입 차단(봉인), v2 6장 메타 게이트 설계
우선순위: **High**
예상 소요: 1 ~ 2일
선행 의존: W8
후속 작업: W10

---

## 1. 배경

W1~W8의 코드 변경이 완료되어도 자동 게이트가 없으면 재유입을 차단할 수 없다. W9는 Phase 19 전용 게이트 7종과 메타 게이트를 구성해 "자동 PASS/FAIL 기준"을 확정한다.

## 2. 신규 게이트 목록

| 스크립트 | 검증 내용 |
|---|---|
| `check_phase19_legacy_directory_zero.ps1` | legacy 디렉터리/파일 0 |
| `check_phase19_atoms_template_zero.ps1` | `AtomsTemplate` 심볼 0 |
| `check_phase19_legacy_runtime_zero.ps1` | `LegacyAtomsRuntime` 심볼 0 |
| `check_phase19_singleton_zero.ps1` | `DECLARE_SINGLETON`, `::Instance()` 0 |
| `check_phase19_friend_class_zero.ps1` | `friend class` 0 |
| `check_phase19_app_cpp_size.ps1` | `app.cpp` LOC ≤ 400 |
| `check_phase19_final_closure.ps1` | Phase19 + 기존 핵심 게이트 메타 실행 |

## 3. 메타 게이트 설계

`check_phase19_final_closure.ps1`는 다음 순서로 검증한다.

### 3.1 Phase 19 게이트

1. `check_phase19_legacy_directory_zero.ps1`
2. `check_phase19_atoms_template_zero.ps1`
3. `check_phase19_legacy_runtime_zero.ps1`
4. `check_phase19_singleton_zero.ps1`
5. `check_phase19_friend_class_zero.ps1`
6. `check_phase19_app_cpp_size.ps1`

### 3.2 기존 게이트 상속

1. `check_phase7_runtime_composition.ps1`
2. `check_phase12_architecture_seal.ps1`
3. `check_phase13_runtime_hard_seal.ps1`
4. `check_phase14_state_ownership_alias_zero.ps1`
5. `check_phase15_render_platform_isolation.ps1`
6. `check_phase17_root_allowlist.ps1`
7. `check_phase18_facade_include_zero.ps1`
8. `check_phase18_atoms_instance_zero.ps1`
9. `check_phase18_legacy_singleton_zero.ps1`
10. `check_phase18_doc_contract_full.ps1`
11. `check_phase18_final_closure.ps1`

## 4. 실행 순서

1. Phase19 게이트 7종 스크립트 작성.
2. `package.json`에 `check:phase19:*` 엔트리 추가.
3. `check_phase19_final_closure.ps1`에서 Phase19+기존 게이트 호출 체인 구성.
4. 각 게이트 단독 실행 후 PASS 확인.
5. 메타 게이트 실행 후 PASS 확인.
6. W9 완료 리포트 작성.

## 5. 파일 변경 명세

| 경로 | 변경 유형 | 비고 |
|---|---|---|
| `scripts/refactoring/check_phase19_legacy_directory_zero.ps1` | **신규** | |
| `scripts/refactoring/check_phase19_atoms_template_zero.ps1` | **신규** | |
| `scripts/refactoring/check_phase19_legacy_runtime_zero.ps1` | **신규** | |
| `scripts/refactoring/check_phase19_singleton_zero.ps1` | **신규** | W6 연계 |
| `scripts/refactoring/check_phase19_friend_class_zero.ps1` | **신규/갱신** | W1 연계 |
| `scripts/refactoring/check_phase19_app_cpp_size.ps1` | **신규/갱신** | W7 연계 |
| `scripts/refactoring/check_phase19_final_closure.ps1` | **신규** | 메타게이트 |
| `package.json` | 수정 | `check:phase19:*` 스크립트 |
| `docs/refactoring/phase19/logs/dependency_gate_report_W9.md` | **신규** | 완료 리포트 |

## 6. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| Phase19 게이트 7종 파일 | 파일 체크 | 전부 존재 |
| `check:phase19:*` npm 엔트리 | 검색 | 전부 존재 |
| 게이트 7종 단독 실행 | 실행 결과 | 전부 PASS |
| `check_phase19_final_closure.ps1` | 실행 결과 | PASS |
| W9 리포트 | 문서 체크 | 작성 완료 |

## 7. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| 파일명 불일치로 메타게이트 실패 | 자동화 신뢰 저하 | 스크립트 호출 경로를 실제 파일명 기준으로 고정 |
| 과도한 문자열 매칭 false positive | 불필요한 FAIL | 소스 확장자/제외 경로 명시 |
| 게이트 누락 | 재유입 검출 실패 | W9 DoD에 7종 전량 실행 증빙 의무화 |

## 8. 산출물

- `check_phase19_*.ps1` 7종
- `package.json` Phase19 스크립트 엔트리
- `dependency_gate_report_W9.md`

## 9. Git 커밋 템플릿

```
chore(gates): phase19/W9 — add phase19 gate package and final meta gate

- Add 7 phase19 gate scripts
- Wire npm check:phase19:* entries
- Add check_phase19_final_closure meta gate chaining phase7/12/13/14/15/17/18/19 checks

metrics:
  phase19 gate scripts: 0 -> 7
  phase19 npm check entries: 0 -> 7

verif:
  check:phase19:final-closure: PASS
```

---

*W9 완료 시 Phase 19 상태는 수동 판정이 아니라 자동 게이트로 봉인된다.*
