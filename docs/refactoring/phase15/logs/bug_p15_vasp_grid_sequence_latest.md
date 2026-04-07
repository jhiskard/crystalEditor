# P9-BUG-01 Tracking Log (Phase 15)

- Bug ID: `P9-BUG-01`
- Scenario: `XSF(Grid) -> VASP -> XSF(Grid) -> VASP`
- Last updated: `2026-04-07 (KST, W0)`
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

## 4) W0 Note

1. W0는 기준선/추적 이관 단계로 동작 변경을 포함하지 않는다.
2. 상태는 `Deferred`를 유지한다.
