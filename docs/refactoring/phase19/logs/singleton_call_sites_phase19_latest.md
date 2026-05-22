# Phase19 W6 Singleton Call Sites (Latest)

- Captured at: `2026-04-27 (KST)`
- Stage: `W6 runtime singleton elimination`
- Branch: `refactor/phase19-legacy-complete-dismantle`

## Baseline (Input from W5.10)

- `DECLARE_SINGLETON` usage (definition included): `1` (`macro/singleton_macro.h` definition only)
- `DECLARE_SINGLETON` production class usage (excluding macro definition): `0`
- `::Instance()` callsites in `webassembly/src`: `0`
- `Instance()` symbol matches in `webassembly/src`: `0`
- `::Shared()` callsites/definitions in `webassembly/src`: `43`
- Target runtime class static singleton entrypoints:
  - `VtkViewer::{Instance|Shared}()`: `0`
  - `MeshManager::{Instance|Shared}()`: `0`
  - `FontManager::{Instance|Shared}()`: `0`

## Changes Applied

1. Revalidated W6 ownership intent after W5.10 global zeroization:
   - Runtime-owned composition path is preserved for `VtkViewer`, `MeshManager`, and `FontManager`.
   - Direct static singleton entrypoints for the three runtime targets remain absent.
2. Revalidated singleton-zero gate behavior:
   - `scripts/refactoring/check_phase19_singleton_zero.ps1` enforces zero `DECLARE_SINGLETON` usage in production classes and zero `Instance()` symbols in production source.
3. Refreshed W6 evidence logs with latest metrics and command results.

## Post-State (After W6 Execution Refresh)

- `DECLARE_SINGLETON` usage (definition included): `1`
- `DECLARE_SINGLETON` production class usage (excluding macro definition): `0`
- `::Instance()` callsites in `webassembly/src`: `0`
- `Instance()` symbol matches in `webassembly/src`: `0`
- `::Shared()` callsites/definitions in `webassembly/src`: `43`
- Target runtime class static singleton entrypoints:
  - `VtkViewer::{Instance|Shared}()`: `0`
  - `MeshManager::{Instance|Shared}()`: `0`
  - `FontManager::{Instance|Shared}()`: `0`

## Shared() Inventory Snapshot

- `MeshDetail::Shared`: `10`
- `WorkbenchRuntime::Shared`: `3`
- `ModelTree::Shared`: `3`
- `ShellStateQueryService::Shared`: `2`
- `atoms::domain::StructureStateStore::Shared`: `2`
- `StructureRepository::Shared`: `2`
- `ShellStateStore::Shared`: `2`
- `WorkbenchController::Shared`: `2`
- `ShellStateCommandService::Shared`: `2`
- `MeshQueryService::Shared`: `2`
- `MeshRepository::Shared`: `2`
- `WorkspaceStore::Shared`: `2`
- `WorkspaceQueryService::Shared`: `2`
- `MeshCommandService::Shared`: `2`
- `WorkspaceCommandService::Shared`: `2`
- `TestWindow::Shared`: `1`
- `MeshGroupDetail::Shared`: `1`
- `StructureStateStore::Shared`: `1`

## Verification Commands

1. `npm run check:phase19:singleton-zero` => `PASS`
2. `cmd /c "..\\emsdk\\emsdk_env.bat && npm.cmd run build-wasm:release"` => `PASS`
3. `npm run test:cpp` => `PASS`
4. `npm run test:smoke` => `PASS` (`2 passed`)
