# 리팩토링 결과보고서: Phase 17-Root Legacy Dismantle 비교 평가

작성일: `2026-04-16`  
대상 저장소: `vtk-workbench_jclee`  
비교 기준:
- 전체 계획서: `docs/refactoring/refactoring_plan_root_legacy_dismantle_260408_53Codex.md`
- R6 계획서: `docs/refactoring/phase17-root/r6/refactoring_phase17_r6_final_target_graph_legacy_seal_execution_plan_260415.md`
- R6 실행 보고: `docs/refactoring/phase17-root/r6/refactoring_phase17_r6_w0_w4_execution_260415.md`
- R6 실행 보고: `docs/refactoring/phase17-root/r6/refactoring_phase17_r6_w5_w8_execution_260416.md`
- R6 게이트 보고: `docs/refactoring/phase17-root/r6/dependency_gate_report.md`
- R6 종료 판정: `docs/refactoring/phase17-root/r6/go_no_go_phase17_close.md`
- 각 Phase17 게이트 스크립트 및 `2026-04-16` 기준 현재 코드

## 1. 종합 결론

Phase 17-Root(R0~R6)의 실행 목표는 자동 게이트/자동 테스트 기준으로 완료되었다.

- 루트 코드 파일은 `main.cpp`, `app.cpp`, `app.h`만 유지
- `webassembly/src/atoms` 하위 코드 파일(`*.cpp/*.h`) 0건
- 최종 모듈 그래프(10개 타깃) 전환 완료
- Phase17 게이트 세트, 릴리즈 빌드, C++ 테스트, 스모크 테스트 PASS

다만 계획서의 수동 회귀 항목(Reset/Layout 1/2/3, Edit/Build/Data, BZ, import)은 본 자동 실행 세션에서 직접 조작 검증을 수행하지 못했다.

따라서 현재 판정은 다음과 같다.

- `Phase17-Root 실행 목표`: 완료
- `최종 DoD(자동 게이트/자동 테스트 기준)`: 달성
- `프로그램 운영 판정`: `GO(자동 기준)`, 수동 회귀 최종 확인 필요

## 2. 핵심 측정 결과

현재 코드와 계획 기준선을 대조한 핵심 지표는 아래와 같다.

| 항목 | 기준선/목표 | 현재값(2026-04-16) | 평가 |
|---|---:|---:|---|
| `webassembly/src` 루트 코드 파일 수(`*.cpp/*.h`) | 3 유지 | 3 | 달성 |
| `webassembly/src/atoms` 하위 코드 파일 수(`*.cpp/*.h`) | 0 | 0 | 달성 |
| legacy facade 구경로 참조(`atoms/legacy/atoms_template_facade.h`) | 0 | 0 | 달성 |
| facade 신경로 참조(`workspace/legacy/atoms_template_facade.h`) | 운영 경로 유지 | 36 | 달성 |
| legacy port adapter 파일 수(`*legacy_port_adapter*`) | 0 | 0 | 달성 |
| 최종 모듈 수(`wb_*.cmake`) | 10 | 10 | 달성 |
| `CMakeLists.txt` 내 legacy 모듈 토큰(`wb_core/wb_ui/wb_atoms`) | 0 | 0 | 달성 |
| Phase17 게이트 세트 | 전부 PASS | PASS(10/10) | 달성 |
| 자동 검증(`build-wasm:release`, `test:cpp`, `test:smoke`) | 전부 PASS | PASS | 달성 |
| 수동 회귀 체크리스트 | 전부 확인 | 미실행 | 검증 유보 |

보조 지표:

- `module_count=10`
- `modules=wb_common,wb_density,wb_io,wb_measurement,wb_mesh,wb_platform,wb_render,wb_shell,wb_structure,wb_workspace`

## 3. R단계별 비교 평가

### R0. 기준선 동결 및 인벤토리

계획 대비 구현:
- root allowlist/ownership 기준선 유지
- 루트 코드 파일 3개, atoms 루트 코드 파일 0개 상태 고정

평가:
- 목표 달성

### R1. facade 계약 가드

계획 대비 구현:
- facade 계약 게이트 유지
- 구경로 facade include 차단 상태 유지

평가:
- 목표 달성

### R2. shell/UI 해체 이관

계획 대비 구현:
- shell/layout/reset 계약 게이트 PASS
- R6 모듈 체계(`wb_shell`/`wb_render`) 반영 후 게이트 기준 정합성 확보

평가:
- 목표 달성

### R3. render/platform/io 해체 이관

계획 대비 구현:
- render/platform/io 이관 게이트 PASS
- `wb_platform` 분리 구조를 반영한 검증 기준으로 정합성 확보

평가:
- 목표 달성

### R4. mesh 해체 이관

계획 대비 구현:
- mesh migration 게이트 PASS
- mesh 관련 legacy root 파일 제거 상태 유지

