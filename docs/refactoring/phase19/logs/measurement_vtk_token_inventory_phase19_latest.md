# Measurement VTK Token Inventory (W2)

- Date: `2026-04-22 (KST)`
- Scope: `webassembly/src/measurement/application`

## Result

| Pattern | Count |
|---|---:|
| `vtkActor` | 0 |
| `vtkVolume` | 0 |
| `vtkSmartPointer` | 0 |
| `vtk*` tokens (broad) | 0 |

## W2 Refactor Notes

- Added `render/application/picked_atom_info.h`.
- Changed `AtomsTemplate::HandleMeasurementClickByPicker(...)` to consume `PickedAtomInfo`.
- Added `AtomsTemplate::ResolvePickedAtomInfo(...)` for render-side translation.
- Updated `viewer_interaction_controller.cpp` call sites.

