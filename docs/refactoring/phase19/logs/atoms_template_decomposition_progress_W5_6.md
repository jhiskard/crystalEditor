# AtomsTemplate Decomposition Progress - W5.6

- Date: `2026-04-27 (KST)`
- Status: `PASS`
- Scope: `XSF/CHGCAR bridge migration to io/application/import_*_service`

## Implemented

1. Added XSF bridge service:
   - `webassembly/src/io/application/import_xsf_service.{h,cpp}`
   - responsibilities:
     - DATAGRID detection (`ContainsDatagrid3d`)
     - XSF parse (`ParseXsfFile`, `ParseXsfGridFile`)
     - XSF parsed-data apply bridge (`LoadXsfParsedData`)
     - density grid payload transfer bridge
2. Added CHGCAR bridge service:
   - `webassembly/src/io/application/import_chgcar_service.{h,cpp}`
   - responsibilities:
     - CHGCAR parse (`ParseChgcarFile`)
     - CHGCAR parsed-data apply bridge (`LoadChgcarParsedData`)
3. Migrated parser path from runtime-port legacy parent bridge to service-based parsing:
   - `ParserWorkerService` now uses `ImportXsfService` / `ImportChgcarService`
4. Migrated apply path from runtime-port parsed-data bridge to service-based apply:
   - `ImportApplyService` now delegates XSF/CHGCAR apply bridge calls to
     `ImportXsfService` / `ImportChgcarService`
5. Reduced `ImportRuntimePort` surface:
   - removed parser/apply bridge methods:
     - `LoadXsfParsedData`
     - `LoadChgcarParsedData`
     - `SetChargeDensityGridDataEntries`
     - `SetAllChargeDensityAdvancedGridVisible`
     - `LegacyParent`
6. Updated IO module source list:
   - `webassembly/cmake/modules/wb_io.cmake`
   - added `import_xsf_service.cpp`, `import_chgcar_service.cpp`

## Verification

1. `build-wasm:release`: `PASS`
2. `ImportRuntimePort` removed bridge methods references: `0`
   - `LegacyParent`, `LoadXsfParsedData`, `LoadChgcarParsedData`,
     `SetChargeDensityGridDataEntries`, `SetAllChargeDensityAdvancedGridVisible`
3. Direct parser bridge construction in parser/apply path:
   - `FileIOManager loader(...)` in `parser_worker_service.cpp` / `import_apply_service.cpp`: `0`
   - direct `ChgcarParser::parse(...)` in `parser_worker_service.cpp` / `import_apply_service.cpp`: `0`
4. Service delegation call sites:
   - `parser_worker_service.cpp` -> `ImportXsfService` parse calls: `5`
   - `parser_worker_service.cpp` -> `ImportChgcarService::ParseChgcarFile`: `1`
   - `import_apply_service.cpp` -> `importXsfService()` calls: `5`
   - `import_apply_service.cpp` -> `importChgcarService()` calls: `1`

## Notes

- W5.6 is still thin-shim: `ImportXsfService` / `ImportChgcarService` internally delegate final parsed-data application to legacy runtime.
- This step completes planned W5.1~W5.6 sequential migration. Next is W5.7 physical deletion of `workspace/legacy` files.
