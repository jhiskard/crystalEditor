# Phase 17-R6 세부 작업계획서 (최종 타깃 그래프 봉인 + 잔여 legacy 해체)

작성일: `2026-04-15`  
기준 문서: `docs/refactoring/refactoring_plan_root_legacy_dismantle_260408_53Codex.md`  
아키텍처 준수 기준: `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md`의 `3.1~3.7`  
정책 준수 기준: `docs/refactoring/refactoring_plan_architecture_completion_phase13plus_260406.md`의 주석/Git 정책

## 0. 목적

R6의 목표는 Phase 17-Root 트랙의 최종 봉인이다.

핵심 목표:
1. R5까지 잔존한 `atoms/legacy` 및 `WorkbenchRuntime::AtomsTemplateFacade()` 경로를 해체해 모듈 직접 조립으로 전환한다.
2. `wb_core/wb_ui/wb_atoms` 중심 임시 빌드 그래프를 `3.7 최종 타깃 그래프`로 전환한다.
3. 루트 allowlist(`main.cpp`, `app.cpp`, `app.h`)와 경계 게이트를 최종 상태로 봉인하고 종료 보고서를 작성한다.

범위 제외:
1. `atoms_template_bravais_lattice.cpp`, `atoms_template_periodic_table.cpp`는 R5에서 실행 경로 제외 처리 완료(현재 코드베이스 비존재)로 R6 직접 작업 범위에서 제외한다.

---

## 1. R0~R5 반영 상태 요약

| 단계 | 상태 | R6에 주는 의미 |
|---|---|---|
| R0 | 완료 | 루트 allowlist/ownership 게이트가 기본 품질선으로 고정됨 |
| R1 | 완료 | facade 계약 게이트 운영 중(경로 업데이트 포함) |
| R2 | 완료 | shell/layout/reset 계약이 게이트로 관리됨 |
| R3 | 완료 | render/platform/io 분리 기반과 port 경계가 마련됨 |
| R4 | 완료 | mesh 루트 해체 및 singleton 사용 축소 완료 |
| R5 | 완료 | `atoms_template.*` 루트 제거 완료, R6 착수 GO 판정 확보 |

R5 완료 근거:
1. `docs/refactoring/phase17-root/r5/dependency_gate_report.md`
2. `docs/refactoring/phase17-root/r5/go_no_go_r6.md`
3. `docs/refactoring/phase17-root/r5/refactoring_phase17_r5_w7_w8_atoms_facade_relocation_gate_execution_260414.md`

---

## 2. R6 착수 기준선 (2026-04-15)

### 2.1 현재 구조 지표

| 항목 | 현재 값 | 목표 값 |
|---|---:|---:|
| `webassembly/src` 루트 코드 파일(`*.cpp/*.h`) | 3 | 3 유지 |
| `webassembly/src/atoms` 루트 코드 파일(`*.cpp/*.h`) | 0 | 0 유지 |
| `webassembly/src/atoms` 하위 코드 파일(`*.cpp/*.h`) | 76 | 0 (또는 목적 모듈로 이관) |
| `atoms_template_facade.h` include 참조 | 36 | 0 |
| `AtomsTemplateFacade()` 호출 | 24 | 0 |
| `AtomsTemplate::Instance()` 호출 | 9 | composition root 내부만 허용 후 최종 0 지향 |

### 2.2 빌드 그래프 잔여 갭

현재 타깃:
1. `wb_core`
2. `wb_render`
3. `wb_mesh`
4. `wb_atoms`
5. `wb_io`
6. `wb_ui`

목표 타깃(3.7):
1. `wb_common`
2. `wb_platform`
3. `wb_shell`
4. `wb_workspace`
5. `wb_render`
6. `wb_mesh`
7. `wb_structure`
8. `wb_measurement`
9. `wb_density`
10. `wb_io`

핵심 잔여 부채:
1. `wb_atoms`에 40개 소스가 잔존하며 과대 모듈 상태가 유지되고 있다.
2. `wb_core/wb_ui` 네이밍과 소스 소유권이 목표 경계와 불일치한다.
3. `shell/runtime`이 `AtomsTemplateFacade()`를 통해 legacy 허브를 역참조하고 있다.

---

## 3. 실질 해체/이관 매핑 (R6 핵심)

