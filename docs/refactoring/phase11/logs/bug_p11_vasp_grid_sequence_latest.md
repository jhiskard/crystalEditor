# P9-BUG-01 Tracking Log (Phase 11)

- Bug ID: `P9-BUG-01`
- Scenario: `XSF(Grid) -> VASP -> XSF(Grid) -> VASP`
- Last updated: `2026-04-03 (KST)`
- Status: `Deferred`

## 1) Baseline

- Source baseline: `docs/refactoring/phase10/logs/bug_p10_vasp_grid_sequence_latest.md`
- Known symptom:
  - After VASP load, stale grid context can remain and not be fully cleared.
  - In repeated sequence runs, stale grid state can survive between imports.

## 2) Phase 11 W0 Registration

1. This bug remains open while shell/panel objectization is in progress.
2. Phase 11 W1~W6 must keep this item visible in checklists and gate reports.
3. Any behavior change in import UI routing (menu/controller) must be checked against this scenario.

## 3) Planned Verification Point

- Verification target phase step: `W6` (build/test/smoke/go-no-go)
- Expected decision state at W6 close:
  - `Resolved`, if scenario no longer reproduces
  - `Deferred`, if reproduction still occurs and requires follow-up in next phase

## 4) W1~W2 Update

1. W1/W2 changed shell state and command routing layers only.
2. Import transaction logic was not modified in this step.
3. Bug status remains `Deferred` and is still a required W6 verification item.

## 5) W3~W5 Update

1. W3/W4 changed shell-panel runtime ownership and menu/toolbar routing boundaries.
2. Import transaction/data-clear sequence logic was not directly changed in W3~W5 scope.
3. Status remains `Deferred` and is carried forward to W6 smoke/manual verification.
