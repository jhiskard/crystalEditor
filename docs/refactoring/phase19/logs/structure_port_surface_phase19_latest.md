# Structure Port Surface Inventory (W3)

- Date: `2026-04-22 (KST)`
- Previous adapter: `legacy_structure_service_port.*`

## Surface Methods Migrated

- `GetStructureCount`
- `GetCurrentStructureId` / `SetCurrentStructureId`
- `GetStructures`
- `IsStructureVisible` / `SetStructureVisible`
- `RegisterStructure` / `RemoveStructure` / `RemoveUnassignedData`
- `GetAtomCountForStructure`
- `IsBoundaryAtomsEnabled` / `SetBoundaryAtomsEnabled`

## Result

- `legacy_structure_service_port` references: `0`
- `structure/infrastructure/legacy` directory: removed

