# Phase 13 세부 작업계획서: Runtime Hard Seal + Singleton Entrypoint 정리

작성일: `2026-04-06 (KST)`  
기준 계획:
- `docs/refactoring/refactoring_plan_architecture_completion_phase13plus_260406.md`
- `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md` (3. 목표 아키텍처)
선행 판정:
- `docs/refactoring/phase12/go_no_go_phase13.md` (Phase 12 Closed, 후속 개선 트랙 착수)
대상 범위:
- `webassembly/src/app.*`
- `webassembly/src/toolbar.*`
- `webassembly/src/file_loader.*`
- `webassembly/src/platform/wasm/*`
- `webassembly/src/shell/runtime/*`
- `scripts/refactoring/*`
- `docs/refactoring/phase13/*`
진행 상태: `W6 완료`

## 0. 착수 배경과 고정 전제

### 0.1 착수 배경
1. Phase 7~12는 게이트 기준으로 종료되었으나, 마스터 계획서의 엄격 DoD는 부분 달성으로 판정되었다.
2. 결과보고서(`refactoring_result_report_phase7_12_260406.md`) 기준 잔여 과제 중 우선순위가 가장 높은 항목은 runtime 외부 singleton entrypoint 정리다.
3. Phase 13은 후속 계획서에서 정의한 첫 단계로, `App/Toolbar/FileLoader` singleton 의존 제거를 목표로 한다.

### 0.2 고정 전제
1. 기존 기능 동작(파일 열기, 메뉴 액션, 레이아웃/상태 동기화)을 유지하면서 경계만 개선한다.
2. `WorkbenchRuntime`를 단일 composition root로 유지한다.
3. 임시 bridge/shim은 `@note Temporary compatibility path`와 제거 목표 Phase를 명시한다.
4. Phase 7~12 보호 규칙은 유지한다.
   - ImGui context 이후 폰트 초기화 순서
   - `PrimeLegacySingletons()`에서 `FontRegistry()` 미호출
   - `ChargeDensityUI` first-grid bootstrap guard 유지

## 1. 목표와 비목표

### 1.1 목표
1. `App::Instance()`, `Toolbar::Instance()`, `FileLoader::Instance()` 호출을 runtime 경유 API로 대체한다.
2. `App`, `Toolbar`, `FileLoader`의 singleton 선언 제거 또는 runtime 내부 제한 형태로 축소한다.
3. `app.cpp`의 orchestration 경계를 명확히 하고 singleton 직접 진입점을 제거한다.
4. Phase 13 정적 게이트 스크립트를 도입해 회귀를 차단한다.

### 1.2 비목표
1. 신규 UI/기능 추가
2. 구조/측정/밀도 도메인 책임 대규모 재분해(Phase 14 이후 범위)
3. 모듈 타깃(`wb_atoms` 제거) 재편(Phase 16 범위)

## 2. W0 기준선(베이스라인)

W0에서 아래 항목을 고정한다.

| 항목 | 기준선(2026-04-06) | Phase 13 목표 |
|---|---:|---:|
| `DECLARE_SINGLETON` 실사용 선언 수 | 7 | 4 이하 |
| `::Instance()` 총 호출(`*.cpp`) | 104 | 80 이하 |
| `App::Instance()` 호출 | 1 | 0 |
| `Toolbar::Instance()` 호출 | 1 | 0 |
| `FileLoader::Instance()` 호출 | 3 | 0 |
| feature app/domain core singleton 호출(allowlist 외) | 0 | 0 유지 |

W0 산출 로그:
- `docs/refactoring/phase13/logs/runtime_singleton_inventory_phase13_latest.md`
- `docs/refactoring/phase13/logs/bug_p13_vasp_grid_sequence_latest.md`

## 3. 설계 원칙

1. 진입점 이전 우선순위:
   - static singleton 진입점 -> runtime public API -> service/query 계약 고정
2. 호출 경로 정리 원칙:
   - UI/presentation 계층은 controller/runtime API만 호출
   - facade 내부 self-dispatch는 제거 대상이며 임시 잔존 시 명시적 주석을 둔다
3. 상태 정합성 원칙:
   - one-shot 상태는 store 기준으로 소비하고 역동기화 회귀를 금지한다

## 4. 작업 단위(WBS)

### W0. 기준선 고정 + 추적 로그 등록
- singleton/`::Instance()` 잔존 지표 재수집
- `P9-BUG-01` 추적 상태를 Phase 13 로그로 이관

산출물:
- `logs/runtime_singleton_inventory_phase13_latest.md`
- `logs/bug_p13_vasp_grid_sequence_latest.md`

완료 기준:
- W1~W6 비교 가능한 기준선이 문서화됨

### W1. `App` singleton 제거/격리
- `app.h`의 singleton 선언 정리
- static API를 runtime 소유 객체 경유로 전환
- `app.cpp`의 `App::Instance()` 호출 제거

예상 영향 파일:
- `webassembly/src/app.h`
- `webassembly/src/app.cpp`
- `webassembly/src/shell/runtime/workbench_runtime.*`

완료 기준:
- `App::Instance()` 호출 0

### W2. `Toolbar` singleton 제거/격리
- `toolbar.h/.cpp` singleton 진입점 제거
- toolbar 명령 경로를 shell/runtime presentation 경유로 정렬

예상 영향 파일:
- `webassembly/src/toolbar.h`
- `webassembly/src/toolbar.cpp`
- `webassembly/src/shell/runtime/workbench_runtime.*`

완료 기준:
- `Toolbar::Instance()` 호출 0

### W3. `FileLoader` entrypoint 전환
- `FileLoader::Instance()` static entrypoint 제거
- import 진입을 runtime + workflow service 경유로 통일

