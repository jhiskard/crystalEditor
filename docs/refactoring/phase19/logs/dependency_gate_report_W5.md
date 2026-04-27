# Dependency Gate Report - W5

- Date: `2026-04-27 (KST)`
- Stage: `W5.1 ~ W5.9`
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
7. `C++ unit regression`: `PASS`  
   - final check: `cmd /c "..\emsdk\emsdk_env.bat && npm.cmd run test:cpp"` => `PASS`
8. `e2e smoke regression`: `PASS`  
   - final check: `npm.cmd run test:smoke` => `PASS` (`2 passed`)

## Comment Contract Checks

1. 신규/변경 public API 주석 존재 여부: `PARTIAL PASS`
   - W5.5/W5.6 신규 서비스 헤더(`atoms_window_presenter`, `import_xsf_service`, `import_chgcar_service`)에 API 주석 유지
2. `Temporary compatibility path` 제거 목표/추적 ID 존재 여부: `PARTIAL PASS`
   - W5.9에서 `workspace::legacy::WorkspaceRuntimeModelRef()`로 단일 shim 명칭 정리
3. 만료 임시 주석 0건 여부: `NOT VERIFIED (W10 final scope)`

## Summary

- W5.1~W5.9 실행 경로가 순차적으로 반영되었고, 빌드 게이트를 통과했다.
- W5 DoD의 핵심 목표인 `AtomsTemplate`/`LegacyAtomsRuntime` 심볼 0이 `webassembly/src` 기준으로 달성되었다.
- 자동 회귀(단위/e2e)는 PASS이며, 수동 점검 항목은 W10 종료 점검에서 최종 봉인한다.
