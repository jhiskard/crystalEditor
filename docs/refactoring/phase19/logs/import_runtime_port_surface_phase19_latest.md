# Import Runtime Port Surface Inventory (W4)

- Date: `2026-04-22 (KST)`
- Previous adapter location: `io/infrastructure/legacy/legacy_import_runtime_port.*`

## Surface

- scene presence queries (`HasStructures`, `HasUnitCell`, `HasChargeDensity`)
- structure/density context getters/setters
- structure registry/cleanup operations
- parsed data apply operations (`LoadXsfParsedData`, `LoadChgcarParsedData`)
- density grid payload transfer
- legacy parent access for parser bridge

## W4 Result

- removed legacy adapter files in `io/infrastructure/legacy`
- moved default runtime adapter implementation to `io/application/import_runtime_port.cpp`

