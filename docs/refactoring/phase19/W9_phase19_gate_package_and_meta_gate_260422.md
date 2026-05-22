# Phase 19 / W9 — 게이트 패키지 및 메타 게이트 구성

작성일: `2026-04-22 (KST)`
최종 업데이트: `2026-05-22 (KST, W8 완료 + W2 보완 반영)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_complete_dismantle_codex_260421_v2.md`
검증보고서 근거: 재유입 차단(봉인), v2 6장 메타 게이트 설계
우선순위: **High**
예상 소요: 1 ~ 2일
선행 의존: W8
후속 작업: W10

---

## 1. 배경

W1~W8 코드 이행은 완료되었지만, 자동 게이트 패키지는 아직 부분 상태다. 현재 코드는 W2 보완으로 `measurement/application`의 VTK 타입 노출까지 제거된 상태이므로, W9는 "원계획 7종 게이트 완성 + 보완 이슈 재유입 차단 게이트 추가"로 정렬해야 한다.

## 2. 착수 전 정합성 점검 (2026-05-22)

| 항목 | 점검값 | 해석 |
|---|---:|---|
| Phase19 게이트 스크립트 존재(원계획 7종 기준) | 3/7 | `friend/singleton/app-cpp-size`만 존재 |
| 미구현 Phase19 게이트 스크립트 | 4 | `legacy_directory_zero`, `atoms_template_zero`, `legacy_runtime_zero`, `final_closure` |
| `package.json` `check:phase19:*` 엔트리(원계획 7종 기준) | 3/7 | `check:phase19:final` 계열 미구현 |
| 상속 게이트(Phase7/12/13/14/15/17/18) 스크립트 존재 | 11/11 | 메타게이트 체인 구성 가능 |
| W9 완료 로그 | 0 | `dependency_gate_report_W9.md` 미생성 |
| W2 보완 후 `measurement/application` VTK 토큰 | 0 | W9에서 봉인 게이트로 고정 필요 |

## 3. 업데이트된 W9 목표

### 3.1 원계획 7종 완성

| 스크립트 | 상태 | 작업 |
|---|---|---|
| `check_phase19_friend_class_zero.ps1` | 존재 | 유지/검증 |
| `check_phase19_singleton_zero.ps1` | 존재 | 유지/검증 |
| `check_phase19_app_cpp_size.ps1` | 존재 | 유지/검증 |
| `check_phase19_legacy_directory_zero.ps1` | 없음 | 신규 |
| `check_phase19_atoms_template_zero.ps1` | 없음 | 신규 |
| `check_phase19_legacy_runtime_zero.ps1` | 없음 | 신규 |
| `check_phase19_final_closure.ps1` | 없음 | 신규(메타) |

### 3.2 보강 게이트 (W2 보완 재유입 차단)

| 스크립트 | 목적 |
|---|---|
| `check_phase19_measurement_vtk_zero.ps1` | `webassembly/src/measurement/application`의 `vtk*` 토큰 0 강제 |

## 4. 메타 게이트 설계 업데이트

`check_phase19_final_closure.ps1`는 아래 순서로 실행한다.
Phase19 게이트는 `strict`, 상속 게이트는 `advisory` 모드로 실행/기록한다.

### 4.1 Phase 19 게이트 체인

1. `check_phase19_legacy_directory_zero.ps1`
2. `check_phase19_atoms_template_zero.ps1`
3. `check_phase19_legacy_runtime_zero.ps1`
4. `check_phase19_singleton_zero.ps1`
5. `check_phase19_friend_class_zero.ps1`
6. `check_phase19_app_cpp_size.ps1`
7. `check_phase19_measurement_vtk_zero.ps1` (보강)

### 4.2 기존 게이트 상속 체인

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

상속 체인 해석 규칙:

- `PASS`: 호환 상태 유지
- `FAIL`: 경고로 리포트에 기록(Phase19 최종 게이트의 block 조건은 아님)
- 사유: 일부 상속 게이트는 삭제된 legacy 파일 존재를 전제로 하며 Phase19 물리 삭제 상태와 충돌

