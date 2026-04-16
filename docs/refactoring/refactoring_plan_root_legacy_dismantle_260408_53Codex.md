# webassembly/src 루트 원본 코드 해체 세부 작업계획서 (Phase 17-Root Legacy Dismantle)

작성일: `2026-04-08`  
문서 인코딩: `UTF-8`

기준 문서:
- `docs/refactoring/refactoring_plan_260324.md`
- `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md`
- `docs/refactoring/refactoring_plan_architecture_completion_phase13plus_260406.md`
- `docs/refactoring/refactoring_plan_root_legacy_dismantle_260408.md`

적용 정책:
- 주석 정책: `refactoring_plan_architecture_completion_phase13plus_260406.md`의 `4.1 주석 작성 정책 v2`
- Git 정책: `refactoring_plan_architecture_completion_phase13plus_260406.md`의 `4.2 Git 저장소 운영 정책 v2`

---

## 0. 배경과 목적

1. Phase 1~16 수행으로 모듈 분리와 런타임 경계는 상당 부분 진행되었으나, `webassembly/src` 및 `webassembly/src/atoms` 루트에 원본 구현 파일이 대량 잔존한다.
2. 현재 구조는 문서상 목표 아키텍처(3.1~3.7)와 실제 파일 소유권이 불일치하며, 특히 `main.cpp`, `app.h/cpp` facade 축소 목표가 미완결 상태다.
3. 본 계획의 목표는 루트 원본 코드를 해체하여, 루트에는 `main.cpp`, `app.cpp`, `app.h`만 facade로 남기고 모든 실질 로직을 `3.2 목표 모듈 경계`에 맞춰 이관하는 것이다.
4. 본 계획은 Phase 17 종료 조건을 대체하지 않고, **Phase 17 완료를 위한 선행/보강 트랙(Phase 17-R)** 으로 운영한다.

---

## 1. 준수 기준 (3.1~3.7 추적 매트릭스)

| 설계 기준 | 준수해야 할 규칙 | 본 계획의 강제 장치 |
|---|---|---|
| 3.1 핵심 원칙 | composition root 외 singleton lookup 금지, bounded context 분리, port/adapter 격리, 상태 단일 소유, build seal | `R1~R6` 단계별 정적 게이트 + 루트 allowlist + 타깃 그래프 게이트 |
| 3.2 목표 모듈 경계 | `atoms` 해체, `app.cpp` shell 축소, `VtkViewer/AtomsTemplate/MeshManager/FileLoader/ModelTree/Toolbar` singleton 제거 | 파일군별 해체 매핑표(2.2) + 단계별 제거 DoD(4장) |
| 3.3 목표 디렉터리 레이아웃 | `platform/worker`, `platform/persistence`, `shell/presentation`, `workspace/presentation` 등 정렬 | `R0` 디렉터리 보강, `R6`에서 잔여 경로 정리 |
| 3.4 Composition Root | 조립은 `shell/runtime/WorkbenchRuntime` 단일화 | `R1`에서 `main/app` 책임 재정의, `R4~R6`에서 패널/서비스 생성자 주입 강제 |
| 3.5 상태 소유권 모델 | 저장소 단일 소유, read model 파생, feature의 VTK 핸들 직접 소유 금지 | 상태 소유권 점검표 + include/역참조 금지 게이트 |
| 3.6 포트/어댑터 계약 | domain/application의 `EM_ASM`, `vtk*`, `imgui.h` 직접 include 금지 | `R3~R5`에서 포트 치환 + 계약 위반 grep 게이트 |
| 3.7 빌드 타깃 최종 설계 | `wb_common~wb_io` 최종 타깃, `wb_atoms` 제거, 순환 의존 금지 | `R6` 타깃 재편 + clean rebuild + target graph 검증 |

---

## 2. 현재 기준선 (2026-04-08 스냅샷)

### 2.1 루트 코드 파일 수

| 위치 | `*.cpp/*.h` 수 | 목표 |
|---|---:|---:|
| `webassembly/src` 루트 | 40 | 3 |
| `webassembly/src/atoms` 루트 | 2 | 0 |
| 합계 | 42 | 3 |

루트 최종 allowlist 목표:
- `webassembly/src/main.cpp`
- `webassembly/src/app.cpp`
- `webassembly/src/app.h`

