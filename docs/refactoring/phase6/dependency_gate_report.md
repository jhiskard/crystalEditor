# Phase 6 테스트/회귀 게이트 리포트

작성일: `2026-03-31 (KST)`  
기준 계획: `docs/refactoring/phase6/refactoring_phase6_test_regression_system_260331.md`  
작업 브랜치: `refactor/phase6-test-regression-system`

근거 로그:
- 정적 게이트: `docs/refactoring/phase6/logs/check_phase6_test_regression_latest.txt`
- C++ 테스트: `docs/refactoring/phase6/logs/unit_test_phase6_latest.txt`
- 브라우저 스모크: `docs/refactoring/phase6/logs/smoke_phase6_latest.txt`
- 테스트 인벤토리: `docs/refactoring/phase6/logs/test_inventory_phase6_latest.md`

## 1. 필수 게이트 결과
| 항목 | 현재값 | 목표값 | 결과 |
|---|---:|---:|---|
| `P6.tests_cmake_exists` | 1 | 1 | PASS |
| `P6.unit_test_files_present` | 4 | 4 | PASS |
| `P6.fixture_files_present` | 3 | 3 | PASS |
| `P6.package_test_scripts_present` | 3 | 3 | PASS |
| `P6.playwright_config_exists` | 1 | 1 | PASS |
| `P6.smoke_spec_exists` | 1 | 1 | PASS |
| `P6.page_ready_signal` | 1 | 1 | PASS |
| `P6.page_test_api` | 1 | 1 | PASS |
| `P6.wasm_state_bindings` | 1 | 1 | PASS |
| `P6.pure_test_modules_present` | 3 | 3 | PASS |
| `P6.test_runner_emsdk_fallback` | 1 | 1 | PASS |

## 2. 실행 게이트 결과
- `npm run test:cpp`
  - 결과: `PASS`
  - 근거: `wb_native_tests.cjs` 1개 CTest 타깃 PASS
- `npm run test:smoke`
  - 결과: `PASS`
  - 근거: Playwright `1 passed`
- `npm run test:phase6`
  - 결과: `PASS`
  - 근거: C++ 테스트 + smoke 일괄 실행 성공

## 3. 구현 요약
- 순수 계산/파서 로직을 브라우저/VTK 경계에서 분리해 테스트 가능한 C++ 모듈로 고정했다.
- `webassembly/tests` 기반 C++ 테스트 러너와 fixture를 추가했다.
- `/workbench` 페이지에 ready signal과 runtime state seam을 노출해 Playwright smoke를 자동화했다.
- wasm binding에 structure/visibility/charge-density 상태 조회 함수를 추가해 브라우저 검증 경로를 만들었다.

## 4. 환경 메모
- 현재 로컬 머신에는 host-native Windows C++ 빌더가 없어 `test:cpp`는 `emsdk + node` fallback으로 실행한다.
- 이 fallback은 테스트 대상 C++ 소스를 바꾸지 않고 실행기만 대체한다.

## 5. 잔여 리스크
| ID | 리스크 | 수준 | 대응 |
|---|---|---|---|
| R1 | C++ 테스트 실행기가 host-native가 아니라 `emsdk/node` fallback임 | Medium | CI 또는 toolchain 정비 시 host-native 경로를 추가로 병행 검증 |
| R2 | 브라우저 smoke는 favicon 등 정적 자산 404 노이즈를 판정에서 제외함 | Low | 실제 runtime exception / console runtime error는 그대로 실패 처리 |

## 6. 요약
- Phase 6 필수 정적 게이트는 모두 `PASS`다.
- C++ 단위 테스트와 브라우저 smoke가 모두 `PASS`다.
- Phase 7 착수 판정에 필요한 테스트/회귀 문서 패키지가 준비되었다.
