# P9-BUG-01 Tracking Log (Phase 15)

- Bug ID: `P9-BUG-01`
- Scenario: `XSF(Grid) -> VASP -> XSF(Grid) -> VASP`
- Last updated: `2026-04-07 (KST, W6)`
- Status: `Deferred`

## 1) Carry-over Baseline

- Source baseline:
  - `docs/refactoring/phase14/logs/bug_p14_vasp_grid_sequence_latest.md`
- Phase 14 close state: `Deferred`
- Known risk:
  - 일반 smoke PASS와 별개로 반복 시퀀스 전용 자동 재현 근거는 부족함

## 2) Phase 15 W0 Registration

1. Phase 15 시작 시점에도 본 버그는 open/deferred 추적 대상이다.
2. render/platform 경계 정리 중 import/표시 전환 순서 회귀를 병행 점검한다.
3. W6 종료 전 상태를 `Resolved` 또는 근거 있는 `Deferred`로 재판정한다.

## 3) Planned Verification Point

- Verification target: `W6`
- Decision at W6 close:
  - `Resolved`: 반복 시퀀스 재현 불가 또는 자동 검증 근거 확보
  - `Deferred`: 재현 가능 또는 검증 근거 불충분

## 4) W6 판정 근거

1. Phase 15 W6 게이트 실행 결과:
   - `check_phase15_render_platform_isolation.ps1` PASS
   - `npm run build-wasm:release` PASS
   - `npm run test:cpp` PASS
   - `npm run test:smoke` PASS
2. 다만 `XSF(Grid) -> VASP -> XSF(Grid) -> VASP` 반복 시퀀스 전용 자동 재현 케이스는 아직 테스트 스위트에 포함되지 않았다.
3. 따라서 W6 종료 시점 상태는 `Deferred`를 유지한다(근거 있는 이월).

## 5) 차기 액션 (Phase 16+)

1. 반복 시퀀스 전용 자동화 시나리오를 smoke 또는 dedicated e2e로 추가한다.
2. 자동화 근거 확보 후 `Resolved` 여부를 재판정한다.
