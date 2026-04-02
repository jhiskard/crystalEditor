# Phase 9 Go/No-Go (Phase 8 종료 판정)

판정일: `2026-04-01 (KST)`  
최종 업데이트: `2026-04-02 (KST, Phase 8 종료 선언 + 추적 이슈 등록)`  
판정 근거:
- 계획서: `docs/refactoring/phase8/refactoring_phase8_atoms_template_dismantle_260401.md`
- 게이트 리포트: `docs/refactoring/phase8/dependency_gate_report.md`

## 1. 게이트 체크리스트
| 게이트 | 기준 | 상태 | 근거 |
|---|---|---|---|
| G1. W0~W6 완료 | 모듈 분리, friend 제거, 게이트/테스트/종료 문서까지 완료 | PASS | Phase 8 계획서 체크리스트 |
| G2. 정적 구조 게이트 | `check_phase8_atoms_template_dismantle.ps1` 전 항목 PASS | PASS | `check_phase8_atoms_template_dismantle_latest.txt` |
| G3. Release 빌드 게이트 | `npm run build-wasm:release` 성공 | PASS | `build_phase8_latest.txt` |
| G4. C++ 테스트 게이트 | `npm run test:cpp` 성공 | PASS | `unit_test_phase8_latest.txt` |
| G5. 브라우저 smoke 게이트 | `npm run test:smoke` 성공 | PASS | `smoke_phase8_latest.txt` |
| G6. Phase 7 보호 규칙 유지 | 폰트 초기화 순서 가드 + XSF bootstrap 가드 유지 | PASS | `dependency_gate_report.md` 1장/3장 |

## 2. 최종 판정
- **GO**
- 해석:
  - Phase 8 목표였던 `AtomsTemplate` 책임 분해 착수 조건(모듈 경계 도입, friend 제거, 호출 경로 전환)이 정적/동적 게이트로 검증되었다.
  - Phase 7에서 복구한 폰트/아이콘 및 VASP->XSF 회귀 방지 규칙도 유지되고 있다.
  - **2026-04-02 기준으로 Phase 8 종료를 공식 선언**하고, Phase 9 계획 수립/착수 준비로 전환한다.

## 3. Phase 9 착수 메모
1. Phase 9 핵심 목표는 `render` 외부의 `VtkViewer::Instance()` 호출을 0건으로 만드는 것이다.
2. 현재 `atoms_template.cpp`의 `VtkViewer::Instance()`는 46건으로, Phase 9에서 우선 전환 대상이다.
3. 로컬 게이트 실행 시 `emsdk`/Playwright 권한 제약이 발생할 수 있으므로 실행 절차에 권한 상승 경로를 포함한다.
4. `Phase 1 이전부터 존재`한 VASP 로딩 관련 회귀를 Phase 9 추적 항목으로 등록한다.
   - 증상: `XSF(Grid) -> VASP -> XSF(Grid) -> VASP` 순서에서 기존 grid 데이터가 잔존할 수 있음.
   - 상태: 2026-04-02 기준 미해결(시도 패치 2건 롤백 완료).
   - 추적 규칙: Phase 9 W0에서 재현 절차/로그를 고정하고, W6 게이트 문서에서 해결/이관(Phase 10) 여부를 판정한다.

## 4. 잔여 리스크
| ID | 리스크 | 수준 | 대응 |
|---|---|---|---|
| R1 | render port 전환 중 overlay/picking 동작 회귀 가능성 | Medium | 뷰어 상호작용 smoke 시나리오 보강 + 단계별 빌드 검증 |
| R2 | legacy alias와 facade 잔존으로 책임 경계가 부분적으로 중첩 | Medium | Phase 9~10에서 호출 추적 로그 기반으로 점진 정리 |
| R3 | 테스트가 최소 smoke 1건 중심이라 UI 회귀 포착 범위 제한 | Low | Phase 9 중 브라우저 회귀 케이스 확대 |
| R4 | `XSF(Grid) -> VASP` 반복 전환 시 grid 잔존 가능성(미해결) | Medium | Phase 9 추적 이슈(`P9-BUG-01`)로 등록, W0 재현 로그 + W6 판정 문서로 상태 고정 |