### 2.2 루트 파일군 해체 매핑 (원본 코드 기준)

| 파일군 | 현재 책임 | 목표 모듈 경계(3.2) | 해체 원칙 |
|---|---|---|---|
| `bind_function.cpp` | wasm 바인딩 레거시 경로 | `platform/wasm` | `workbench_bindings.cpp`로 완전 치환 후 삭제 |
| `font_manager.cpp/h` | 폰트/아이콘 자원 관리 | `shell/presentation` 또는 `platform` | UI 자원 계층으로 이관, facade 직접 의존 제거 |
| `custom_ui.cpp/h` | 공통 ImGui 헬퍼 | `shell/presentation` | shell 전용 위젯 헬퍼로 정리 |
| `image.cpp/h`, `texture.cpp/h` | 이미지/텍스처 래퍼 | `render/infrastructure` | 렌더 자원 어댑터화 |
| `test_window.cpp/h` | 테스트 패널 | `shell/presentation` | 디버그 패널 모듈화 |
| `toolbar.cpp/h` | 메뉴/툴바 + 기능 제어 | `shell/presentation` | command dispatch만 남기고 feature 동작은 application 서비스 위임 |
| `vtk_viewer.cpp/h` | viewport/camera/picking/overlay | `render/application`, `render/infrastructure` | viewer facade 분해 후 root 삭제 |
| `mouse_interactor_style.cpp/h` | VTK 인터랙션 스타일 | `render/infrastructure` | adapter 편입 |
| `mesh.cpp/h`, `mesh_group.cpp/h`, `lcrs_tree.cpp/h` | mesh 모델/트리 | `mesh/domain` | 도메인 엔티티/트리 모델로 이동 |
| `mesh_manager.cpp/h` | mesh 조회/가시성/조정 | `mesh/application`, `mesh/domain` | repository/query/command service로 분해 |
| `mesh_detail.cpp/h`, `mesh_group_detail.cpp/h`, `model_tree.cpp/h` | mesh UI 패널 | `mesh/presentation` | panel/presenter로 이동 |
| `file_loader.cpp/h`, `unv_reader.cpp/h` | import orchestration/parser | `io/application`, `io/infrastructure`, `platform/browser`, `platform/worker` | workflow+parser+worker 분리 |
| `atoms_template_bravais_lattice.cpp` | Bravais lattice 코드 설명 자료(실행 경로 아님) | 계획 배제 | 해체/삭제 대상에서 제외(설명 자료 유지) |
| `atoms_template_periodic_table.cpp` | periodic table 코드 설명 자료(실행 경로 아님) | 계획 배제 | 해체/삭제 대상에서 제외(설명 자료 유지) |
| `atoms/atoms_template.cpp/h` | atoms god-object + 다중 기능 | `structure`, `measurement`, `density`, `shell/presentation`, `render/application` | 기능 단위 분해 후 파일 제거 |

### 2.3 구조적 갭 (3.3 대비)

현재 부재/보강 필요 디렉터리:
- `webassembly/src/platform/worker`
- `webassembly/src/platform/persistence`
- `webassembly/src/shell/presentation`
- `webassembly/src/workspace/presentation`
- `webassembly/src/render/presentation`
- `webassembly/src/structure/infrastructure`
- `webassembly/src/io/platform`

### 2.4 빌드 타깃 갭 (3.7 대비)

현재 CMake 기준 모듈 타깃:
- `wb_core`, `wb_render`, `wb_mesh`, `wb_atoms`, `wb_io`, `wb_ui`

목표 CMake 타깃(3.7):
- `wb_common`, `wb_platform`, `wb_shell`, `wb_workspace`, `wb_render`, `wb_mesh`, `wb_structure`, `wb_measurement`, `wb_density`, `wb_io`

정리 필요 항목:
- `wb_atoms` 제거
- `wb_core/wb_ui`를 목표 경계 기준으로 분해
- 타깃 간 비순환 의존 고정

### 2.5 고위험 대형 파일

