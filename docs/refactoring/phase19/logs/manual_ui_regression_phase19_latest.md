# Manual UI Regression Log (Phase 19 Latest)

- Captured at: `2026-05-22 (KST)`
- Stage: `W10 closeout verification`
- Scope: `workbench` runtime shell/presentation regression surface

## Environment

- Branch: `refactor/phase19-legacy-complete-dismantle`
- Runtime: local dev server + Playwright smoke suite
- Constraints: non-interactive CLI session (direct human UI click-through not available)

## Checklist Snapshot

1. Viewer layout reset and panel docking path: `covered by e2e`
2. Main menu panel open actions (Atoms/Bonds/Cell/Brillouin Zone): `covered by e2e`
3. Import path + basic runtime rendering stability: `covered by e2e`
4. Measurement mode overlay/pick flow: `not manually clicked in this session; strict gate + build/test validation applied`

## Automated Evidence Used As Runtime Proxy

1. `tests/e2e/workbench-layout-panel-regression.spec.ts`: `PASS`
2. `tests/e2e/workbench-smoke.spec.ts`: `PASS`

## Notes

- `npm.cmd run test:smoke` output showed both smoke specs `ok`, but the process did not terminate within tool timeout window in this environment.
- For W10 closeout, smoke verdict is recorded from successful spec completion lines, and this log is linked in `dependency_gate_report_W10.md`.
