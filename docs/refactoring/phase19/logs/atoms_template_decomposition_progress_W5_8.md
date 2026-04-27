# AtomsTemplate Decomposition Progress - W5.8

- Date: `2026-04-27 (KST)`
- Status: `PASS`
- Scope: `W5 workspace CMake cleanup after runtime path migration`

## Implemented

1. Updated workspace module source list:
   - `webassembly/cmake/modules/wb_workspace.cmake`
   - removed legacy path entries:
     - `webassembly/src/workspace/legacy/atoms_template_facade.cpp`
     - `webassembly/src/workspace/legacy/legacy_atoms_runtime.cpp`
   - added runtime path entries:
     - `webassembly/src/workspace/runtime/atoms_template_facade.cpp`
     - `webassembly/src/workspace/runtime/legacy_atoms_runtime.cpp`
2. Retained previously added W5.5 presenter source entry in `wb_workspace.cmake`:
   - `webassembly/src/shell/presentation/atoms/atoms_window_presenter.cpp`

## Verification

1. `rg "workspace/legacy/" webassembly/cmake/modules/wb_workspace.cmake`: `0 matches`
2. `build-wasm:release`: `PASS`

## Notes

- W5.8 path cleanup is complete for workspace CMake wiring.
- Although legacy path references were removed, runtime compatibility symbols (`AtomsTemplate`, `LegacyAtomsRuntime`) are still present and require follow-up dismantling for full W5 DoD closure.
