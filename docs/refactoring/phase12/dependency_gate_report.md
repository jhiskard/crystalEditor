# Phase 12 의존성/아키텍처 게이트 리포트

- 생성일: `2026-04-06 (KST)`
- 검증 범위: Phase 12 W6 정적/동적 게이트 + W6 사후 보완(Layout 버튼 복구)
- 기준 계획서: `docs/refactoring/phase12/refactoring_phase12_compatibility_facade_seal_ci_gate_260403.md`

## 1. 게이트 요약

| 게이트 | 명령 | 상태 | 근거 |
|---|---|---|---|
| G1. 아키텍처 seal 게이트 | `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase12_architecture_seal.ps1` | PASS | `docs/refactoring/phase12/logs/check_phase12_architecture_seal_latest.txt` |
| G2. 릴리즈 빌드 게이트 | `npm run build-wasm:release` | PASS | `docs/refactoring/phase12/logs/build_phase12_latest.txt` |
| G3. C++ 단위 테스트 게이트 | `npm run test:cpp` | PASS | `docs/refactoring/phase12/logs/unit_test_phase12_latest.txt` |
| G4. 브라우저 스모크 게이트 | `npm run test:smoke` | PASS | `docs/refactoring/phase12/logs/smoke_phase12_latest.txt` |
| G5. Phase6 통합 파이프라인 게이트 | `npm run test:phase6` | PASS | 기존 W6 실행 로그 |

## 2. 런타임 이슈 보완 내역

### 2.1 GL 초기화 타이밍 오류(`createTexture`)

- 관찰 증상:
  - `TypeError: Cannot read properties of undefined (reading 'createTexture')`
- 적용 조치:
  1. `PrimeLegacySingletons()` 호출 시점을 `ImGui::CreateContext()` 이후로 이동
  2. warm-up 범위를 `AppController()`로 제한
  3. ImGui/GL 컨텍스트 선행 조건 주석 보강
- 관련 파일:
  - `webassembly/src/main.cpp`
  - `webassembly/src/shell/runtime/workbench_runtime.cpp`
  - `webassembly/src/shell/runtime/workbench_runtime.h`

### 2.2 Layout 1/2/3 버튼 미동작 회귀(`P12-BUG-02`)

- 관찰 증상:
  - 메뉴의 `Layout 1/2/3` 버튼 클릭 시 도킹 레이아웃이 적용되지 않음
- 원인 분석:
  - `pendingLayoutPreset`는 one-shot 명령인데, `syncShellStateToStore()`에서 App 로컬 캐시(`m_PendingLayoutPreset`)를 store로 역기록하여 같은 프레임에 요청값이 `None`으로 덮어써짐
- 적용 조치:
  1. `syncShellStateToStore()`에서 `pendingLayoutPreset` 역기록 제거
  2. 레이아웃 적용 직후 store의 `pendingLayoutPreset`를 `None`으로 consume 처리
  3. `App::RequestLayout1()`을 store 직접 기록 방식으로 정합화
- 관련 파일:
  - `webassembly/src/app.cpp`
- 보완 로그:
  - `docs/refactoring/phase12/bug_p12_layout_button_latest.md`

## 3. 사후 검증(2026-04-06)

1. `npm run build-wasm:release` 재실행: PASS
2. `npm run test:smoke` 재실행: PASS
3. Layout 버튼 수정은 빌드/스모크 회귀 기준에서 이상 없음

## 4. 버그 추적 상태

- `P9-BUG-01` (`XSF(Grid) -> VASP -> XSF(Grid) -> VASP`): `Deferred`
  - 추적 파일: `docs/refactoring/phase12/logs/bug_p12_vasp_grid_sequence_latest.md`
- `P12-BUG-02` (Layout 버튼 미동작): `Resolved`
  - 추적 파일: `docs/refactoring/phase12/bug_p12_layout_button_latest.md`

## 5. 결론

- Phase 12의 아키텍처/의존성 게이트는 PASS 상태를 유지한다.
- W6 사후 보완(Layout 버튼 복구)까지 반영한 기준으로 종료 산출문서를 갱신했다.
- 마스터 계획서 기준 실행 phase는 Phase 7~12이며, `Phase 13`은 계획 범위 밖(N/A)으로 기록한다.


