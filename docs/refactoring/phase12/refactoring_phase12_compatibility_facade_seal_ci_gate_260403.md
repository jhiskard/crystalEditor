# Phase 12 상세 작업계획서: Compatibility Facade 제거 + 모듈 Seal + Architecture/CI 게이트

작성일: `2026-04-03 (KST)`  
최종 갱신: `2026-04-06 (KST)`  
기준 계획: `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md` (Phase 12)  
선행 의사결정: **GO** (`docs/refactoring/phase11/go_no_go_phase12.md`)  
대상 범위: `webassembly/src/*`, `webassembly/cmake/modules/*`, `webassembly/tests/*`, `scripts/refactoring/*`, `docs/refactoring/phase12/*`  
진행 상태: `W0~W6 완료 + W6 사후 보완(레이아웃 버튼 복구) 반영`

## 0. 착수 배경과 고정 전제

### 0.1 착수 배경
1. Phase 11에서 shell/panel 객체화와 controller 경유 구조가 완료되었고 W6 게이트가 PASS로 종료되었다.
2. Phase 12의 목적은 리팩터링 마무리 단계로서 compatibility facade 제거, 모듈 경계 seal, 정적/동적 게이트 고정을 완료하는 것이다.
3. Phase 11 종료 시점의 잔여 리스크(`P12-R1~R3`)를 W0~W6에서 추적하고 문서화한다.

### 0.2 고정 전제
1. Composition root는 `WorkbenchRuntime` 하나로 유지한다.
2. shell/workspace 상태는 store를 단일 진실원천(single source of truth)으로 유지한다.
3. 임시 shim/bridge는 `@note`로 제거 조건과 제거 phase를 명시한다.
4. 아키텍처 규칙은 문서뿐 아니라 스크립트 게이트로 검증한다.

## 1. 목표와 비목표

### 1.1 목표
1. `App`, `Toolbar`, `FileLoader`, `MeshManager`, `AtomsTemplate`, `VtkViewer`의 호환 facade/직접 singleton 의존을 축소 또는 제거한다.
2. feature/application/domain 경계에서 `::Instance()` 직접 호출을 차단하고 runtime/service 경유를 고정한다.
3. CMake 모듈 그래프를 비순환(acyclic)으로 유지하고 게이트 스크립트로 회귀를 차단한다.
4. 빌드/테스트/아키텍처 게이트를 W6 패키지로 고정한다.

### 1.2 비목표
1. 신규 사용자 기능 추가
2. UX 전면 개편
3. 성능 최적화 전용 작업

## 2. W0 기준선(베이스라인)

W0에서 아래 항목을 수집하고 로그를 고정한다.

- facade/singleton/`::Instance()` 잔존 위치
- 모듈 의존 그래프
- `P9-BUG-01` 추적 상태

W0 산출 로그:
- `docs/refactoring/phase12/logs/facade_inventory_phase12_latest.md`
- `docs/refactoring/phase12/logs/module_graph_phase12_latest.md`
- `docs/refactoring/phase12/logs/bug_p12_vasp_grid_sequence_latest.md`

## 3. 설계 원칙

1. 호출 이관 순서는 `호출부 이전 -> 계약 고정 -> 레거시 경로 제거`를 따른다.
2. one-shot 명령 상태(예: pending preset)는 store 기준으로 소비/초기화한다.
3. UI 계층은 orchestration 중심으로 유지하고 feature 로직 직접 제어를 최소화한다.
4. 공개 API는 Doxygen 주석으로 책임과 경계 조건을 명시한다.

## 4. 작업 단위(WBS)

### W0. 기준선 고정 + 리스크 추적 등록
- facade/singleton/alias 현황 수집
- `P9-BUG-01` Phase 12 추적 로그 연계
- 모듈 의존 그래프 수집

### W1. Shell/Runtime 잔여 singleton 정리
- `App`, `Toolbar` 진입 경계 정리
- 메뉴/툴바 액션의 controller 경유 고정