### 3.1 Legacy facade 해체군

| 현재 파일 | 현재 책임 | 목표 모듈 | R6 처리 |
|---|---|---|---|
| `atoms/legacy/atoms_template_facade.*` | 구조/측정/밀도/패널/렌더 조정 허브 | `structure`, `measurement`, `density`, `shell`, `render` | API 기능군 분해 후 파일 삭제 |
| `shell/runtime/workbench_runtime.*`의 `AtomsTemplateFacade()` | legacy 진입점 | `StructureFeature`, `MeasurementFeature`, `DensityFeature` 직접 조립 | accessor 제거 |

### 3.2 Legacy adapter 해체군

| 현재 파일 | 목표 |
|---|---|
| `atoms/application/structure_service_legacy_port_adapter.*` | `structure/application` 직접 구현으로 치환 후 삭제 |
| `atoms/application/measurement_service_legacy_port_adapter.*` | `measurement/application` 직접 구현으로 치환 후 삭제 |
| `atoms/application/density_service_legacy_port_adapter.*` | `density/application` 직접 구현으로 치환 후 삭제 |

### 3.3 atoms 하위 기능군 분산군

| 기능군 | 현재 위치 | 목표 위치(최종) |
|---|---|---|
| 구조 lifecycle/visibility/read model | `atoms/application/*`, `atoms/domain/structure_state_store.*` | `structure/application|domain|presentation` |
| 측정 제어/기하 계산 | `atoms/application/measurement_controller.cpp`, `atoms/domain/measurement_geometry.*` | `measurement/application|domain|presentation` |
| density UI/상태/렌더 요청 | `atoms/presentation/*`, `atoms/ui/charge_density_ui.*`, `atoms/domain/charge_density.*` | `density/application|domain|presentation|infrastructure` |
| BZ/격자 UI | `atoms/ui/bz_plot_ui.*`, `atoms/ui/bravais_lattice_ui.*`, `atoms/domain/bz_plot.*`, `atoms/domain/special_points.h` | `structure/presentation` 또는 `shell/presentation` + `structure/domain` |
| atoms VTK/배치 렌더 헬퍼 | `atoms/infrastructure/*` | `render/infrastructure` 또는 feature-specific infrastructure |
| parser bridge | `atoms/infrastructure/file_io_manager.*`, `chgcar_parser.*` | `io/infrastructure` |

### 3.4 빌드 타깃 재배치군

| 현재 타깃 | 목표 타깃 | 처리 |
|---|---|---|
| `wb_core` | `wb_common`, `wb_workspace` | 소스 분리/이관 후 `wb_core` 제거 |
| `wb_ui` | `wb_shell` (+ 일부 `wb_platform`) | shell/runtime/presentation 소유화 후 `wb_ui` 제거 |
| `wb_atoms` | `wb_structure`, `wb_measurement`, `wb_density`, `wb_render`, `wb_io` | 소스 재배치 후 `wb_atoms` 제거 |

---

## 4. 단계별 WBS (실행 순서 고정)

### W0. R6 기준선 동결 + 영향도 인벤토리

작업:
1. `atoms/legacy`, `atoms/application/*legacy_port_adapter*`, `AtomsTemplateFacade()` 호출 위치 고정 로그 작성
2. CMake 타깃-소스 ownership 스냅샷 작성
3. R5 게이트 재실행으로 착수 기준 PASS 확인

완료 기준:
1. `docs/refactoring/phase17-root/r6/logs/r6_inventory_snapshot_latest.md` 생성

### W1. Runtime 조립 경로 전환

작업:
1. `workbench_runtime.*`에서 `AtomsTemplateFacade()` accessor 제거
2. `app.cpp`, `render/*`, `mesh/*`, `io/*`, `shell/*`의 facade 호출을 feature service/port 호출로 치환
3. composition root 외 `AtomsTemplate` 타입 노출 경로 차단

완료 기준:
1. `rg 'AtomsTemplateFacade\\(' webassembly/src` 결과 0

### W2. Structure/Measurement/Density 직접 소유 완결

작업:
1. legacy port adapter 3종 제거
2. `structure_service`, `measurement_service`, `density_service`가 각자 저장소/규칙을 직접 소유하도록 정렬
3. `atoms/application/structure_lifecycle_service.cpp`, `visibility_service.cpp`, `measurement_controller.cpp` 책임을 각 feature로 이관