예상 영향 파일:
- `webassembly/src/file_loader.h`
- `webassembly/src/file_loader.cpp`
- `webassembly/src/io/application/*`
- `webassembly/src/platform/wasm/*`

완료 기준:
- `FileLoader::Instance()` 호출 0

### W4. Runtime/Binding 계약 보강 + 주석 정책 v2 적용
- runtime public API 계약 정리
- 변경된 public API Doxygen 보강
- 임시 경로에 `Temporary compatibility path` 주석 및 제거 목표 Phase 명시

완료 기준:
- 변경 public API 주석 누락 0

### W5. 정적 게이트 스크립트 도입
- `scripts/refactoring/check_phase13_runtime_hard_seal.ps1` 작성
- 필수 체크:
  1. `App/Toolbar/FileLoader` singleton 선언 0 또는 명시 allowlist
  2. `App::Instance()`, `Toolbar::Instance()`, `FileLoader::Instance()` 호출 0
  3. feature app/domain core singleton 호출(allowlist 외) 0
  4. Phase 7~12 보호 규칙 유지
  5. bug 추적 로그 존재 + 상태 태그

산출물:
- `scripts/refactoring/check_phase13_runtime_hard_seal.ps1`
- `docs/refactoring/phase13/logs/check_phase13_runtime_hard_seal_latest.txt`

완료 기준:
- 반복 실행 시 PASS

### W6. 빌드/테스트/게이트 + 종료 문서
- 실행 게이트:
  - `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase13_runtime_hard_seal.ps1`
  - `npm run build-wasm:release`
  - `npm run test:cpp`
  - `npm run test:smoke`
- 종료 문서:
  - `docs/refactoring/phase13/dependency_gate_report.md`
  - `docs/refactoring/phase13/go_no_go_phase14.md`

로그 산출 경로:
- `docs/refactoring/phase13/logs/check_phase13_runtime_hard_seal_latest.txt`
- `docs/refactoring/phase13/logs/build_phase13_latest.txt`
- `docs/refactoring/phase13/logs/unit_test_phase13_latest.txt`
- `docs/refactoring/phase13/logs/smoke_phase13_latest.txt`

완료 기준:
- W6 게이트 전 항목 PASS

## 5. 완료 기준(DoD)

1. `App::Instance()`, `Toolbar::Instance()`, `FileLoader::Instance()` 호출이 0이다.
2. `App/Toolbar/FileLoader` singleton 선언이 제거되었거나 문서화된 runtime 내부 shim만 남는다.
3. Phase 13 정적 게이트, 빌드, C++ 테스트, 스모크 테스트가 모두 PASS다.
4. 변경 public API의 Doxygen 주석이 누락 없이 반영된다.
5. `P9-BUG-01` 추적 로그가 Phase 13 문서 패키지에 유지된다.

## 6. 주석 정책 적용 메모 (v2)

1. public API 변경 시 `@brief` + 경계 계약(`@details` 또는 `@note`) 필수
2. 임시 호환 경로는 아래를 함께 기록
   - `@note Temporary compatibility path`
   - 제거 목표 Phase (`remove in Phase XX`)
   - 추적 ID (`ARCH-DEBT-XXX`)

## 7. Git 운영 정책 적용 메모 (v2)

1. 작업 브랜치: `refactor/phase13-runtime-hard-seal`
2. WBS 단위 로컬 커밋, 메시지에 `phase13 Wn` 포함
3. 원격 push는 Phase 13 완료 선언 이후에 수행
4. 종료 후 회귀 발생 시 `hotfix/phase13-*` 브랜치로 분리

## 8. 리스크와 대응

| ID | 리스크 | 수준 | 대응 |
|---|---|---|---|
| P13-R1 | singleton 제거 중 초기화 순서 회귀 | High | runtime/main 순서 게이트 유지 + smoke |
| P13-R2 | import 진입점 전환 중 파일 로딩 회귀 | Medium | parser/apply 경로 단위 테스트 + 스모크 |
| P13-R3 | 임시 shim 누적 | Medium | W5 게이트에서 만료 임시 경로 검사 |

## 8.1 사후 보완 메모 (2026-04-06)

1. 증상:
   - `Edit > Atoms/Bonds/Cell`
   - `Build > Add atoms/Bravais Lattice Templates`
   - `Data` 하위 메뉴 및 `Utilities > Brillouin Zone`
   선택 시 해당 창이 열리지 않는 회귀가 확인되었다.
2. 원인:
   - 메뉴 액션에서 `ShellState`를 변경한 직후 같은 프레임의 `syncShellStateToStore()`가 stale local 상태로 store를 재덮어씀.
3. 수정 원칙:
   - 메뉴 액션 프레임에서 `ShellState`와 `App` local visibility/focus를 동시 갱신.
4. 재발 방지:
   - 차기 Phase부터 W6 검증에 메뉴 창 오픈 회귀 체크를 필수로 포함.
   - 공통 정책 문서(`refactoring_plan_architecture_completion_phase13plus_260406.md`, 4.4) 적용.

## 9. 참조 문서

- `docs/refactoring/refactoring_plan_architecture_completion_phase13plus_260406.md`
- `docs/refactoring/refactoring_result_report_phase7_12_260406.md`
- `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md`
- `docs/refactoring/phase12/refactoring_phase12_compatibility_facade_seal_ci_gate_260403.md`
- `docs/refactoring/phase12/dependency_gate_report.md`

## 10. 진행 체크리스트

- [x] W0 기준선/중간 산출문서 등록
- [x] W1 `App` singleton 정리
- [x] W2 `Toolbar` singleton 정리
- [x] W3 `FileLoader` entrypoint 정리
- [x] W4 runtime/binding 계약 및 주석 보강
- [x] W5 정적 게이트 스크립트 도입
- [x] W6 빌드/테스트/종료 문서 완료