평가:
- 목표 달성

### R5. atoms 해체 이관

계획 대비 구현:
- atoms root/subtree 코드 파일 0건 유지
- legacy facade는 `workspace/legacy` 경로로 이관 유지
- R6 체계에서 `wb_atoms` 제거 상태 반영

평가:
- 목표 달성

### R6. 최종 타깃 그래프 + legacy 봉인

계획 대비 구현:
- `wb_core`, `wb_ui`, `wb_atoms` 제거
- 최종 10개 모듈 그래프 전환 완료
- 신규 게이트 2종 추가 및 전체 게이트 PASS
- `npm run build-wasm:release`, `npm run test:cpp`, `npm run test:smoke` PASS

평가:
- 자동 검증 기준 목표 달성
- 수동 회귀는 별도 최종 확인 필요

## 4. 마스터 계획 DoD 달성도(Phase17 종료 시점)

`refactoring_plan_root_legacy_dismantle_260408_53Codex.md`의 R0~R6 완료 기준 관점 평가:

| DoD 항목 | 현재 상태 | 평가 |
|---|---|---|
| 1) root allowlist(`main.cpp`, `app.cpp`, `app.h`) 유지 | 유지됨 | 달성 |
| 2) `webassembly/src/atoms` 하위 코드 파일 0 | 0건 | 달성 |
| 3) legacy facade 구경로 참조 0 | 0건 | 달성 |
| 4) legacy adapter 잔존 0 | 0건 | 달성 |
| 5) 최종 10개 모듈 그래프 전환 | 전환 완료 | 달성 |
| 6) `wb_core/wb_ui/wb_atoms` 제거 | 제거 완료 | 달성 |
| 7) Phase17 게이트 세트 PASS | PASS | 달성 |
| 8) `build-wasm`, `test:cpp`, `test:smoke` PASS | PASS | 달성 |
| 9) 수동 회귀 체크리스트 완료 | 자동 세션에서 미실행 | 검증 유보 |

## 5. 계획 대비 차이점

1. 계획서는 자동/수동 검증을 모두 종료 조건으로 제시했으나, 본 실행 증빙은 자동 검증 중심으로 완료되었다.
2. `test:cpp`, `test:smoke`는 실행 환경 권한 제약으로 1차 실패 후 권한 상승 재실행으로 PASS를 확보했다.
3. R2/R3/R5 게이트는 기존 모듈명(`wb_ui`, `wb_atoms`) 기준에서 R6 최종 모듈 구조 기준으로 보정되었다.

## 6. 현재 코드 기준 장점

1. 루트/atoms 원본 해체 목표가 수치 기준으로 명확히 달성되었다.
2. 최종 모듈 그래프가 고정되어 의존성 관리가 단순해졌다.
3. 게이트 스크립트가 R6 구조를 반영해 회귀 탐지력이 높아졌다.
4. 빌드/테스트/게이트/판정 문서가 한 세트로 연결되어 추적 가능성이 높다.

## 7. 현재 코드 기준 한계

1. 수동 UI 회귀 체크리스트는 아직 최종 완료 증빙이 없다.
2. `workspace/legacy/atoms_template_facade.h` 참조가 다수(36건)여서 후속 구조 단순화 여지는 남아 있다.
3. `AtomsTemplate::Instance()` 기반 경로는 운영상 유지되고 있어 완전한 탈-legacy 구조로 보기는 어렵다.

## 8. 향후 보완 계획(운영 백로그)

### B1. 수동 회귀 체크리스트 종료

- 목표:
  - Reset/Layout 1/2/3, Edit/Build/Data, BZ, import 시나리오 최종 확인
- 실행:
  1. 운영 UI 환경에서 시나리오별 체크리스트 수행
  2. 실패 시 재현 로그를 `phase17-root/r6/logs`에 축적

### B2. facade 의존 축소

- 목표:
  - `workspace/legacy/atoms_template_facade.*` 직접 include 경로 단계적 축소
- 실행:
  1. feature service API 경유로 호출 대체
  2. 고빈도 include 경로부터 순차 전환

### B3. CI 상시 검증 고정

- 목표:
  - Phase17 게이트 + `build-wasm` + `test:cpp` + `test:smoke`의 상시 실행
- 실행:
  1. CI 파이프라인에 R6 게이트 세트 추가
  2. 권한 제약 없는 runner에서 테스트 안정화

## 9. 최종 판정

현재 코드 기준으로 Phase17-Root는 계획된 해체/이관/그래프 전환 목표를 자동 검증 기준으로 달성했다.

최종 판정은 다음과 같다.

- `실행 완료 판정`: 완료
- `구조 완성도 판정`: 자동 기준 달성, 수동 회귀 검증 유보
- `운영 상태`: `Phase17-Root Closed (자동 기준)` / 수동 회귀 최종 사인오프 대기
