# AtomsTemplate Decomposition Progress - W5.7

- Date: `2026-04-27 (KST)`
- Status: `PASS`
- Scope: `Physical removal of workspace/legacy directory entries`

## Implemented

1. Moved legacy workspace runtime files out of `workspace/legacy` path:
   - `webassembly/src/workspace/legacy/atoms_template_facade.{h,cpp}`
     -> `webassembly/src/workspace/runtime/atoms_template_facade.{h,cpp}`
   - `webassembly/src/workspace/legacy/legacy_atoms_runtime.{h,cpp}`
     -> `webassembly/src/workspace/runtime/legacy_atoms_runtime.{h,cpp}`
2. Updated include paths across `webassembly/src/**`:
   - `workspace/legacy/*` -> `workspace/runtime/*`
3. Removed now-empty directory:
   - `webassembly/src/workspace/legacy/`

## Verification

1. `Test-Path webassembly/src/workspace/legacy`: `False`
2. `rg "workspace/legacy/" webassembly/src webassembly/cmake/modules`: `0 matches`
3. `build-wasm:release`: `PASS`

## Notes

- This stage fulfills the physical path-removal objective for `workspace/legacy`.
- Runtime symbols/classes are still retained under `workspace/runtime` as compatibility shim and will need additional dismantling to satisfy full W5 DoD symbol-zero targets.