완료 기준:
1. `atoms/application/*legacy_port_adapter*` 파일 0
2. feature application에서 `atoms/legacy` include 0

### W3. atoms UI/Presentation 최종 분산

작업:
1. `atoms/presentation/*`, `atoms/ui/*` 엔트리들을 `shell/structure/measurement/density` presentation으로 분산
2. Reset/Layout 안정화 계약이 분산된 경로에서도 동일 동작하도록 유지
3. `atoms_template_main_window_ui.*` 및 facade 중심 UI 루트 제거

완료 기준:
1. `atoms/ui/*`, `atoms/presentation/*`의 runtime 진입점 잔존 0

### W4. atoms domain/infrastructure 잔여 해체

작업:
1. `atoms/domain/*`, `atoms/infrastructure/*`를 목표 모듈(`structure/measurement/density/render/io`)로 재배치
2. 파서 관련(`file_io_manager`, `chgcar_parser`)은 `io/infrastructure`로 고정
3. VTK 렌더 헬퍼(`vtk_renderer`, `bond_renderer`, `batch_update_system` 등)는 `render/infrastructure`로 편입

완료 기준:
1. `webassembly/src/atoms` 하위 코드 파일(`*.cpp/*.h`) 0

### W5. CMake 최종 타깃 그래프 전환

작업:
1. `webassembly/cmake/modules`를 최종 타깃 체계로 재구성:
   - `wb_common.cmake`, `wb_platform.cmake`, `wb_shell.cmake`, `wb_workspace.cmake`,
   - `wb_render.cmake`, `wb_mesh.cmake`, `wb_structure.cmake`, `wb_measurement.cmake`, `wb_density.cmake`, `wb_io.cmake`
2. `CMakeLists.txt`의 `WB_MODULE_TARGETS`와 link 순서를 최종 설계로 교체
3. `wb_core`, `wb_ui`, `wb_atoms` 제거

완료 기준:
1. `CMakeLists.txt`에 `wb_atoms`, `wb_core`, `wb_ui` 참조 0
2. 최종 타깃 10종으로 빌드 가능

### W6. 아키텍처 게이트 봉인

작업:
1. 신규 게이트 도입:
   - `scripts/refactoring/check_phase17_r6_atoms_legacy_zero.ps1`
   - `scripts/refactoring/check_phase17_target_graph_final.ps1`
2. 기존 게이트 업데이트:
   - `check_phase17_root_allowlist.ps1`
   - `check_phase17_root_ownership_map.ps1`
   - `check_phase17_facade_contract.ps1`
3. target graph 비순환성 및 금지 include 규칙 검증

완료 기준:
1. R6 게이트 PASS

### W7. 통합 검증 + 회귀 테스트

작업:
1. `npm run build-wasm:release`
2. `npm run test:cpp`
3. `npm run test:smoke`
4. 수동 회귀(Reset/Layout 1/2/3, Edit/Build/Data, BZ, import)

완료 기준:
1. 자동/수동 검증 PASS

### W8. 종료 문서 패키지

작업:
1. `docs/refactoring/phase17-root/r6/dependency_gate_report.md` 작성
2. `docs/refactoring/phase17-root/r6/go_no_go_phase17_close.md` 작성
3. `docs/refactoring/refactoring_result_report_phase17_root_legacy_dismantle_2604xx.md` 작성

완료 기준:
1. Phase 17-Root 종료 문서 세트 완성

---

## 5. 코드 변경 파일 목록(예상)

필수 변경:
1. `CMakeLists.txt`
2. `webassembly/cmake/modules/wb_core.cmake` (제거 대상)
3. `webassembly/cmake/modules/wb_ui.cmake` (제거 대상)
4. `webassembly/cmake/modules/wb_atoms.cmake` (제거 대상)
5. `webassembly/src/shell/runtime/workbench_runtime.h`
6. `webassembly/src/shell/runtime/workbench_runtime.cpp`
7. `webassembly/src/app.cpp`
8. `webassembly/src/io/application/import_apply_service.cpp`
9. `webassembly/src/io/application/import_orchestrator.cpp`
10. `webassembly/src/io/application/import_workflow_service.cpp`
11. `webassembly/src/render/presentation/viewer_window.cpp`
12. `webassembly/src/mesh/presentation/model_tree_*.cpp`