## 5. 실행 순서 (업데이트)

1. 누락된 Phase19 스크립트 5종 작성(`legacy_directory_zero`, `atoms_template_zero`, `legacy_runtime_zero`, `measurement_vtk_zero`, `final_closure`).
2. `package.json`에 아래 엔트리 추가:
   - `check:phase19:legacy-directory-zero`
   - `check:phase19:atoms-template-zero`
   - `check:phase19:legacy-runtime-zero`
   - `check:phase19:measurement-vtk-zero`
   - `check:phase19:final` (또는 `check:phase19:final-closure`)
3. Phase19 단일 게이트 전부 개별 실행 및 PASS 확인.
4. `check_phase19_final_closure.ps1` 실행 및 PASS 확인.
5. `docs/refactoring/phase19/logs/dependency_gate_report_W9.md` 작성.

## 6. 파일 변경 명세 (업데이트)

| 경로 | 변경 유형 | 비고 |
|---|---|---|
| `scripts/refactoring/check_phase19_legacy_directory_zero.ps1` | **신규** | |
| `scripts/refactoring/check_phase19_atoms_template_zero.ps1` | **신규** | |
| `scripts/refactoring/check_phase19_legacy_runtime_zero.ps1` | **신규** | |
| `scripts/refactoring/check_phase19_measurement_vtk_zero.ps1` | **신규** | W2 보완 봉인 |
| `scripts/refactoring/check_phase19_final_closure.ps1` | **신규** | 메타게이트 |
| `scripts/refactoring/check_phase19_singleton_zero.ps1` | 유지/점검 | W5.10/W6 결과 기준 |
| `scripts/refactoring/check_phase19_friend_class_zero.ps1` | 유지/점검 | W1 결과 기준 |
| `scripts/refactoring/check_phase19_app_cpp_size.ps1` | 유지/점검 | W7 결과 기준 |
| `package.json` | 수정 | `check:phase19:*` 엔트리 보강 |
| `docs/refactoring/phase19/logs/dependency_gate_report_W9.md` | **신규** | 완료 리포트 |

## 7. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| Phase19 단일 게이트 스크립트 | 파일 체크 | 8종 존재(원계획 7 + 보강 1) |
| `check:phase19:*` npm 엔트리 | 검색 | 누락 0 |
| Phase19 단일 게이트 개별 실행 | 실행 결과 | 전부 PASS |
| `check_phase19_final_closure.ps1` | 실행 결과 | PASS (strict chain) |
| W9 리포트 | 문서 체크 | 작성 완료 |

## 8. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| 파일명/엔트리명 불일치 | 메타게이트 실패 | 스크립트 파일명과 `package.json` 키를 1:1 매핑 |
| 문자열 매칭 과검출 | 불필요한 FAIL | 검색 범위(`webassembly/src`, `scripts`, `docs`)를 목적별로 분리 |
| W2 보완 회귀 | DoD 8 위반 재발 | `measurement_vtk_zero` 게이트 상시 편입 |

## 9. 산출물

- `check_phase19_*.ps1` 패키지(원계획 7 + 보강 1)
- `package.json`의 Phase19 게이트 엔트리
- `docs/refactoring/phase19/logs/dependency_gate_report_W9.md`

## 10. Git 커밋 템플릿

```text
chore(gates): phase19/W9 - complete phase19 gate package and add measurement vtk zero seal

- Add missing phase19 gate scripts and final closure meta gate
- Keep existing friend/singleton/app-cpp gates and wire full npm entries
- Add measurement/application vtk-token zero gate to prevent W2 regression

metrics:
  phase19 gate scripts: 3 -> 8
  phase19 npm check entries: 3 -> full

verif:
  check:phase19:final: PASS
```

---

*W9 완료 시 Phase 19 상태는 수동 판정이 아니라 자동 게이트 체인으로 봉인된다.*
