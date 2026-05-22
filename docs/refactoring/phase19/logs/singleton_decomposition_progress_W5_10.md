# W5.10 Singleton Decomposition Progress

- Date: `2026-04-27 (KST)`
- Stage: `W5.10 ::Instance() 호출 전면 제로화`
- Branch: `refactor/phase19-legacy-complete-dismantle`

## Baseline

- Reference snapshot: `docs/refactoring/phase19/logs/singleton_call_sites_phase19_latest.md`
- `::Instance()` callsites in `webassembly/src`: `43`

## Execution

1. Replaced `Instance()` access paths in `webassembly/src` classes with `Shared()` paths.
2. Converted compatibility wrappers from `Class::Instance()` to `Class::Shared()`.
3. Updated `scripts/refactoring/check_phase19_singleton_zero.ps1` from target-3 checks to global `Instance()` symbol zero checks.
4. Fixed pre-existing broken string literals in `workspace/runtime/atoms_template_facade.cpp` that blocked C++ compilation.

## Verification

1. `rg -n "::Instance\\(" webassembly/src` => `0 matches`
2. `rg -n "\\bInstance\\s*\\(" webassembly/src --glob "*.h" --glob "*.hpp" --glob "*.cpp" --glob "*.cc" --glob "*.cxx"` => `0 matches`
3. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase19_singleton_zero.ps1` => `PASS`
4. `cmd /c "..\emsdk\emsdk_env.bat && npm.cmd run build-wasm:release"` => `PASS`

## Regression Status

1. `npm.cmd run test:cpp` => `BLOCKED (sandbox permission)`  
   - error: `PermissionError: ... emsdk_set_env.bat`
2. `npm.cmd run test:smoke` => `BLOCKED (sandbox permission)`  
   - error: `spawn EPERM`

## Notes

- W5.10 code-level objective (`::Instance()`/`Instance()` symbol zero in `webassembly/src`) is achieved.
- Full regression confirmation (`test:cpp`, `test:smoke`) needs an unrestricted environment rerun.
