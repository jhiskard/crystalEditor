# Dependency Gate Report - W8

- Date: `2026-04-27 (KST)`
- Stage: `W8 physical legacy cleanup`
- Branch: `refactor/phase19-legacy-complete-dismantle`

## Gate Results

1. Physical cleanup checks: `PASS`
   - legacy directories in `webassembly/src`: `0`
   - `legacy_atoms_runtime.{h,cpp}` files: `0`
   - include refs to `workspace/runtime/legacy_atoms_runtime.h`: `0`
   - `wb_*.cmake` entries for `legacy_atoms_runtime.cpp`: `0`
2. `build-wasm:debug`: `PASS`
3. `build-wasm:release`: `PASS`

## Scope Notes

- W8 plan was reconciled to current branch state before execution:
  - legacy directories and `legacy_viewer_facade` were already removed before W8 run.
  - actual remaining physical target was `workspace/runtime/legacy_atoms_runtime.{h,cpp}`.
- `legacy_atoms_runtime` path was replaced by `workspace_runtime_model_ref` path:
  - new include refs to `workspace/runtime/workspace_runtime_model_ref.h`: `31`
  - `wb_workspace.cmake` now references `workspace_runtime_model_ref.cpp`.

## Additional Validation

1. `check:phase19:singleton-zero`: `PASS`
2. `check:phase19:app-cpp-size`: `PASS` (`73 <= 400`)

## Implementation Note

- During debug build verification, a pre-existing debug-only compile issue was encountered and fixed:
  - `webassembly/src/workspace/runtime/atoms_template_facade.cpp`
  - `AtomType::ORIGINAL` -> `atoms::domain::AtomType::ORIGINAL`

## Summary

- Updated W8 DoD is satisfied in the current codebase state.
- Remaining physical legacy runtime path is removed and build graph no longer references legacy runtime source entries.