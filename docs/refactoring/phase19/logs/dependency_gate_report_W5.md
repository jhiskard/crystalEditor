# Dependency Gate Report - W5

- Date: `2026-04-27 (KST)`
- Stage: `W5.1 ~ W5.10`
- Branch: `refactor/phase19-legacy-complete-dismantle`

## Gate Results

1. `build-wasm:release`: `PASS`
2. `workspace/legacy directory exists`: `PASS`  
   - final check: `Test-Path webassembly/src/workspace/legacy` => `False`
3. `workspace/legacy path refs in src/cmake`: `PASS`  
   - final check: `rg "workspace/legacy/" webassembly/src webassembly/cmake/modules` => `0 matches`
4. `AtomsTemplate symbol references in code`: `PASS`  
   - final check: `rg -n "AtomsTemplate" webassembly/src` => `0 matches`
5. `LegacyAtomsRuntime symbol references in code`: `PASS`  
   - final check: `rg -n "LegacyAtomsRuntime" webassembly/src` => `0 matches`
6. `W5.9 rename/compatibility compile gate`: `PASS`  
   - final check: `cmd /c "..\emsdk\emsdk_env.bat && npm.cmd run build-wasm:release"` => `PASS`
7. `W5.10 ::Instance() callsites in src`: `PASS`  
   - final check: `rg -n "::Instance\(" webassembly/src` => `0 matches`
8. `W5.10 Instance() symbols in src`: `PASS`  
   - final check: `rg -n "\bInstance\s*\(" webassembly/src --glob "*.h" --glob "*.hpp" --glob "*.cpp" --glob "*.cc" --glob "*.cxx"` => `0 matches`
9. `W5.10 singleton gate`: `PASS`  
   - final check: `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase19_singleton_zero.ps1` => `PASS`
10. `C++ unit regression`: `BLOCKED (sandbox permission)`  
   - final check: `npm.cmd run test:cpp` => `PermissionError: ... emsdk_set_env.bat`
11. `e2e smoke regression`: `BLOCKED (sandbox permission)`  
   - final check: `npm.cmd run test:smoke` => `spawn EPERM`

## Comment Contract Checks

1. 신규/변경 public API 주석 존재 여부: `PARTIAL PASS`
   - W5.5/W5.6 신규 서비스 헤더(`atoms_window_presenter`, `import_xsf_service`, `import_chgcar_service`)에 API 주석 유지
2. `Temporary compatibility path` 제거 목표/추적 ID 존재 여부: `PARTIAL PASS`
   - W5.9에서 `workspace::legacy::WorkspaceRuntimeModelRef()`로 단일 shim 명칭 정리
3. 만료 임시 주석 0건 여부: `NOT VERIFIED (W10 final scope)`

## Summary

- W5.1~W5.10 실행 경로가 순차적으로 반영되었고, W5.10의 `::Instance()` 전면 제로화가 `webassembly/src` 기준으로 달성되었다.
- W5 DoD의 핵심 목표인 `AtomsTemplate`/`LegacyAtomsRuntime` 심볼 0 상태는 유지된다.
- 빌드와 singleton 게이트는 PASS이며, 자동 회귀(`test:cpp`, `test:smoke`)는 샌드박스 권한 제약으로 차단되어 재실행이 필요하다.
