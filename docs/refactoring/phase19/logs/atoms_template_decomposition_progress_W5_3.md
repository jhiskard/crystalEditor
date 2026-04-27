# AtomsTemplate Decomposition Progress - W5.3

- Date: `2026-04-24 (KST)`
- Status: `PASS`
- Scope: `Charge density tree/UI/viewer migration to DensityService`

## Implemented

1. Expanded `density/application/DensityService` API to cover density UI/viewer entrypoints:
   - grid list/query (`GetSimpleGridEntries`, `GetSliceGridEntries`, `IsSliceVisible`)
   - grid visibility/select controls (`SetAllSimpleGridVisible`, `SetAllSliceGridVisible`, `SetSimpleGridVisible`, `SetSliceGridVisible`, `SelectSimpleGridByName`, `SelectSliceGridByName`)
   - viewer windows (`RenderChargeDensityViewerWindow`, `RenderSliceViewerWindow`)
2. Added density application DTOs for tree rendering:
   - `density/application/density_service_types.h` (`SimpleGridEntry`, `SliceGridEntry`)
3. Migrated charge-density call sites from direct `atomsTemplate` usage to `DensityService` in:
   - `mesh/presentation/model_tree_structure_section.cpp`
   - `app.cpp`

## Verification

1. `build-wasm:release`: `PASS`
2. `atomsTemplate.<W5.3 method set>` references in `webassembly/src`: `0`
3. `LegacyAtomsRuntime().<W5.3 method set>` references outside `density_service.cpp`: `5`
   - residual caller: `io/application/import_runtime_port.cpp` (runtime adapter bridge)

## Notes

- This stage remains thin-shim: density ownership still delegates to legacy runtime through `DensityService`.
- Import runtime adapter cleanup remains for `W5.6`/legacy deletion stages.