신규(예상):
1. `webassembly/cmake/modules/wb_common.cmake`
2. `webassembly/cmake/modules/wb_platform.cmake`
3. `webassembly/cmake/modules/wb_shell.cmake`
4. `webassembly/cmake/modules/wb_workspace.cmake`
5. `webassembly/cmake/modules/wb_structure.cmake`
6. `webassembly/cmake/modules/wb_measurement.cmake`
7. `webassembly/cmake/modules/wb_density.cmake`
8. `scripts/refactoring/check_phase17_r6_atoms_legacy_zero.ps1`
9. `scripts/refactoring/check_phase17_target_graph_final.ps1`

삭제(목표):
1. `webassembly/src/atoms/legacy/atoms_template_facade.cpp`
2. `webassembly/src/atoms/legacy/atoms_template_facade.h`
3. `webassembly/src/atoms/application/*legacy_port_adapter*`
4. `webassembly/cmake/modules/wb_atoms.cmake`
5. `webassembly/cmake/modules/wb_core.cmake`
6. `webassembly/cmake/modules/wb_ui.cmake`

---

## 6. 검증 계획

### 6.1 자동 검증

1. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_root_allowlist.ps1`
2. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_root_ownership_map.ps1`
3. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_facade_contract.ps1`
4. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_r2_shell_ui_migration.ps1`
5. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_r3_render_platform_io_migration.ps1`
6. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_r4_mesh_migration.ps1`
7. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_mesh_singleton_usage.ps1`
8. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_r5_atoms_migration.ps1`
9. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_r5_shell_layout_reset_contract.ps1`
10. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_r6_atoms_legacy_zero.ps1` (신규)
11. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_target_graph_final.ps1` (신규)
12. `npm run build-wasm:release`
13. `npm run test:cpp`
14. `npm run test:smoke`

### 6.2 수동 회귀 포인트

1. 초기 진입 직후 `Reset` 동작 안정성(반복 초기화/창 즉시 닫힘 재발 금지)
2. `Layout 1/2/3` 프리셋 적용 차등성 유지
3. `Edit/Build/Data` 메뉴 경로 창 열림/유지 동작
4. `Utilities > Brillouin Zone` 동작 및 lattice 표시 경로
5. import(XSF/CHGCAR/UNV) 후 구조/밀도/메시 트리 동기화

---

## 7. 리스크 및 대응

| 리스크 | 수준 | 대응 |
|---|---|---|
| `atoms/legacy` 해체 중 기능 누락 | High | W1~W4를 기능군별 독립 커밋 + 시나리오 회귀 체크리스트 고정 |
| 타깃 그래프 재편 중 링크/순환 의존 회귀 | High | `check_phase17_target_graph_final.ps1` + clean rebuild 필수 |
| facade 제거 후 import/render 경로 파손 | High | io/render 경로 전용 스모크 시나리오 추가 |
| R2 Reset/Layout 계약 재회귀 | High | R5 shell-layout 게이트를 R6 필수 게이트로 승격 |
| 대규모 파일 이동으로 리뷰 난이도 상승 | Medium | WBS 단위 커밋 분리와 로그/근거 문서 동시 업데이트 |

---

## 8. R6 완료 기준 (최종 봉인)

1. `webassembly/src` 루트 코드 파일은 `main.cpp`, `app.cpp`, `app.h`만 유지된다.
2. `webassembly/src/atoms` 하위 포함 코드 파일(`*.cpp/*.h`)이 0개다.
3. `AtomsTemplateFacade()` 호출이 0건이다.
4. `atoms_template_facade.h` include 참조가 0건이다.
5. `CMakeLists.txt`의 모듈 타깃이 `wb_common~wb_io` 최종 10종으로 구성된다.
6. `wb_atoms`, `wb_core`, `wb_ui` 참조가 0건이다.
7. `check_phase17_target_graph_final.ps1` PASS.
8. `build-wasm`, `test:cpp`, `test:smoke` PASS.
9. R6 게이트 보고서/종료 판정서/최종 결과보고서가 작성된다.

