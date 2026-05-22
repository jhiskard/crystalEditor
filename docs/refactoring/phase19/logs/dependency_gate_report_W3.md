# Dependency Gate Report — W3

- Date: `2026-04-22 (KST)`
- Status: `PASS`
- Scope: feature service port internalization

## Validation

1. `legacy_structure_service_port` refs: `0`
2. `legacy_measurement_service_port` refs: `0`
3. `legacy_density_service_port` refs: `0`
4. `build-wasm:release`: `PASS`

## CMake Updates

- `wb_structure.cmake`: removed structure legacy adapter source
- `wb_measurement.cmake`: removed measurement legacy adapter source
- `wb_density.cmake`: removed density legacy adapter source

