# Phase 19 Inventory Snapshot (Latest)

- Captured at: `2026-04-22 (KST)`
- Scope: `webassembly/src`
- Baseline source: `refactoring_plan_phase19_legacy_complete_dismantle_codex_260421_v2.md`

## Baseline (W0 target freeze)

| Metric | Baseline |
|---|---:|
| legacy directories | 5 |
| legacy files in legacy directories | 12 |
| legacy files outside legacy directories | 2 |
| legacy target LOC | 7,872 |
| `LegacyAtomsRuntime()` caller files | 14 |
| `workspace/legacy/` token files | 32 |
| `friend class` declarations | 1 |
| `DECLARE_SINGLETON` usages | 4 |
| `::Instance()` calls | 48 |
| `app.cpp` LOC | 1,430 |

## Current (after W4 implementation)

| Metric | Current |
|---|---:|
| legacy directories | 1 |
| legacy files in legacy directories | 4 |
| legacy files outside legacy directories | 0 |
| `friend class` declarations | 0 |
| `DECLARE_SINGLETON` usages | 4 |
| `::Instance()` calls | 48 |
| `measurement/application` VTK token refs | 0 |
| `app.cpp` LOC | 1,281 |

## Notes

- W0~W4 scope excludes `workspace/legacy` physical deletion (W5/W8 scope).
- `legacy` residuals are now confined to `webassembly/src/workspace/legacy` only.