### W2. Import/Mesh/Atoms 경로 이관
- `FileLoader`/`MeshManager`/`AtomsTemplate`의 직접 의존 축소
- workflow/service 경유 호출 정리

### W3. Render 경계 Seal
- non-render 계층의 `VtkViewer` 직접 의존 축소
- render application/infrastructure 책임 경계 명확화

### W4. 빌드 타깃 재정렬 + 모듈 그래프 고정
- CMake 타깃 의존 정리
- 순환 의존 차단

### W5. Architecture Gate 스크립트 고정
- `scripts/refactoring/check_phase12_architecture_seal.ps1` 작성/보완
- 아키텍처 회귀 검사 항목 자동화

### W6. 빌드/테스트/의사결정 패키지 마감
- 게이트 실행:
  - `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase12_architecture_seal.ps1`
  - `npm run build-wasm:release`
  - `npm run test:cpp`
  - `npm run test:smoke`
- 종료 문서:
  - `docs/refactoring/phase12/dependency_gate_report.md`
  - `docs/refactoring/phase12/go_no_go_phase13.md` (파일명 호환용 최종 종료 판정 문서, Phase 13 착수 문서 아님)

### W6 사후 보완(2026-04-06): Layout 1/2/3 버튼 복구
- 증상:
  - `Layout 1/2/3` 버튼 클릭 시 도킹 레이아웃이 적용되지 않음
- 원인:
  - `pendingLayoutPreset`가 one-shot 명령임에도 `syncShellStateToStore()`에서 역동기화되어 같은 프레임에 `None`으로 덮어써짐
- 조치:
  1. `syncShellStateToStore()`에서 `pendingLayoutPreset` 역기록 제거
  2. 레이아웃 적용 직후 store의 `pendingLayoutPreset`를 consume(`None`) 처리
  3. `App::RequestLayout1()`을 store 직접 기록 방식으로 정합화
- 영향 파일:
  - `webassembly/src/app.cpp`
- 검증:
  - `npm run build-wasm:release` PASS (`2026-04-06`)
  - `npm run test:smoke` PASS (`2026-04-06`)
- 보완 로그:
  - `docs/refactoring/phase12/bug_p12_layout_button_latest.md`

## 5. 완료 기준(DoD)

1. Phase 12 대상 facade/singleton 경계가 계획 수준으로 정리되었다.
2. architecture gate, build, C++ test, smoke test가 재현 가능하게 PASS이다.
3. `P9-BUG-01` 상태가 W6 문서에 명시되어 있다.
4. 레이아웃 버튼 회귀(`P12-BUG-02`)가 해결 상태로 문서화되어 있다.
5. 종료 산출문서가 최신 기준으로 정리되어 있다.

## 6. 산출문서 목록

- `docs/refactoring/phase12/dependency_gate_report.md`
- `docs/refactoring/phase12/go_no_go_phase13.md` (최종 종료 판정)
- `docs/refactoring/phase12/logs/bug_p12_vasp_grid_sequence_latest.md`
- `docs/refactoring/phase12/bug_p12_layout_button_latest.md`
- `docs/refactoring/phase12/logs/check_phase12_architecture_seal_latest.txt`
- `docs/refactoring/phase12/logs/build_phase12_latest.txt`
- `docs/refactoring/phase12/logs/unit_test_phase12_latest.txt`
- `docs/refactoring/phase12/logs/smoke_phase12_latest.txt`

## 7. 진행 체크리스트

- [x] W0 기준선/리스크 로그 고정
- [x] W1 shell/runtime 경계 정리
- [x] W2 import/mesh/atoms 호출 이관
- [x] W3 render 경계 seal
- [x] W4 빌드 타깃/모듈 그래프 정리
- [x] W5 architecture gate 고정
- [x] W6 빌드/테스트/의사결정 문서 완료
- [x] W6 사후 보완: Layout 1/2/3 버튼 복구 및 로그 반영


