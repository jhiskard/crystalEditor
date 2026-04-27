# AtomsTemplate Decomposition Progress - W5.4

- Date: `2026-04-24 (KST)`
- Status: `PASS`
- Scope: `Structure lifecycle/BZ migration to StructureLifecycleService`

## Implemented

1. Added `structure/application/StructureLifecycleService` facade and runtime exposure:
   - service API: `RegisterStructure`, `RemoveStructure`, `RemoveUnassignedData`
   - BZ entrypoints: `RenderBrillouinZonePlotWindow`, `EnterBZPlotMode`, `ExitBZPlotMode`, `IsBZPlotMode`
   - runtime accessor: `WorkbenchRuntime::StructureLifecycleFeature()`
2. Migrated structure lifecycle call sites from direct legacy/atomsTemplate usage to `StructureLifecycleService` in:
   - `mesh/presentation/model_tree_dialogs.cpp` (structure delete confirm)
   - `io/application/import_runtime_port.cpp` (register/remove/unassigned)
3. Migrated BZ render entrypoint from direct `atomsTemplate` call to `StructureLifecycleService` in:
   - `app.cpp`
4. Updated `structure/application/structure_service.cpp` lifecycle compatibility methods to delegate via `StructureLifecycleService`.

## Verification

1. `build-wasm:release`: `PASS`
2. `atomsTemplate.<W5.4 method set>` references in `webassembly/src`: `0`
3. `LegacyAtomsRuntime().<W5.4 method set>` references outside `structure_lifecycle_service.cpp`: `0`
4. `StructureLifecycleFeature.<W5.4 methods>` call sites: `5`

## Notes

- This stage remains thin-shim: lifecycle/BZ ownership still delegates to legacy runtime through `StructureLifecycleService`.
- W5.5 will continue with UI render-window decomposition.
