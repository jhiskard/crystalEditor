# Phase 6 Test Inventory

작성일: `2026-03-31 (KST)`  
기준 계획: `docs/refactoring/phase6/refactoring_phase6_test_regression_system_260331.md`  
기준 브랜치: `refactor/phase6-test-regression-system`

## 1. Snapshot
- C++ 테스트 소스: `4`
- 테스트 fixture: `3`
- Playwright smoke spec: `1`
- `package.json` 테스트 스크립트: `3`
- 테스트용 순수 모듈 분리: `3`
  - `webassembly/src/atoms/domain/cell_transform.cpp`
  - `webassembly/src/atoms/domain/measurement_geometry.cpp`
  - `webassembly/src/io/infrastructure/xsf_parser.cpp`

## 2. Unit Test Inventory
- 실행 명령: `npm run test:cpp`
- 실제 실행 산출물: `webassembly/tests/build-em/wb_native_tests.cjs`
- 실행 방식:
  - 현재 로컬 머신에는 host C++ compiler / Windows RC toolchain이 없어서 `emsdk + emcmake + node` fallback으로 C++ 테스트를 실행한다.
  - 테스트 대상 소스는 브라우저용 production C++ 모듈과 동일하다.
- 등록된 테스트 파일:
  - `webassembly/tests/io/chgcar_parser_test.cpp`
  - `webassembly/tests/io/xsf_parser_test.cpp`
  - `webassembly/tests/atoms/cell_transform_test.cpp`
  - `webassembly/tests/atoms/measurement_geometry_test.cpp`
- 등록된 테스트 케이스 수: `13`
  - CHGCAR parser: `2`
  - XSF parser: `3`
  - Cell transform: `3`
  - Measurement geometry: `5`

## 3. Fixture Inventory
- `webassembly/tests/fixtures/io/sample_chgcar.vasp`
- `webassembly/tests/fixtures/io/sample_structure.xsf`
- `webassembly/tests/fixtures/io/sample_grid.xsf`

## 4. Browser Smoke Inventory
- 실행 명령: `npm run test:smoke`
- 설정 파일: `playwright.config.ts`
- smoke spec:
  - `tests/e2e/workbench-smoke.spec.ts`
- 검증 시나리오:
  - `/workbench` ready signal 대기
  - XSF 구조 import
  - 현재 structure id 확인
  - structure visibility off/on 토글
  - CHGCAR import
  - charge density 로드 상태 확인
  - page error / relevant console error 부재 확인

## 5. Package Entry Points
- `test:cpp`
- `test:smoke`
- `test:phase6`
