# Phase 19 Inventory Snapshot (Latest)

- Captured at: `2026-04-27 (KST)`
- Stage: `W8 physical legacy cleanup`
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

## Current (after W8 execution)

| Metric | Current |
|---|---:|
| legacy directories | 0 |
| files with `legacy` in filename under `webassembly/src` | 0 |
| `legacy_atoms_runtime.{h,cpp}` files | 0 |
| include refs to `workspace/runtime/legacy_atoms_runtime.h` | 0 |
| include refs to `workspace/runtime/workspace_runtime_model_ref.h` | 31 |
| `wb_*.cmake` entries for `legacy_atoms_runtime.cpp` | 0 |
| `wb_*.cmake` entries for `workspace_runtime_model_ref.cpp` | 1 |
| `AtomsTemplate` symbol matches (`webassembly/src`) | 0 |
| `LegacyAtomsRuntime` symbol matches (`webassembly/src`) | 0 |
| `friend class` declarations | 0 |
| `DECLARE_SINGLETON` usages (definition included) | 1 |
| `::Instance()` calls | 0 |
| `app.cpp` LOC | 73 |

## Notes

- W8 scope focused on final physical cleanup of `legacy_atoms_runtime` path and build-graph migration.
- `workspace::legacy` namespace alias remains by design for compatibility and is handled by subsequent gate/closure stages.