| 파일 | 라인 수(스냅샷) | 리스크 |
|---|---:|---|
| `webassembly/src/font_manager.cpp` | 33802 | 초대형 단일 파일로 충돌/회귀 위험 매우 높음 |
| `webassembly/src/atoms/atoms_template.cpp` | 5208 | god-object 분해 시 상태/계약 동시 붕괴 위험 |
| `webassembly/src/atoms/atoms_template.h` | 1339 | 광범위 include 파급 가능성 |
| `webassembly/src/vtk_viewer.cpp` | 1996 | 렌더/feature 경계 혼재 |
| `webassembly/src/app.cpp` | 1284 | facade 범위 초과 책임 누적 |

---

## 3. 목표 완료 상태 정의

### 3.1 루트 파일 규칙

1. `webassembly/src` 루트 코드 파일은 `main.cpp`, `app.cpp`, `app.h`만 허용한다.
2. `webassembly/src/atoms` 루트 코드 파일은 0개다.
3. 루트 신규 `*.cpp/*.h` 생성 시 게이트에서 즉시 FAIL 처리한다.

### 3.2 facade 계약

`main.cpp` 허용 책임:
- GLFW/Emscripten/ImGui 초기화
- `WorkbenchRuntime` 생성/실행/종료

`app.h/cpp` 허용 책임:
- 메뉴/도킹/창 가시성 등 shell presentation 조정
- runtime API 호출을 통한 위임

`main.cpp`, `app.h/cpp` 금지 책임:
- feature domain/application 직접 구현
- parser/import orchestration 구현
- VTK actor/volume 직접 소유
- singleton 전역 오케스트레이션

### 3.3 모듈 경계와 상태 소유권

1. 구조 상태는 `structure::domain::StructureRepository` 단일 소유.
2. mesh 상태는 `mesh::domain::MeshRepository` 단일 소유.
3. 선택/활성 문맥은 `workspace::domain::WorkspaceStore` 단일 소유.
4. 측정 상태는 `measurement::domain::MeasurementRepository` 단일 소유.
5. density 상태는 `density::domain::DensityRepository` 단일 소유.
6. render 핸들(actor/volume/camera)은 `render::infrastructure` 단일 소유.

---

## 4. 실행 계획 (Phase 17-R: R0~R6)

운영 원칙:
- 큰 파일을 한 번에 이동하지 않고, **파일군 단위 분해 + 컴파일 고정 + 다음 파일군** 순으로 진행한다.
- 각 단계 종료마다 build/test/gate 로그와 go/no-go 문서를 남긴다.

### R0. 기준선 동결 + 게이트 도입 (2일)

목표:
- 해체 대상 범위와 allowlist를 자동검사 가능한 상태로 고정

작업:
1. 루트 파일 인벤토리(`42개`)를 문서/스크립트로 고정.
2. `check_phase17_root_allowlist.ps1` 추가.
3. `check_phase17_root_ownership_map.ps1` 추가.
4. 누락 디렉터리 생성(`shell/presentation`, `platform/worker`, `platform/persistence` 등).

완료 기준:
- 루트 allowlist와 ownership gate가 CI/로컬에서 동일 동작.

### R1. Facade 축소 선행 (2~3일)

목표:
- `main.cpp`, `app.h/cpp`의 책임 한계를 먼저 고정해 이후 이관 흔들림 방지

작업:
1. `main.cpp`에서 feature 직접 접근 제거 여부 점검.
2. `app.cpp`의 루트 include(`font_manager.h`, `vtk_viewer.h`, `mesh*.h`, `atoms_template.h`) 의존 목록 축소 계획 확정.
3. `check_phase17_facade_contract.ps1` 도입(금지 include, 금지 타입 사용 검사).

완료 기준:
- facade 계약 위반 0, 이후 단계에서 facade 복잡도 역증가 차단.

### R2. Shell/UI 파일군 이관 (4~6일)

목표:
- UI helper와 메뉴 계층을 `shell/presentation` 중심으로 이동

대상:
- `font_manager.*`, `custom_ui.*`, `test_window.*`, `toolbar.*`
- 필요 시 `image.*`, `texture.*` 일부 분리

작업:
1. UI 자원(폰트/아이콘)과 위젯 헬퍼를 shell presentation 계층으로 분리.
2. 툴바 메뉴는 command dispatch만 유지하고 feature 동작은 controller/service 위임.
3. `app.cpp`에서 직접 상태 조작 코드 제거.

완료 기준:
- shell/UI 루트 파일 0개.
- `app.cpp`는 shell facade + 위임만 유지.

### R3. Render/Platform/IO 파일군 이관 (5~7일)

