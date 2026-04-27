# AtomsTemplate Decomposition Progress - W5.2

- Date: `2026-04-24 (KST)`
- Status: `PASS`
- Scope: `Measurement click/mode/overlay/tree migration to MeasurementService`

## Implemented

1. Expanded `measurement/application/MeasurementService` API to cover measurement interaction entrypoints:
   - mode state (`IsModeActive`, `IsDragSelectionEnabled`)
   - picker flow (`ResolvePickedAtomInfo`, `HandlePickerClick`, `HandleEmptyClick`)
   - drag rectangle (`HandleDragSelectionInScreenRect`)
   - overlay render (`RenderModeOverlay`)
2. Added `MeasurementType` to measurement application DTOs and migrated `MeasurementListItem` typing.
3. Migrated measurement call sites from direct `atomsTemplate` usage to `MeasurementService` in:
   - `render/application/viewer_interaction_controller.cpp`
   - `render/presentation/viewer_window.cpp`
   - `mesh/presentation/model_tree_structure_section.cpp`
   - `mesh/presentation/model_tree_dialogs.cpp`

## Verification

1. `build-wasm:release`: `PASS`
2. `atomsTemplate.<W5.2 method set>` references in `webassembly/src`: `0`
3. `LegacyAtomsRuntime().<W5.2 method set>` references outside `measurement_service.cpp`: `0`

## Notes

- This stage remains thin-shim: measurement ownership still delegates to legacy runtime through `MeasurementService`.
- Density migration remains deferred to `W5.3`.
