# Dependency Gate Report — W4

- Date: `2026-04-22 (KST)`
- Status: `PASS`
- Scope: IO/render legacy removal

## Validation

1. `io/infrastructure/legacy` directory removed.
2. `render/application/legacy_viewer_facade.*` removed.
3. `legacy_import_runtime_port` refs: `0`
4. `GetLegacyViewerFacade` refs: `0`
5. `build-wasm:release`: `PASS`

## CMake Updates

- `wb_io.cmake`: removed legacy import adapter source, added `import_runtime_port.cpp`
- `wb_render.cmake`: removed `legacy_viewer_facade.cpp`