목표:
- 렌더/플랫폼/IO의 포트/어댑터 경계를 실제 코드로 고정

대상:
- `vtk_viewer.*`, `mouse_interactor_style.*`, `bind_function.cpp`, `file_loader.*`, `unv_reader.*`

작업:
1. viewer/interaction 로직을 `render/application`과 `render/infrastructure`로 분해.
2. 파일 선택/백그라운드 파싱 경로를 `FileDialogPort`, `WorkerPort` 기준으로 재배치.
3. wasm 바인딩은 `platform/wasm/workbench_bindings.cpp` 경로로 단일화.
4. `io/platform` 디렉터리 신설 및 browser/worker adapter 정리.

완료 기준:
- render/io/platform 관련 루트 파일 0개.
- 3.6 금지 규칙 위반 include 0.

### R4. Mesh 파일군 이관 (4~6일)

목표:
- mesh domain/application/presentation 3계층 분리를 완료

대상:
- `mesh.*`, `mesh_group.*`, `lcrs_tree.*`, `mesh_manager.*`, `mesh_detail.*`, `mesh_group_detail.*`, `model_tree.*`

작업:
1. domain 엔티티/트리 모델과 application 서비스를 분리.
2. `MeshManager` 역할을 repository + query/command service로 치환.
3. panel/presenter는 `mesh/presentation`으로 완전 이동.

완료 기준:
- mesh 관련 루트 파일 0개.
- `MeshManager` 루트 singleton 경로 제거.

### R5. Atoms 원본 해체 (6~9일)

목표:
- `webassembly/src/atoms/atoms_template.*`를 완전 제거
- `atoms_template_bravais_lattice.cpp`, `atoms_template_periodic_table.cpp`는 실행용 파일이 아닌 설명 자료로 분류하여 본 해체 계획에서 제외

대상:
- `atoms/atoms_template.*`

작업:
1. 구조 편집/lifecycle는 `structure`로 이동.
2. 측정 계산/overlay descriptor 생성은 `measurement`로 이동.
3. density(grid/isosurface/slice)는 `density`로 이동.
4. builder/editor/advanced 패널 UI는 `structure/density/measurement` presentation으로 분산.
5. 임시 호환 경로는 Doxygen `Temporary compatibility path` + 제거 Phase + 추적 ID를 강제.

완료 기준:
- `webassembly/src/atoms` 루트 코드 파일 0개.
- `AtomsTemplate` 직접 의존 경로 0.

### R6. 빌드 타깃 최종 봉인 + 종료 패키지 (3~5일)

목표:
- 3.7 최종 타깃 그래프로 전환하고 루트 레거시 재유입을 봉쇄

작업:
1. `wb_core/wb_ui/wb_atoms` 중심 그래프를 `wb_common~wb_io` 목표 그래프로 재편.
2. `CMakeLists.txt`, `webassembly/cmake/modules/*` 소스 ownership 정렬.
3. `check_phase17_target_graph_final.ps1` + clean rebuild 수행.
4. 최종 게이트(build/test/smoke/architecture) 통합 실행.
5. 결과보고서 및 종료 판정 문서 작성.

완료 기준:
- `wb_atoms` 참조 0.
- 타깃 그래프 비순환 PASS.
- 루트 allowlist 위반 0.

---

## 5. 검증 및 게이트

### 5.1 공통 명령 게이트

모든 R단계 종료 시 실행:
1. `npm run build-wasm:release`
2. `npm run test:cpp`
3. `npm run test:smoke`
4. 단계별 정적 게이트 스크립트

### 5.2 정적 게이트 목록

1. `scripts/refactoring/check_phase17_root_allowlist.ps1`
2. `scripts/refactoring/check_phase17_root_ownership_map.ps1`
3. `scripts/refactoring/check_phase17_facade_contract.ps1`
4. `scripts/refactoring/check_phase17_port_adapter_contract.ps1`
5. `scripts/refactoring/check_phase17_target_graph_final.ps1`

### 5.3 수동 회귀 체크(정책 4.3 연계)

다음 메뉴 경로는 각 단계 최소 1회 점검:
1. `Edit > Atoms | Bonds | Cell`
2. `Build > Add atoms | Bravais Lattice Templates`
3. `Data > Isosurface | Surface | Volumetric | Plane`
4. `Utilities > Brillouin Zone`

