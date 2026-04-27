# AtomsTemplate Decomposition Progress - W5.1

- Date: `2026-04-24 (KST)`
- Status: `PASS`
- Scope: `Atom/Bond/Cell interaction migration to StructureInteractionService`

## Implemented

1. Added `StructureInteractionService` thin-shim facade in `structure/application`.
2. Wired service into runtime composition via `WorkbenchRuntime::StructureInteractionFeature()`.
3. Registered new source in `wb_structure.cmake`.
4. Migrated interaction call sites from direct `atomsTemplate` usage to `structureInteraction` in:
   - `render/presentation/viewer_window.cpp`
   - `render/application/viewer_interaction_controller.cpp`
   - `mesh/presentation/model_tree_structure_section.cpp`
5. Replaced structure root visibility toggle path in `model_tree_mesh_section.cpp` to use `StructureService`.

## Verification

1. `build-wasm:release`: `PASS`
2. `atomsTemplate.<W5.1 method set>` references in `webassembly/src`: `0`
3. `LegacyAtomsRuntime().<W5.1 method set>` references outside `structure_interaction_service.cpp`: `0`

## Notes

- Measurement and density interactions intentionally remain on legacy facade for upcoming `W5.2` and `W5.3`.
