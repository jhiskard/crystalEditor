# AtomsTemplate Decomposition Progress - W5.9

- Date: `2026-04-27 (KST)`
- Status: `PASS`
- Scope: `W5 symbol zeroization for AtomsTemplate and LegacyAtomsRuntime`

## Implemented

1. Runtime symbol renaming completed across `webassembly/src`:
   - `AtomsTemplate` -> `WorkspaceRuntimeModel`
   - `LegacyAtomsRuntime` -> `WorkspaceRuntimeModelRef`
   - `LegacyAtomsRuntimeConst` -> `WorkspaceRuntimeModelRefConst`
2. All service/presenter/domain call paths were updated to the renamed runtime symbols:
   - structure interaction/lifecycle services
   - measurement service/controller
   - density service/presentation
   - IO import bridge services/ports
   - shell atoms presenter + runtime composition exposure
3. Post-rename build completed to validate compile/link integrity.

## Verification

1. Symbol scan (code scope):
   - `rg -n "AtomsTemplate" webassembly/src` => `0 matches`
   - `rg -n "LegacyAtomsRuntime" webassembly/src` => `0 matches`
2. Build gate:
   - `cmd /c "..\emsdk\emsdk_env.bat && npm.cmd run build-wasm:release"` => `PASS`
3. Automated regression gate:
   - `cmd /c "..\emsdk\emsdk_env.bat && npm.cmd run test:cpp"` => `PASS`
   - `npm.cmd run test:smoke` => `PASS` (`2 passed`)
4. Legacy path gate (carry-over):
   - `Test-Path webassembly/src/workspace/legacy` => `False`
   - `rg "workspace/legacy/" webassembly/src webassembly/cmake/modules` => `0 matches`

## Notes

- W5.9 achieved symbol zeroization in production code scope (`webassembly/src`).
- W5 DoD items for symbol zero (`AtomsTemplate`, `LegacyAtomsRuntime`) are now satisfied at W5 stage.
- Automated regression checks (`test:cpp`, `test:smoke`) passed after symbol rename.
- W6~W10 should focus on singleton elimination, `app.cpp` redecomposition, gate packaging, and closeout verification.
