# P9-BUG-01 Tracking Log (Phase 13)

- Bug ID: `P9-BUG-01`
- Scenario: `XSF(Grid) -> VASP -> XSF(Grid) -> VASP`
- Last updated: `2026-04-06 (KST, W0)`
- Status: `Deferred`

## 1) Carry-over Baseline

- Source baseline:
  - `docs/refactoring/phase12/logs/bug_p12_vasp_grid_sequence_latest.md`
- Phase 12 close state: `Deferred`
- Known risk:
  - repeated mixed import sequence is not fully automated in smoke tests yet
  - final status requires dedicated replay coverage

## 2) Phase 13 W0 Registration

1. This bug remains open at Phase 13 start.
2. W1~W6 must keep this item visible in logs/checklists/gate reports.
3. Runtime hard-seal changes must not regress import apply sequence behavior.

## 3) Planned Verification Point

- Verification target: `W6`
- Decision at W6 close:
  - `Resolved`: sequence no longer reproduces or is covered by stable automated test
  - `Deferred`: still reproducible or still not fully verifiable

## 4) W0 Note

1. W0 only fixed baseline and Phase 13 migration scope.
2. No direct import sequence semantics change is applied in W0.
3. Status remains `Deferred`.