---

## 6. 주석/문서 정책 적용

### 6.1 주석 정책 적용 항목

1. 신규/변경 public API는 Doxygen `@brief` 필수.
2. 경계 계약 설명은 `@details` 또는 `@note` 필수.
3. 임시 경로는 아래 3개를 모두 기입.
   - `@note Temporary compatibility path`
   - 제거 목표 Phase
   - 추적 ID
4. 동작 변경 코드의 기존 주석 미갱신은 FAIL 처리.

### 6.2 단계별 산출물 형식

각 R단계 산출물:
1. `docs/refactoring/phase17-root/rN/refactoring_phase17_rN_<topic>_2604xx.md`
2. `docs/refactoring/phase17-root/rN/dependency_gate_report.md`
3. `docs/refactoring/phase17-root/rN/go_no_go_r{N+1}.md`
4. `docs/refactoring/phase17-root/rN/logs/*`

최종 산출물:
- `docs/refactoring/refactoring_result_report_phase17_root_legacy_dismantle_2604xx.md`

---

## 7. Git 운영 계획 (정책 4.2 적용)

브랜치 운영:
1. 기본: `refactor/phase17-final-architecture-seal`에서 `phase17 root-dismantle` 작업 트랙으로 진행.
2. Phase 17 종료 후 보완인 경우: `hotfix/phase17-root-legacy-dismantle` 사용.

커밋 규칙:
1. WBS 단위 커밋.
2. 커밋 메시지에 `phase17 Rx` 포함.
3. rebase/squash로 WBS 추적성 제거 금지.
4. force push 금지(긴급 승인 예외).

로그 추적:
1. build/test/gate 로그에 명령/시각/결과를 함께 기록.
2. `logs/` 파일 추적 누락이 없도록 `.gitignore` 선확인.

---

## 8. 리스크 및 대응

| ID | 리스크 | 수준 | 대응 |
|---|---|---|---|
| RL-1 | `atoms_template` 분해 중 기능 누락 | High | 기능 단위 체크리스트 + 패널별 회귀 시나리오 고정 |
| RL-2 | `font_manager.cpp` 대형 변경 충돌 | High | 자원 데이터/로직 분리 커밋, 파일군 분할 머지 |
| RL-3 | `app.cpp` facade 축소 중 UI 동작 회귀 | High | `check_phase17_facade_contract` + 메뉴 수동 체크 |
| RL-4 | render/feature 경계 재혼합 | High | 포트 계약 위반 grep 게이트 상시 실행 |
| RL-5 | 타깃 재편 중 링크/순환 회귀 | Medium | clean build + target graph 스크립트 필수 |
| RL-6 | 루트 파일 재유입 | Medium | allowlist 게이트를 PR 필수 체크로 고정 |

---

## 9. 최종 완료 기준 (Definition of Done)

1. `webassembly/src` 루트 코드 파일은 `main.cpp`, `app.cpp`, `app.h`만 남는다.
2. `webassembly/src/atoms` 루트 코드 파일은 0개다.
3. `main.cpp`는 runtime 부트스트랩만, `app.h/cpp`는 shell facade만 담당한다.
4. `atoms` 원본 로직은 `structure/measurement/density` 및 연관 presentation 계층으로 분산된다.
5. `3.1~3.7` 위반 항목이 정적 게이트/테스트/문서 기준으로 0건이다.
6. 최종 빌드 타깃이 `wb_common`, `wb_platform`, `wb_shell`, `wb_workspace`, `wb_render`, `wb_mesh`, `wb_structure`, `wb_measurement`, `wb_density`, `wb_io`로 정렬되고, `wb_atoms`는 제거된다.
7. `build-wasm`, `test:cpp`, `test:smoke`, architecture gate, root allowlist gate를 모두 PASS한다.

---

## 10. 착수 순서

1. `R0` 기준선/게이트
2. `R1` facade 축소
3. `R2` shell/UI 이관
4. `R3` render/platform/io 이관
5. `R4` mesh 이관
6. `R5` atoms 해체
7. `R6` 타깃 봉인/종료 패키지

순차 원칙:
- 앞 단계 go/no-go PASS가 다음 단계 착수 조건이다.
- 루트 allowlist 예외는 어떤 단계에서도 늘리지 않는다.
