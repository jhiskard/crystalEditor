# AtomsTemplate Call Graph (Phase 19 Latest)

- Captured at: `2026-04-24 (KST)`
- Stage: `W5.4 Structure lifecycle/BZ migration`
- Branch: `refactor/phase19-legacy-complete-dismantle`
- Build gate: `PASS` (`build-wasm:release`)

## Runtime Flow

1. Atom/Bond/Cell interaction (W5.1):
   - `viewer_window.cpp` / `viewer_interaction_controller.cpp` / `model_tree_structure_section.cpp`
   - `GetWorkbenchRuntime().StructureInteractionFeature()`
   - `StructureInteractionService`
   - `workspace::legacy::LegacyAtomsRuntime()` (thin-shim target)
2. Measurement click/drag/overlay/list (W5.2):
   - `viewer_interaction_controller.cpp`
   - `viewer_window.cpp`
   - `model_tree_structure_section.cpp`
   - `model_tree_dialogs.cpp`
   - `GetWorkbenchRuntime().MeasurementFeature()`
   - `MeasurementService`
   - `workspace::legacy::LegacyAtomsRuntime()` (thin-shim target)
3. Charge density tree/UI/viewers (W5.3):
   - `model_tree_structure_section.cpp`
   - `app.cpp`
   - `shell/presentation/toolbar/viewer_toolbar_presenter.cpp`
   - `GetWorkbenchRuntime().DensityFeature()`
   - `DensityService`
   - `workspace::legacy::LegacyAtomsRuntime()` (thin-shim target)
4. Structure lifecycle + BZ window entrypoint (W5.4):
   - `app.cpp`
   - `mesh/presentation/model_tree_dialogs.cpp`
   - `io/application/import_runtime_port.cpp`
   - `GetWorkbenchRuntime().StructureLifecycleFeature()`
   - `StructureLifecycleService`
   - `workspace::legacy::LegacyAtomsRuntime()` (thin-shim target)

## Coverage Metrics

- `StructureInteractionService` wrapper methods: `23`
- `structureInteraction.<W5.1 methods>` call sites: `64`
- `MeasurementService` wrapper methods: `18`
- `measurementService.<W5.2 methods>` call sites: `19`
- `DensityService` wrapper methods: `24`
- `densityService.<W5.3 methods>` call sites: `60`
- `StructureLifecycleService` wrapper methods: `7`
- `StructureLifecycleFeature.<W5.4 methods>` call sites: `5`
- `atomsTemplate.<W5.4 method set>` call sites (outside legacy implementation): `0`
- `LegacyAtomsRuntime().<W5.4 method set>` call sites:
  - total: `4`
  - outside `structure_lifecycle_service.cpp`: `0`

## Pending (Next W5 Stages)

- Density import runtime bridge still keeps direct legacy density calls (`W5.6` follow-up).
- UI render-window decomposition remains (`W5.5` target).
