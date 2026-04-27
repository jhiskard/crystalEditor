# W5 State Ownership Matrix (Phase 19 Latest)

- Captured at: `2026-04-24 (KST)`
- Scope: `W5.1 + W5.2 + W5.3 + W5.4 completed`

## Ownership Snapshot

| Concern | Current owner | W5 state |
|---|---|---|
| Atom hover/select interaction entrypoints | `structure/application/StructureInteractionService` | `W5.1 DONE` |
| Atom visibility and labels | `structure/application/StructureInteractionService` | `W5.1 DONE` |
| Bond visibility and labels | `structure/application/StructureInteractionService` | `W5.1 DONE` |
| Unit-cell visibility access | `structure/application/StructureInteractionService` | `W5.1 DONE` |
| Structure visibility toggle (mesh tree root icon) | `structure/application/StructureService` | `W5.1 DONE` |
| Measurement mode/picks/drag/overlay | `measurement/application/MeasurementService` | `W5.2 DONE` |
| Measurement tree list/visibility/remove | `measurement/application/MeasurementService` | `W5.2 DONE` |
| Charge density tree visibility/UI grid controls | `density/application/DensityService` | `W5.3 DONE` |
| Charge density/slice viewer window rendering entrypoints | `density/application/DensityService` | `W5.3 DONE` |
| Structure lifecycle register/remove/unassigned | `structure/application/StructureLifecycleService` | `W5.4 DONE` |
| Brillouin-zone window rendering entrypoint | `structure/application/StructureLifecycleService` | `W5.4 DONE` |
| Charge density import runtime bridge | `io/application/import_runtime_port` | `W5.6 TODO` |
| Atom-related UI window decomposition | `shell/presentation/atoms/*` | `W5.5 TODO` |
| XSF/CHGCAR bridge routines | `io/application/import_*` | `W5.6 TODO` |

## Evidence

- New service: `webassembly/src/structure/application/structure_interaction_service.{h,cpp}`
- Expanded service: `webassembly/src/measurement/application/measurement_service.{h,cpp}`
- Expanded service: `webassembly/src/density/application/density_service.{h,cpp}`
- New DTOs: `webassembly/src/density/application/density_service_types.h`
- New service: `webassembly/src/structure/application/structure_lifecycle_service.{h,cpp}`
- Runtime exposure: `WorkbenchRuntime::StructureInteractionFeature()`
- Runtime exposure: `WorkbenchRuntime::MeasurementFeature()`
- Runtime exposure: `WorkbenchRuntime::DensityFeature()`
- Runtime exposure: `WorkbenchRuntime::StructureLifecycleFeature()`
- Caller migration:
  - `viewer_window.cpp`
  - `viewer_interaction_controller.cpp`
  - `model_tree_structure_section.cpp`
  - `model_tree_dialogs.cpp` (structure delete + measurement clear-all)
  - `app.cpp` (density/slice + BZ window render entrypoints)
  - `io/application/import_runtime_port.cpp` (register/remove/unassigned routed to lifecycle feature)
