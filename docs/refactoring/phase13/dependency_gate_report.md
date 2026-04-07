# Phase 13 의존성/아키텍처 게이트 리포트

- 작성 상태: `W6 완료`
- 작성일: `2026-04-06 (KST)`
- 기준 계획서: `docs/refactoring/phase13/refactoring_phase13_runtime_hard_seal_singleton_entrypoint_260406.md`

## 1. 게이트 요약

| 게이트 | 명령 | 상태 | 근거 |
|---|---|---|---|
| G1. Runtime hard-seal 정적 게이트 | `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase13_runtime_hard_seal.ps1` | PASS | `docs/refactoring/phase13/logs/check_phase13_runtime_hard_seal_latest.txt` |
| G2. 릴리즈 빌드 게이트 | `npm run build-wasm:release` | PASS | `docs/refactoring/phase13/logs/build_phase13_latest.txt` |
| G3. C++ 테스트 게이트 | `npm run test:cpp` | PASS | `docs/refactoring/phase13/logs/unit_test_phase13_latest.txt` |
| G4. 스모크 테스트 게이트 | `npm run test:smoke` | PASS | `docs/refactoring/phase13/logs/smoke_phase13_latest.txt` |

## 2. Runtime/Shell 회귀 점검 메모

1. `App::Instance()` 호출: `0` (W1 반영 유지)
2. `Toolbar::Instance()` 호출: `0` (W2 반영 유지)
3. `FileLoader::Instance()` 호출: `0` (W3 반영 완료)
4. W5 보호 규칙 점검:
   - main ImGui 초기화 순서 게이트 PASS
   - `PrimeLegacySingletons()` 내 `FontRegistry()` 미호출 게이트 PASS
   - `ChargeDensityUI` first-grid bootstrap guard 게이트 PASS

## 3. 버그 추적 상태

- 추적 파일: `docs/refactoring/phase13/logs/bug_p13_vasp_grid_sequence_latest.md`
- 버그 ID: `P9-BUG-01`
- W6 종료 시 상태: `Deferred`
- 메모: Phase 13 범위는 singleton entrypoint 정리이며, 반복 시퀀스(`XSF(Grid) -> VASP -> XSF(Grid) -> VASP`) 전용 자동 재현 검증은 Phase 14에서 강화한다.

## 4. 결론

1. Phase 13 W6 게이트 4종 모두 PASS.
2. Runtime hard-seal 목표(`App/Toolbar/FileLoader` 진입점 정리)는 계획 기준 충족.
3. 차기 Phase 14 착수 조건은 충족되며, `P9-BUG-01`은 추적 상태 `Deferred`로 이월한다.

## 5. 사후 보완 및 재발 방지 규칙

1. 사후 확인 이슈:
   - 메뉴 선택 시 창 오픈이 누락되는 회귀(`Edit/Build/Data/Utilities`)가 발견되었고, 원인은 메뉴 프레임 내 상태 동기화 순서(`ShellState -> local -> store`)의 충돌이었다.
2. 보완 조치:
   - 메뉴 액션 처리 시 `ShellState`와 `App` local visibility/focus를 동일 프레임에 동기화하도록 수정 완료.
3. 차기 Phase 적용 규칙:
   - W6 검증 시 메뉴 창 오픈 매트릭스를 필수 체크:
     - `Edit > Atoms/Bonds/Cell`
     - `Build > Add atoms/Bravais Lattice Templates`
     - `Data > Isosurface/Surface/Volumetric/Plane` (grid 로드 상태 포함)
     - `Utilities > Brillouin Zone`
