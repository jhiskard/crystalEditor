# W5 State Ownership Matrix (Phase 19 Latest)

- Captured at: `2026-04-27 (KST)`
- Scope: `W5.1 + W5.2 + W5.3 + W5.4 + W5.5 + W5.6 + W5.7 + W5.8 + W5.9 completed`

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
| Atom UI split-window rendering + pending panel request routing | `shell/presentation/atoms/AtomsWindowPresenter` | `W5.5 DONE` |
| XSF parser/apply bridge routines | `io/application/ImportXsfService` | `W5.6 DONE` |
| CHGCAR parser/apply bridge routines | `io/application/ImportChgcarService` | `W5.6 DONE` |
| Import runtime context/lifecycle bridge | `io/application/import_runtime_port` | `W5.6 DONE (surface reduced)` |
| Workspace runtime source path (`legacy` -> `runtime`) | `workspace/runtime/*` | `W5.7 DONE` |
| Workspace CMake path cleanup | `wb_workspace.cmake` | `W5.8 DONE` |
| `AtomsTemplate` symbol removal | `workspace/runtime/WorkspaceRuntimeModel` | `W5.9 DONE` |
| `LegacyAtomsRuntime` call-site removal | `workspace::legacy::WorkspaceRuntimeModelRef` shim | `W5.9 DONE` |

## Evidence

- New service: `webassembly/src/structure/application/structure_interaction_service.{h,cpp}`
- Expanded service: `webassembly/src/measurement/application/measurement_service.{h,cpp}`
- Expanded service: `webassembly/src/density/application/density_service.{h,cpp}`
- New DTOs: `webassembly/src/density/application/density_service_types.h`
- New service: `webassembly/src/structure/application/structure_lifecycle_service.{h,cpp}`
- New presenter: `webassembly/src/shell/presentation/atoms/atoms_window_presenter.{h,cpp}`
- New service: `webassembly/src/io/application/import_xsf_service.{h,cpp}`
- New service: `webassembly/src/io/application/import_chgcar_service.{h,cpp}`
- Runtime path migration:
  - `webassembly/src/workspace/runtime/atoms_template_facade.{h,cpp}`
  - `webassembly/src/workspace/runtime/legacy_atoms_runtime.{h,cpp}`
- Runtime symbol zeroization:
  - `AtomsTemplate` -> `WorkspaceRuntimeModel`
  - `LegacyAtomsRuntime` -> `WorkspaceRuntimeModelRef`
- Runtime exposure: `WorkbenchRuntime::StructureInteractionFeature()`
- Runtime exposure: `WorkbenchRuntime::MeasurementFeature()`
- Runtime exposure: `WorkbenchRuntime::DensityFeature()`
- Runtime exposure: `WorkbenchRuntime::StructureLifecycleFeature()`
- Runtime exposure: `WorkbenchRuntime::AtomsWindowFeature()`
- Caller migration:
  - `viewer_window.cpp`
  - `viewer_interaction_controller.cpp`
  - `model_tree_structure_section.cpp`
  - `model_tree_dialogs.cpp` (structure delete + measurement clear-all)
  - `app.cpp` (density/slice + BZ + atom UI split-window render entrypoints)
  - `io/application/parser_worker_service.cpp` (delegates XSF/CHGCAR parsing to import services)
  - `io/application/import_apply_service.cpp` (delegates XSF/CHGCAR apply bridge to import services)
  - `io/application/import_runtime_port.cpp` (bridge surface reduced to context/lifecycle state)
  - `wb_workspace.cmake` (`workspace/runtime/*` source paths)
  - `webassembly/src/**` (`AtomsTemplate`/`LegacyAtomsRuntime` symbol zeroization)
