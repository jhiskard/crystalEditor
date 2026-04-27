# AtomsTemplate Decomposition Progress - W5.5

- Date: `2026-04-27 (KST)`
- Status: `PASS`
- Scope: `UI render-window migration to shell/presentation/atoms`

## Implemented

1. Added `shell/presentation/atoms/AtomsWindowPresenter` thin-shim facade:
   - pending panel request routing:
     - `ApplyPendingPanelRequests(shell::domain::ShellUiState&)`
   - forced layout routing:
     - `RequestForcedBuilderWindowLayout`
     - `RequestForcedEditorWindowLayout`
     - `RequestForcedAdvancedWindowLayout`
   - atom-related split-window rendering entrypoints:
     - `RenderPeriodicTableWindow`
     - `RenderCrystalTemplatesWindow`
     - `RenderCreatedAtomsWindow`
     - `RenderBondsManagementWindow`
     - `RenderCellInformationWindow`
2. Exposed presenter via runtime composition root:
   - `WorkbenchRuntime::AtomsWindowFeature()`
3. Migrated `app.cpp` callers from direct `LegacyAtomsRuntime/AtomsTemplate` usage to presenter entrypoints:
   - pending request handling block
   - reset-layout forced window requests
   - 5 atom-related split-window render calls
4. Updated workspace module source list:
   - `webassembly/cmake/modules/wb_workspace.cmake`
   - added `shell/presentation/atoms/atoms_window_presenter.cpp`

## Verification

1. `build-wasm:release`: `PASS`
2. `atomsTemplate.<W5.5 method set>` references outside `atoms_window_presenter.cpp`: `0`
3. `LegacyAtomsRuntime().<W5.5 method set>` references outside `atoms_window_presenter.cpp`: `0`
4. `atomsWindowPresenter.<W5.5 methods>` call sites in `app.cpp`: `9`
5. `AtomsWindowPresenter` wrapper methods: `9`

## Notes

- W5.5 is in thin-shim mode: the presenter delegates to `workspace::legacy::LegacyAtomsRuntime()` while caller ownership is moved to shell presentation boundary.
- W5.6 should migrate XSF/CHGCAR bridge paths (`io/application/import_*`) before W5.7 physical legacy file removal.
