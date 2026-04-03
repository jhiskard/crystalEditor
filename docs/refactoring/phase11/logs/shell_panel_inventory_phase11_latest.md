# Phase 11 Shell/Panel Inventory Snapshot (W0)

- Snapshot date: `2026-04-03 (KST)`
- Branch: `refactor/phase11-shell-objectization`
- Scope: W0 baseline for shell/panel objectization

## 1) Singleton Declaration Baseline

| Target header | Marker |
|---|---|
| `webassembly/src/app.h` | `DECLARE_SINGLETON(App)` |
| `webassembly/src/toolbar.h` | `DECLARE_SINGLETON(Toolbar)` |
| `webassembly/src/model_tree.h` | `DECLARE_SINGLETON(ModelTree)` |
| `webassembly/src/mesh_detail.h` | `DECLARE_SINGLETON(MeshDetail)` |
| `webassembly/src/mesh_group_detail.h` | `DECLARE_SINGLETON(MeshGroupDetail)` |
| `webassembly/src/test_window.h` | `DECLARE_SINGLETON(TestWindow)` |

## 2) Singleton Call Baseline (`webassembly/src/**/*.cpp`)

| Metric | Count |
|---|---:|
| `MeshManager::Instance()` | 66 |
| `FileLoader::Instance()` | 3 |
| `App::Instance()` | 15 |

## 3) Action Routing Baseline

1. `App::renderDockSpaceAndMenu()` still directly dispatches menu actions to feature facades (`AtomsTemplate`, `MeasurementService`, `FileLoader`) while mutating local `App` visibility/focus/layout state.
2. `Toolbar` still directly dispatches viewer/mesh actions (`MeshManager::Instance()`, `render::application::GetRenderGateway()`, `AtomsTemplate::Instance()`).
3. `shell` currently contains only `runtime` (`webassembly/src/shell/runtime/*`) and has no dedicated shell state store/controller layer yet.

## 4) W1/W2 Baseline Gate Targets

- Introduce `shell/domain/shell_state_store.*` to centralize shell window/layout/focus state ownership.
- Introduce `shell/application/workbench_controller.*` and migrate menu/toolbar dispatch entrypoints to controller commands.
- Keep Phase 9 guardrails intact:
  - no new direct `VtkViewer::Instance()` usage in non-render modules
  - no `FontRegistry()` warm-up in `PrimeLegacySingletons()`
  - no regression in XSF first-grid bootstrap behavior

## 5) Execution Update (W1~W2)

1. W1 completed:
   - Added shell state store:
     - `webassembly/src/shell/domain/shell_state_store.h`
     - `webassembly/src/shell/domain/shell_state_store.cpp`
   - Added shell state query/command services:
     - `webassembly/src/shell/application/shell_state_query_service.h`
     - `webassembly/src/shell/application/shell_state_query_service.cpp`
     - `webassembly/src/shell/application/shell_state_command_service.h`
     - `webassembly/src/shell/application/shell_state_command_service.cpp`
   - Added runtime accessors for shell state services in `workbench_runtime`.

2. W2 completed:
   - Added shell controller:
     - `webassembly/src/shell/application/workbench_controller.h`
     - `webassembly/src/shell/application/workbench_controller.cpp`
   - Routed `App` menu command actions through `WorkbenchController`.
   - Routed `Toolbar` action handlers (boundary toggle / mesh mode / projection / reset / axis align / step input) through `WorkbenchController`.
   - Added temporary App<->ShellStateStore synchronization bridge for visibility/focus/layout state migration.

3. Build/Test verification:
   - `npm run build-wasm:release`: `PASS`
   - `npm run test:cpp`: `PASS` (`1/1`)

## 6) Execution Update (W3~W5)

1. W3 completed (panel singleton replacement stage 1):
   - Removed `DECLARE_SINGLETON` from:
     - `webassembly/src/model_tree.h`
     - `webassembly/src/mesh_group_detail.h`
     - `webassembly/src/test_window.h`
   - Converted panel mutable static state to instance state:
     - `ModelTree` selection/delete/confirm-popup state
     - `MeshGroupDetail` selection/delete/change-flag state
   - Added compatibility `Instance()` runtime shim wrappers in panel classes (`@note` Phase 12 removal target).

2. W4 completed (panel singleton replacement stage 2 + app shrink):
   - Removed `DECLARE_SINGLETON` from `webassembly/src/mesh_detail.h`.
   - Added compatibility `MeshDetail::Instance()` runtime shim.
   - Switched `WorkbenchRuntime` panel accessors to runtime-owned static panel objects (no direct panel `::Instance()` in runtime accessors).
   - Reduced direct `MeshManager::Instance()` usage in `mesh_detail.cpp` to central helper path.
   - Further reduced `app.cpp` feature direct-control path by routing settings toggles (`Node Tooltip` / `Viewer FPS Overlay`) through `WorkbenchController`.

3. W5 completed (static gate script):
   - Added `scripts/refactoring/check_phase11_shell_panel_objectization.ps1`.
   - Generated gate log: `docs/refactoring/phase11/logs/check_phase11_shell_panel_objectization_latest.txt` (`PASS`).

4. Verification snapshot (W3~W5):
   - `DECLARE_SINGLETON` count in `App/Toolbar/ModelTree/MeshDetail/MeshGroupDetail/TestWindow`: `2` (`App`, `Toolbar` only)
   - `MeshManager::Instance()` count in `mesh_detail.cpp`: `1`
   - `npm run build-wasm:release`: `PASS`
   - `npm run test:cpp`: `PASS` (`1/1`)
