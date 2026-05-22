# Dependency Gate Report — W2

- Date: `2026-04-22 (KST)`
- Status: `PASS`
- Scope: measurement boundary cleanup

## Validation

1. `measurement/application` VTK tokens: `0`
2. Build verification (`build-wasm:release`): `PASS`
3. `viewer_interaction_controller` measurement pick flow uses `PickedAtomInfo`.

## Changed Areas

- `render/application/picked_atom_info.h` (new)
- `workspace/legacy/atoms_template_facade.{h,cpp}`
- `measurement/application/measurement_controller.cpp`
- `render/application/viewer_interaction_controller.cpp`

