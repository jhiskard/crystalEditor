# P9-BUG-01 Tracking Log (Phase 14)

- Bug ID: `P9-BUG-01`
- Scenario: `XSF(Grid) -> VASP -> XSF(Grid) -> VASP`
- Last updated: `2026-04-07 (KST, W6)`
- Status: `Deferred`

## 1) Carry-over Baseline

- Source baseline:
  - `docs/refactoring/phase13/logs/bug_p13_vasp_grid_sequence_latest.md`
- Phase 13 close state: `Deferred`
- Known risk:
  - mixed import 반복 시퀀스 전용 자동 재현 범위가 아직 제한적

## 2) Phase 14 W0 Registration

1. Phase 14 시작 시점에도 본 버그는 open/deferred 추적 대상이다.
2. W1~W6 동안 alias/state ownership 변경이 import 적용 순서에 회귀를 만들지 않는지 병행 점검한다.
3. W6 종료 전 자동 재현 범위를 확대하고 상태를 재판정한다.

## 3) Planned Verification Point

- Verification target: `W6`
- Decision at W6 close:
  - `Resolved`: 재현 불가 또는 안정적 자동 테스트로 커버
  - `Deferred`: 재현 가능 또는 검증 근거 불충분

## 4) W0 Note

1. W0는 기준선/추적 이관 단계로 동작 변경을 포함하지 않는다.
2. 상태는 `Deferred`를 유지한다.

## 5) W6 Re-evaluation Note

1. Phase 14 W6 게이트(`alias/friend/menu-codepath/build/test:cpp/test:smoke`)는 PASS.
2. 다만 `P9-BUG-01`의 전용 반복 시퀀스(`XSF(Grid) -> VASP -> XSF(Grid) -> VASP`) 자동 재현 테스트는 본 Phase에서 별도 추가되지 않았다.
3. 현재 근거로는 `Resolved` 판정이 불충분하므로 상태를 `Deferred`로 유지한다.
