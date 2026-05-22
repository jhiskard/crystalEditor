# AtomsTemplate Call Graph (Phase 19 Latest)

- Captured at: `2026-04-27 (KST)`
- Stage: `W5.9 symbol zeroization complete`
- Branch: `refactor/phase19-legacy-complete-dismantle`
- Build gate: `PASS` (`build-wasm:release`)

## Runtime Flow

1. Atom/Bond/Cell interaction (W5.1):
   - `viewer_window.cpp` / `viewer_interaction_controller.cpp` / `model_tree_structure_section.cpp`
   - `GetWorkbenchRuntime().StructureInteractionFeature()`
   - `StructureInteractionService`
   - `workspace::legacy::WorkspaceRuntimeModelRef()` (thin-shim target)
2. Measurement click/drag/overlay/list (W5.2):
   - `viewer_interaction_controller.cpp`
   - `viewer_window.cpp`
   - `model_tree_structure_section.cpp`
   - `model_tree_dialogs.cpp`
   - `GetWorkbenchRuntime().MeasurementFeature()`
   - `MeasurementService`
   - `workspace::legacy::WorkspaceRuntimeModelRef()` (thin-shim target)
3. Charge density tree/UI/viewers (W5.3):
   - `model_tree_structure_section.cpp`
   - `app.cpp`
   - `shell/presentation/toolbar/viewer_toolbar_presenter.cpp`
   - `GetWorkbenchRuntime().DensityFeature()`
   - `DensityService`
   - `workspace::legacy::WorkspaceRuntimeModelRef()` (thin-shim target)
4. Structure lifecycle + BZ window entrypoint (W5.4):
   - `app.cpp`
   - `mesh/presentation/model_tree_dialogs.cpp`
   - `io/application/import_runtime_port.cpp`
   - `GetWorkbenchRuntime().StructureLifecycleFeature()`
   - `StructureLifecycleService`
   - `workspace::legacy::WorkspaceRuntimeModelRef()` (thin-shim target)
5. Atom UI split-window/panel request entrypoints (W5.5):
   - `app.cpp`
   - `GetWorkbenchRuntime().AtomsWindowFeature()`
   - `shell::presentation::atoms::AtomsWindowPresenter`
   - `workspace::legacy::WorkspaceRuntimeModelRef()` (thin-shim target)
6. XSF/CHGCAR import bridge entrypoints (W5.6):
   - `io/application/parser_worker_service.cpp`
   - `io/application/import_apply_service.cpp`
   - `ImportXsfService` / `ImportChgcarService`
   - `workspace::legacy::WorkspaceRuntimeModelRef()` (thin-shim target)
7. Workspace runtime path migration (W5.7/W5.8):
   - source path: `workspace/legacy/*` -> `workspace/runtime/*`
   - CMake wiring: `wb_workspace.cmake` path update
   - compatibility shim retained under renamed runtime symbols
8. Runtime symbol zeroization (W5.9):
   - class symbol: `AtomsTemplate` -> `WorkspaceRuntimeModel`
   - accessor symbol: `LegacyAtomsRuntime()` -> `WorkspaceRuntimeModelRef()`
   - const accessor symbol: `LegacyAtomsRuntimeConst()` -> `WorkspaceRuntimeModelRefConst()`

## Coverage Metrics

- `StructureInteractionService` wrapper methods: `23`
- `structureInteraction.<W5.1 methods>` call sites: `64`
- `MeasurementService` wrapper methods: `18`
- `measurementService.<W5.2 methods>` call sites: `19`
- `DensityService` wrapper methods: `24`
- `densityService.<W5.3 methods>` call sites: `60`
- `StructureLifecycleService` wrapper methods: `7`
- `StructureLifecycleFeature.<W5.4 methods>` call sites: `5`
- `AtomsWindowPresenter` wrapper methods: `9`
- `atomsWindowPresenter.<W5.5 methods>` call sites in `app.cpp`: `9`
- `atomsTemplate.<W5.5 method set>` call sites outside `atoms_window_presenter.cpp`: `0`
- `WorkspaceRuntimeModelRef().<W5.5 method set>` call sites outside `atoms_window_presenter.cpp`: `0`
- `ImportXsfService` bridge methods: `6`
- `ImportChgcarService` bridge methods: `2`
- `parser_worker_service.cpp` -> `ImportXsfService` parse calls: `5`
- `parser_worker_service.cpp` -> `ImportChgcarService::ParseChgcarFile` calls: `1`
- `import_apply_service.cpp` -> `importXsfService()` calls: `5`
- `import_apply_service.cpp` -> `importChgcarService()` calls: `1`
- `ImportRuntimePort` legacy parser/apply bridge methods:
  - before W5.6: `5`
  - after W5.6: `0`
- `workspace/legacy` path references in `webassembly/src` + `webassembly/cmake/modules`:
  - before W5.7/W5.8: `>0`
  - after W5.7/W5.8: `0`
- `webassembly/src/workspace/legacy` directory:
  - before W5.7: `present`
  - after W5.7: `removed`
- `AtomsTemplate` symbol references in `webassembly/src`:
  - before W5.9: `>0`
  - after W5.9: `0`
- `LegacyAtomsRuntime` symbol references in `webassembly/src`:
  - before W5.9: `>0`
  - after W5.9: `0`

## Pending (Next WBS)

- W5 scope pending: `None`
- Follow-up consolidation: W6~W10 gates (singleton/closeout/meta-gate sequence)
