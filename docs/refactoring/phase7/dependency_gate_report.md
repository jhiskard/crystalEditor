# Phase 7 Runtime Composition 게이트 리포트

작성일: `2026-04-01 (KST)`  
기준 계획: `docs/refactoring/phase7/refactoring_phase7_composition_root_singleton_quarantine_260331.md`  
작업 브랜치: `refactor/phase7-composition-root`

근거 로그:
- 정적 게이트: `docs/refactoring/phase7/logs/check_phase7_runtime_composition_latest.txt`
- WASM 빌드: `docs/refactoring/phase7/logs/build_phase7_latest.txt`
- C++ 테스트: `docs/refactoring/phase7/logs/unit_test_phase7_latest.txt`
- 브라우저 스모크: `docs/refactoring/phase7/logs/smoke_phase7_latest.txt`
- 기준선/인벤토리: `docs/refactoring/phase7/logs/runtime_inventory_phase7_latest.md`

## 1. 필수 정적 게이트 결과
| 항목 | 현재값 | 목표값 | 결과 |
|---|---:|---:|---|
| `P7.runtime_header_exists` | 1 | 1 | PASS |
| `P7.runtime_source_exists` | 1 | 1 | PASS |
| `P7.active_binding_exists` | 1 | 1 | PASS |
| `P7.main_instance_calls` | 0 | 0 | PASS |
| `P7.active_binding_instance_calls` | 0 | 0 | PASS |
| `P7.legacy_binding_instance_calls` | 0 | 0 | PASS |
| `P7.app_instance_calls_total` | 0 | <=2 | PASS |
| `P7.app_feature_singleton_lookups` | 0 | 0 | PASS |
| `P7.singleton_declaration_budget` | 11 | <=11 | PASS |
| `P7.runtime_header_doxygen` | 1 | 1 | PASS |
| `P7.runtime_accessor_present` | 1 | 1 | PASS |
| `P7.binding_doxygen` | 1 | 1 | PASS |
| `P7.binding_runtime_facade_usage` | 1 | 1 | PASS |
| `P7.runtime_prime_excludes_font_registry` | 0 | 0 | PASS |
| `P7.main_font_registry_after_imgui_context` | 1 | 1 | PASS |
| `P7.cmake_new_binding_entrypoint` | 1 | 1 | PASS |
| `P7.cmake_legacy_binding_removed` | 1 | 1 | PASS |

## 2. 실행 게이트 결과
- `npm run build-wasm:release`
  - 결과: `PASS`
  - 근거: `build_phase7_latest.txt` (`VTK-Workbench.js/.wasm/.data` install 경로 갱신 확인)
- `npm run test:cpp`
  - 결과: `PASS`
  - 근거: `unit_test_phase7_latest.txt` (`1/1` test passed)
- `npm run test:smoke`
  - 결과: `PASS`
  - 근거: `smoke_phase7_latest.txt` (`1 passed`)

## 3. W6 추가 수정 반영
- 폰트/아이콘 회귀 복구
  - 원인: `PrimeLegacySingletons()`에서 ImGui context 이전에 `FontRegistry()`가 초기화되어 폰트 atlas/아이콘 merge 순서가 깨짐.
  - 조치:
    - `webassembly/src/shell/runtime/workbench_runtime.cpp`에서 `PrimeLegacySingletons()`의 `FontRegistry()` 제거
    - `webassembly/src/shell/runtime/workbench_runtime.h`에 ImGui 의존 서비스 초기화 순서 제약 주석 명시
    - `scripts/refactoring/check_phase7_runtime_composition.ps1`에 순서 가드(`runtime_prime_excludes_font_registry`, `main_font_registry_after_imgui_context`) 추가

- VASP -> XSF(Grid) 순서 회귀 수정
  - 증상: VASP 파일을 먼저 읽은 뒤 XSF(Grid)를 읽으면 `Model Tree > Crystal Structure`의 Isosurface가 누락되는 케이스 존재.
  - 원인: `ChargeDensityUI::setGridDataEntries()`가 기존 density 로드 상태에서 첫 grid bootstrap을 건너뛰어 구조 컨텍스트 전환이 누락될 수 있었음.
  - 조치:
    - `webassembly/src/atoms/ui/charge_density_ui.cpp`에서 grid entry 설정 후 `loadFromGridEntry(0)`를 항상 수행하도록 변경

## 4. 잔여 리스크
| ID | 리스크 | 수준 | 대응 |
|---|---|---|---|
| R1 | `AtomsTemplate`/`VtkViewer` 중심의 상위 구조 결합은 여전히 잔존 | Medium | Phase 8~9에서 도메인/렌더 경계 분리를 우선 수행 |
| R2 | 로컬 sandbox 환경에서 `emsdk`/Playwright 실행 시 권한 제약(`Permission denied`, `spawn EPERM`)이 발생 가능 | Low | 게이트 실행 시 권한 상승 실행 경로를 표준화 |
| R3 | C++ 테스트는 `emsdk` 기반 실행 경로에 의존 | Low | 후속 단계에서 host-native CI 경로 병행 검증 |

## 5. 요약
- Phase 7 정적 게이트와 동적 게이트(빌드/C++ 테스트/스모크)는 모두 `PASS`다.
- W5 이후 추가된 폰트 초기화 순서 복구와 VASP/XSF 회귀 수정이 문서/게이트에 반영되었다.
- Phase 8 착수에 필요한 종료 문서 패키지가 준비되었다.
