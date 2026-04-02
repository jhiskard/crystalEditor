# Phase 8 AtomsTemplate Dismantle 게이트 리포트

작성일: `2026-04-01 (KST)`  
기준 계획: `docs/refactoring/phase8/refactoring_phase8_atoms_template_dismantle_260401.md`  
상위 계획: `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md` (Phase 8 절)  
작업 브랜치: `refactor/phase8-atoms-template-dismantle`

근거 로그:
- 정적 게이트: `docs/refactoring/phase8/logs/check_phase8_atoms_template_dismantle_latest.txt`
- WASM 빌드: `docs/refactoring/phase8/logs/build_phase8_latest.txt`
- C++ 테스트: `docs/refactoring/phase8/logs/unit_test_phase8_latest.txt`
- 브라우저 스모크: `docs/refactoring/phase8/logs/smoke_phase8_latest.txt`
- 기준선/인벤토리: `docs/refactoring/phase8/logs/atoms_template_inventory_phase8_latest.md`

## 1. 필수 정적 게이트 결과
| 항목 | 현재값 | 목표값 | 결과 |
|---|---:|---:|---|
| `P8.structure_dir_exists` | 1 | 1 | PASS |
| `P8.measurement_dir_exists` | 1 | 1 | PASS |
| `P8.density_dir_exists` | 1 | 1 | PASS |
| `P8.structure_repository_header_exists` | 1 | 1 | PASS |
| `P8.structure_repository_source_exists` | 1 | 1 | PASS |
| `P8.structure_service_header_exists` | 1 | 1 | PASS |
| `P8.structure_service_source_exists` | 1 | 1 | PASS |
| `P8.measurement_service_header_exists` | 1 | 1 | PASS |
| `P8.measurement_service_source_exists` | 1 | 1 | PASS |
| `P8.density_service_header_exists` | 1 | 1 | PASS |
| `P8.density_service_source_exists` | 1 | 1 | PASS |
| `P8.atoms_template_friend_class_zero` | 0 | 0 | PASS |
| `P8.atoms_template_instance_total_budget` | 72 | <=80 | PASS |
| `P8.atoms_template_vtk_viewer_instance_budget` | 46 | <=50 | PASS |
| `P8.runtime_header_module_api` | 1 | 1 | PASS |
| `P8.runtime_structure_feature_usage` | 1 | 1 | PASS |
| `P8.runtime_density_feature_usage` | 1 | 1 | PASS |
| `P8.app_measurement_feature_usage` | 1 | 1 | PASS |
| `P8.model_tree_module_service_usage` | 1 | 1 | PASS |
| `P8.runtime_prime_excludes_font_registry` | 0 | 0 | PASS |
| `P8.main_font_registry_after_imgui_context` | 1 | 1 | PASS |
| `P8.xsf_first_grid_bootstrap_guard` | 1 | 1 | PASS |
| `P8.cmake_registers_structure_module_sources` | 1 | 1 | PASS |
| `P8.cmake_registers_measurement_module_sources` | 1 | 1 | PASS |
| `P8.cmake_registers_density_module_sources` | 1 | 1 | PASS |
| `P8.legacy_createdAtoms_budget` | 117 | <=117 | PASS |
| `P8.legacy_createdBonds_budget` | 47 | <=47 | PASS |
| `P8.legacy_cellInfo_budget` | 250 | <=250 | PASS |

## 2. 실행 게이트 결과
- `npm run build-wasm:release`
  - 결과: `PASS`
  - 근거: `build_phase8_latest.txt` (`public/wasm/VTK-Workbench.js/.wasm/.data` install 경로 갱신 확인)
- `npm run test:cpp`
  - 결과: `PASS`
  - 근거: `unit_test_phase8_latest.txt` (`1/1` test passed)
- `npm run test:smoke`
  - 결과: `PASS`
  - 근거: `smoke_phase8_latest.txt` (`1 passed`)

## 3. W6 판정 근거 요약
| 지표 | Phase 8 기준선 | W6 시점 | 해석 |
|---|---:|---:|---|
| `atoms_template.cpp` `::Instance()` 호출 수 | 103 | 72 | 전역 singleton 의존 축소 진행 |
| `atoms_template.cpp` `VtkViewer::Instance()` 호출 수 | 75 | 46 | render 직접 결합 축소 진행 |
| `atoms_template.h` `friend class` 수 | 9 | 0 | 캡슐화 약화 포인트 제거 완료 |
| `structure/measurement/density` 모듈 디렉터리 | 미도입 | 도입 완료 | Phase 8 목표 모듈 골격 충족 |

추가 확인:
1. Phase 7 보호 규칙(폰트 초기화 순서, runtime prime에서 `FontRegistry()` 제외)이 유지된다.
2. VASP -> XSF(Grid) 회귀 방지용 첫 grid bootstrap 가드가 유지된다.
3. `model_tree`/`app`/`runtime` 경로가 새 feature 서비스 호출을 사용한다.

## 4. 잔여 리스크
| ID | 리스크 | 수준 | 대응 |
|---|---|---|---|
| R1 | `AtomsTemplate` 내부 `VtkViewer::Instance()`가 아직 46건 잔존 | Medium | Phase 9에서 `render` 외부 호출 0건 목표로 포트 전환 |
| R2 | legacy alias(`createdAtoms`, `createdBonds`, `cellInfo`) 참조량이 아직 높음 | Medium | Phase 10~12에서 compatibility 축소/제거 단계 진행 |
| R3 | 로컬 sandbox에서 `emsdk`/Playwright 실행 시 권한 제약 가능 | Low | 게이트 실행 시 권한 상승 경로를 표준 절차로 유지 |

## 5. 요약
- Phase 8 정적 게이트와 동적 게이트(빌드/C++ 테스트/스모크)가 모두 `PASS`다.
- `AtomsTemplate` 책임 분해의 핵심 목표(모듈 도입, friend class 제거, singleton 결합 축소)가 W6 기준으로 검증되었다.
- Phase 9(`VtkViewer` 해체 + render 포트 완성) 착수에 필요한 종료 문서 패키지가 준비되었다.
