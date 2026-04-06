# P12-BUG-02 추적 로그: Layout 1/2/3 버튼 미동작

- Bug ID: `P12-BUG-02`
- 최초 보고: `2026-04-06 (KST)`
- 최종 갱신: `2026-04-06 (KST)`
- 상태: `Resolved`
- 관련 계획서: `docs/refactoring/phase12/refactoring_phase12_compatibility_facade_seal_ci_gate_260403.md`

## 1. 증상

1. 메뉴의 `Layout 1/2/3` 버튼 클릭 시 도킹 레이아웃이 변경되지 않음
2. UI 이벤트는 발생하지만 적용 프레임에서 preset이 소실됨

## 2. 원인 분석

1. 버튼 클릭 시 controller 경유로 store의 `pendingLayoutPreset`에는 정상 기록됨
2. 같은 프레임의 `syncShellStateToStore()`에서 App 로컬 값(`m_PendingLayoutPreset`)이 store로 역기록됨
3. 그 결과 one-shot 명령(`pendingLayoutPreset`)이 적용 전에 `None`으로 덮어써져 레이아웃 적용 분기를 통과하지 못함

## 3. 수정 내용

대상 파일:
- `webassembly/src/app.cpp`

적용 조치:
1. `syncShellStateToStore()`에서 `pendingLayoutPreset` 역동기화 로직 제거
2. 레이아웃 적용 완료 후 store의 `pendingLayoutPreset`를 `None`으로 consume 처리
3. `App::RequestLayout1()`을 store 직접 기록 방식으로 정합화

## 4. 검증 결과

1. `npm run build-wasm:release` 실행 결과: PASS (`2026-04-06`)
2. `npm run test:smoke` 실행 결과: PASS (`2026-04-06`)

## 5. 결론

- `P12-BUG-02`는 조치 완료 및 회귀 검증 PASS로 `Resolved` 처리한다.

