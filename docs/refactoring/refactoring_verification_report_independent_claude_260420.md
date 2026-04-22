# 리팩토링 계획-코드 독립 검증 보고서 (Phase 0~18)

작성일: 2026-04-20
대상 저장소: `vtk-workbench_jclee`
기준 브랜치: `main`
작성 목적: 각 리팩토링 계획서(Phase 0~18)의 핵심 요구사항이 실제 코드에 반영되었는지를, 기존 `refactoring_result_report_*.md` 문서와 독립적으로 재확인한다.

---

## 0. 검증 범위와 방법

### 0.1 기준 문서

| 구분 | 파일 | 적용 Phase |
|---|---|---|
| 최초 계획서 | `docs/refactoring/refactoring_plan_260324.md` | Phase 1~6 |
| 모듈 아키텍처 계획서 | `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md` | Phase 7~12 |
| 아키텍처 완결 계획서 | `docs/refactoring/refactoring_plan_architecture_completion_phase13plus_260406.md` | Phase 13~17 |
| Root Legacy 해체 계획서 | `docs/refactoring/refactoring_plan_root_legacy_dismantle_260408_53Codex.md` | Phase 17-Root (R0~R6) |
| Phase 18 세부계획서 | `docs/refactoring/phase18/refactoring_phase18_phase17_limit_closure_execution_plan_260416.md` | Phase 18 |

### 0.2 검증 방법

1. 파일 구조 확인: 계획에서 명시한 모듈/디렉터리/파일의 실재 여부를 `ls`, `find`, `Glob`로 확인
2. 코드 내용 심층 분석: 핵심 API, 헤더, 클래스, `::Instance()`, `DECLARE_SINGLETON`, `friend class`, legacy alias 등을 `Grep`으로 정량 측정
3. 계획 대비 누락 항목 식별: DoD/완료 기준과 현재 코드 상태를 대조하여 달성/부분 달성/미달성 판정

### 0.3 본 보고서의 입장

- 본 보고서는 `docs/refactoring/refactoring_result_report_*.md`와는 독립적으로 작성되었다.
- 동일 지표에 대한 결과가 일치하면 교차 검증을 통해 신뢰도가 높아진다.
- 불일치가 있을 경우 현재 코드 상태를 우선하며, 그 차이를 본문에 명시한다.

---

## 1. 종합 결론

### 1.1 프로그램 전체 판정

`vtk-workbench_jclee` 저장소의 현재 상태는 **"계획된 아키텍처 전환 목표의 대부분을 달성했다"**고 판단된다. 특히 god-object 해체, 싱글턴 격리, 모듈 분리, 빌드 타깃 재구성, 테스트/게이트 체계 도입은 계획서의 수치 기준을 실제 코드에서 충족하고 있다.

다만 다음 두 가지 측면에서 "완전한 완결"이라고 보기는 어렵다.

1. **완전한 해체가 아니라 "legacy 격리(quarantine)" 방식의 종료**: 계획서가 지향했던 god-object(`AtomsTemplate`, `VtkViewer`)의 **완전한 해체**는 달성되지 않았고, 대신 `workspace/legacy/`, `render/application/legacy_viewer_facade.*`로의 **논리적 격리** 방식으로 종료되었다.
2. **Phase 11/12의 `app.cpp` 슬림화 기준 미충족**: `app.cpp`는 현재 1,430 라인이며, 34개의 `App::*` 메서드가 메뉴/레이아웃/폰트/팝업/창 관리를 수행한다. Phase 11 DoD의 "프레임 오케스트레이션만 담당" 기준에는 도달하지 못했다.

### 1.2 핵심 지표 (본 보고서 측정값)

| 지표 | 계획 최종 목표 | 측정값(2026-04-20) | 기존 보고서값 | 판정 |
|---|---:|---:|---:|---|
| `webassembly/src` 루트 코드 파일 수 | 3 (main/app) | 3 | 3 | 일치, 달성 |
| `webassembly/src/atoms/` 하위 코드 파일 수 | 0 | 0 (디렉터리 자체 없음) | 0 | 일치, 달성 |
| CMake 최종 모듈 타깃 수 | 10 | 10 | 10 | 일치, 달성 |
| `wb_atoms` / `wb_core` / `wb_ui` 참조 | 0 | 0 | 0 | 일치, 달성 |
| `AtomsTemplate::Instance()` 호출 | 0 | 0 | 0 | 일치, 달성 |
| `DECLARE_SINGLETON(AtomsTemplate)` | 0 | 0 | 0 | 일치, 달성 |
| `DECLARE_SINGLETON` 실사용 선언 | 최종 0 또는 composition root 한정 | 3 (VtkViewer, MeshManager, FontManager) | 3 | 일치, 부분 달성 |
| `::Instance()` 전체 호출 | 최종 0 또는 composition root 한정 | 48 | 48 | 일치, 부분 달성 |
| `friend class` 전체 | 최종 0 | 1 (`io/application/import_entry_service.h`) | 0 (atoms_template.h 기준) | **불일치, 미달성** |
| `workspace/legacy/atoms_template_facade.h` 외부 include | 0 (legacy 디렉터리 외부) | 0 | 0 | 일치, 달성 |
| `.gitkeep` / 빈 디렉터리 / `.DS_Store` / `.vscode` | 0 | 0 | 0 | 일치, 달성 |
| `workspace/legacy/atoms_template_facade.cpp` 크기 | facade 수준 축소 | 5,808 lines | 5,808 | 일치, **규모 면에서 미달성** |
| `app.cpp` 크기 | "프레임 오케스트레이션만 담당" | 1,430 lines | 보고서에서 수치 미언급 | 판단 근거로 **부분 달성** |

---

## 2. Phase별 상세 검증

### 2.1 Phase 1 — 의존 경계 정리

**계획 목표** (`refactoring_plan_260324.md` 9장 매핑표 + Phase 1 세부계획서)

- `ModelTree`의 atoms 전역 직접 접근 제거
- `atoms/domain`, `atoms/infrastructure`의 `VtkViewer::Instance()` 직접 호출 제거
- `RenderGateway` 도입
- `model_tree.cpp`의 섹션별 분리

**검증 결과**

| 항목 | 상태 |
|---|---|
| `webassembly/src/mesh/presentation/model_tree_*_section.cpp` 존재 | 달성 (`model_tree_structure_section.cpp`, `model_tree_mesh_section.cpp` 등 실재) |
| `render/application/render_gateway.h` 도입 | 달성 (`render/application/render_gateway.h` 존재) |
| `render/infrastructure/vtk_render_gateway.*` 도입 | 달성 (파일 실재) |
| `atoms/domain`의 `extern` 선언 | 달성 (`extern ...createdAtoms|createdBonds|cellInfo` 0건) |

판정: **달성**. 경계 정리 방향은 현재 구조에 실제로 반영되어 있다.

---

### 2.2 Phase 2 — 상태 저장소 통합

**계획 목표**

- `StructureStateStore` 도입
- `createdAtoms`, `createdBonds`, `atomGroups`, `bondGroups`, `cellInfo` 등을 구조 ID 기반 저장소로 통합
- `id_generator.h`, `bond_settings.h` 분리

**검증 결과**

| 항목 | 상태 |
|---|---|
| `structure_state_store.h` 존재 | 달성 (`webassembly/src/structure/domain/atoms/structure_state_store.h`) |
| 구조 ID 기반 관리 | 달성 (`StructureRepository`가 `structure/domain/structure_repository.h`로 존재) |
| `extern` 전역 제거 | 달성 (grep 결과 0건) |
| legacy alias(`createdAtoms|createdBonds|cellInfo`) 내부 잔존 | 부분 달성 (이름 자체는 302건 출현하지만, 모두 `repository.CreatedAtoms()` 반환값의 지역 바인딩, 메서드 이름, 주석이며, 전역 변수/`using` 선언은 0건) |

판정: **달성(의미적 기준)**. legacy alias 이름은 코드 내 변수/메서드명으로 남았지만, 전역 alias 자체는 제거되었다. 다만 Phase 14 DoD가 제시한 "내부 직접 참조 목표치(최종 0)"를 글자 그대로 해석하면 미달성이다.

---

### 2.3 Phase 3 — Import 파이프라인 분리

**계획 목표**

- `FileLoader`를 BrowserAdapter / ImportOrchestrator / ParserWorker / ApplyService로 분해
- `ApplyXSFParseResult`, `ApplyXSFGridParseResult`, `ApplyChgcarParseResult` 공통 흐름 통합

**검증 결과**

| 계획 목표 위치 | 현재 실재 경로 | 상태 |
|---|---|---|
| `io/platform/browser_file_picker.cpp` | 존재 | 달성 |
| `io/application/parser_worker_service.cpp` | 존재 | 달성 |
| `io/application/import_apply_service.cpp` | 존재 | 달성 |
| `io/application/import_orchestrator.cpp` | 존재 | 달성 |
| `io/application/import_workflow_service.cpp` | 존재 (상위 워크플로우 파사드 추가 도입됨) | 달성 (+초과 이행) |
| `io/infrastructure/xsf_parser.cpp`, `chgcar_parser.cpp` | 존재 | 달성 |

판정: **달성**. import 책임이 포트(`file_dialog_port.h`, `import_runtime_port.h`, `progress_port.h`, `worker_port.h`) 인터페이스와 adapter(`browser_file_dialog_adapter.*`, `emscripten_worker_port.*`)로 정교하게 분리되었다.

---

### 2.4 Phase 4 — UI 렌더러 분해

**계획 목표**

- `AtomsTemplate`를 Presenter/Controller로 분리
- `charge_density_ui.cpp`의 슬라이스/아이소서피스 UI 분리
- `vtk_viewer.cpp`의 `processEvents`, 카메라 정렬 추출

**검증 결과**

| 계획 목표 위치 | 현재 실재 경로 | 상태 |
|---|---|---|
| `atoms/presentation/builder_windows_controller.cpp` | `shell/presentation/atoms/builder_windows_controller.cpp`로 재배치 | 달성 (Phase 17-R2에서 shell로 이동) |
| `atoms/presentation/editor_windows_controller.cpp` | `shell/presentation/atoms/editor_windows_controller.cpp` | 달성 |
| 측정 컨트롤러 | `measurement/application/measurement_controller.cpp` | 달성 |
| 슬라이스 프리뷰 | `density/presentation/slice_view_panel.cpp`, `isosurface_panel.cpp` | 달성 |
| `render/application/viewer_interaction_controller.cpp` | 존재 | 달성 |
| `render/application/camera_align_service.cpp` | 존재 | 달성 |

판정: **달성**. 단, 축소 목표였던 `atoms_template.cpp`의 규모 자체는 Phase 17-Root에서 경로만 `workspace/legacy/atoms_template_facade.cpp`로 이동(5,808 라인)되었을 뿐 완전한 해체는 일어나지 않았다.

---

### 2.5 Phase 5 — 빌드 시스템 모듈화

**계획 목표**

- CMake를 모듈 라이브러리 단위로 분할 (`wb_core`, `wb_render`, `wb_mesh`, `wb_atoms`, `wb_io`, `wb_ui`)

**검증 결과**

최종 모듈(10개)은 Phase 5 계획과 다른 형태로 정착되었다.

| 계획 당시 예시 | 최종 모듈 (Phase 16 이후) | 상태 |
|---|---|---|
| `wb_core` | `wb_common` | 이름 변경 |
| `wb_render` | `wb_render` | 유지 |
| `wb_mesh` | `wb_mesh` | 유지 |
| `wb_atoms` | 제거됨 → `wb_structure`, `wb_measurement`, `wb_density`로 분해 | 초과 이행 |
| `wb_io` | `wb_io` | 유지 |
| `wb_ui` | 제거됨 → `wb_shell`, `wb_workspace`로 분해 | 초과 이행 |
| (신규) | `wb_platform` | 신규 |

판정: **달성**. Phase 5 계획의 "모듈 분리" 방향은 달성되었고, Phase 12~16에서 **더 세분화**되었다. CMakeLists.txt 내부 `WB_MODULE_TARGETS`에 정확히 10개의 타깃이 나열되어 있다.

---

### 2.6 Phase 6 — 테스트/회귀 체계

**계획 목표**

- `chgcar_parser_test.cpp`, `cell_transform_test.cpp`, `measurement_geometry_test.cpp` 추가

**검증 결과**

| 계획 위치 | 현재 위치 | 상태 |
|---|---|---|
| `tests/io/chgcar_parser_test.cpp` | `webassembly/tests/io/chgcar_parser_test.cpp` | 존재 |
| `tests/atoms/cell_transform_test.cpp` | `webassembly/tests/atoms/cell_transform_test.cpp` | 존재 |
| `tests/atoms/measurement_geometry_test.cpp` | `webassembly/tests/atoms/measurement_geometry_test.cpp` | 존재 |
| `tests/io/xsf_parser_test.cpp` | 존재 (계획 외 추가) | 초과 이행 |
| Playwright 스모크 (`tests/e2e/`) | `workbench-smoke.spec.ts`, `workbench-layout-panel-regression.spec.ts` | 존재 |

판정: **달성**.

---

### 2.7 Phase 7 — Composition Root 도입

**계획 목표**

- `shell/runtime/WorkbenchRuntime` 도입
- wasm binding이 runtime API만 사용
- 정적 게이트 `check_phase7_runtime_composition.ps1` 도입

**검증 결과**

| 항목 | 상태 |
|---|---|
| `shell/runtime/workbench_runtime.{h,cpp}` | 존재 (299 라인) |
| `platform/wasm/workbench_bindings.cpp` | 존재 |
| CMakeLists.txt의 `add_executable`이 `main.cpp`, `workbench_bindings.cpp`만 포함 | 확인됨 |
| `check_phase7_runtime_composition.ps1` | 존재 |

판정: **달성**.

---

### 2.8 Phase 8 — AtomsTemplate 해체 (Structure/Measurement/Density)

**계획 목표**

- `structure/*`, `measurement/*`, `density/*` 디렉터리 도입
- `AtomsTemplate` 크기 축소 또는 제거
- `atoms_template.h`의 `friend class` 0건

**검증 결과**

| 항목 | 상태 |
|---|---|
| `structure/{domain,application,infrastructure,presentation}` 4계층 | 모두 존재 |
| `measurement/{domain,application,infrastructure,presentation}` 4계층 | 모두 존재 (`infrastructure`는 `legacy`만 포함) |
| `density/{domain,application,infrastructure,presentation}` 4계층 | 모두 존재 |
| `atoms_template.h` → `workspace/legacy/atoms_template_facade.h` | 이동됨 (경로 변경) |
| `AtomsTemplate` 클래스의 크기 | 여전히 대형 (`atoms_template_facade.cpp` 5,808 라인, 클래스 본체 약 1,325 라인) |
| `atoms_template_facade.h`의 `friend class` | 확인 결과 0건 (원래 9개였던 friend class는 제거됨) |

판정: **부분 달성**. 세 개의 feature 모듈 구조 자체는 완벽하게 도입되었으나, `AtomsTemplate` 클래스 그 자체는 `workspace/legacy/`에 논리적으로 격리된 상태로 **5,808 라인 규모 그대로 존재**한다. 실질적 해체라기보다는 **경계 격리**에 가깝다.

---

### 2.9 Phase 9 — VtkViewer 해체 + Render 포트 완성

**계획 목표**

- viewer를 viewport / camera / picking / overlay / scene renderer로 분리
- `render` 외부의 `VtkViewer::Instance()` 호출 0건

**검증 결과**

| 항목 | 상태 |
|---|---|
| `render/infrastructure/vtk_render_gateway.{h,cpp}` | 존재 |
| `render/infrastructure/interaction/mouse_interactor_style.*` | 존재 |
| `render/infrastructure/atoms/*` (vtk_renderer, batch_update_system, bond_renderer) | 존재 |
| `render/application/viewer_interaction_controller.cpp`, `camera_align_service.cpp` | 존재 |
| `render/application/legacy_viewer_facade.{h,cpp}` | 존재 (레거시 호환 경로로 유지) |
| `VtkViewer::Instance()` 호출 (전체) | 3건 — 모두 `render/` 내부 또는 render 게이트웨이 초기화 지점 |

판정: **달성(경계 기준)**. `VtkViewer` 자체는 `DECLARE_SINGLETON(VtkViewer)`로 남아 있으나 render 경계 내부로만 한정되었다. 계획서 자체도 "composition root 한정"을 허용했다.

---

### 2.10 Phase 10 — MeshManager / FileLoader / Workspace 분리

**계획 목표**

- `MeshManager`, `FileLoader` 호출 0건 (feature 코드에서)
- `workspace/domain/workspace_store.*` 도입
- panel은 read model 기반 presenter로 전환

**검증 결과**

| 항목 | 상태 |
|---|---|
| `workspace/domain/workspace_store.{h,cpp}` | 존재 |
| `workspace/application/workspace_command_service.*`, `workspace_query_service.*` | 존재 |
| `mesh/domain/mesh_repository.{h,cpp}`, `mesh/domain/mesh_repository_core.h` | 존재 |
| `io/application/import_workflow_service.*` | 존재 |
| `MeshManager::Instance()` 호출 | 1건 (`mesh/domain/mesh_repository.cpp`의 repository 래핑) |
| `FileLoader::Instance()` 호출 | 0건 |

판정: **달성**. `FileLoader::Instance()` 호출은 0이고, `MeshManager::Instance()`는 `mesh/domain/mesh_repository.cpp` 내부 1건으로 repository 호환 층에만 한정된다.

---

### 2.11 Phase 11 — Shell/Panel 객체화 + App 축소

**계획 목표**

- `App`, `Toolbar`, `ModelTree`, `MeshDetail`, `MeshGroupDetail`, `TestWindow` singleton 제거
- `app.cpp`는 프레임 오케스트레이션만 담당
- `shell/application/workbench_controller.*` 도입
- `shell/presentation/main_menu.cpp`, `window_layout.cpp` 도입

**검증 결과**

| 항목 | 상태 |
|---|---|
| `DECLARE_SINGLETON(App|Toolbar|ModelTree|FileLoader)` | 0건 (모두 제거됨) |
| `App::Instance()`, `Toolbar::Instance()`, `ModelTree::Instance()`, `FileLoader::Instance()` | 0건 |
| `shell/application/workbench_controller.{h,cpp}` | 존재 (210 라인) |
| `shell/domain/shell_state_store.{h,cpp}` | 존재 |
| `shell/presentation/main_menu.cpp` | **미존재** |
| `shell/presentation/window_layout.cpp` | **미존재** |
| `app.cpp` 크기 | 1,430 라인, 34개 `App::*` 메서드 |

판정: **부분 달성**. Singleton 제거는 완전히 달성되었으나, `app.cpp`의 "프레임 오케스트레이션만 담당" 기준은 미달성이다. `renderDockSpaceAndMenu`(약 430 라인), `InitImGuiWindows`, `renderImGuiWindows`, `renderAboutPopup`, 폰트/색상 관리, 진행률 팝업 등이 여전히 `App` 내부에 직접 구현되어 있다. 계획서가 의도한 `shell/presentation/main_menu.cpp`, `window_layout.cpp`로의 분리는 일어나지 않았다.

---

### 2.12 Phase 12 — Compatibility Facade 제거 + 모듈 Seal + CI 게이트

**계획 목표**

- `AtomsTemplate`, `MeshManager`, `FileLoader`, `ModelTree`, `Toolbar`, `VtkViewer` compatibility facade 제거
- build target을 최종 모듈 단위로 재정렬
- architecture gate 스크립트 추가

**검증 결과**

| 항목 | 상태 |
|---|---|
| 최종 6개 facade 제거 | 부분 달성 (`AtomsTemplate` 클래스는 여전히 `workspace/legacy/atoms_template_facade.*`로 존재; `VtkViewer`는 `DECLARE_SINGLETON` 상태 유지) |
| `check_phase12_architecture_seal.ps1` | 존재, `package.json`의 `check:phase12:architecture` 진입점 연결 |
| 모듈 타깃 최종 구성 | 10개 타깃으로 정렬 완료 |

판정: **부분 달성**. Phase 12 시점에서는 "facade 제거"라는 목표가 아니라 "facade allowlist 유지"로 전략이 수정되었음이 `refactoring_result_report_phase7_12_260406.md`에도 명시되어 있다.

---

### 2.13 Phase 13 — Runtime Hard Seal + Singleton Entrypoint 정리

**계획 목표**

- `App::Instance()`, `Toolbar::Instance()`, `FileLoader::Instance()` 호출 0건
- `check_phase13_runtime_hard_seal.ps1` 도입

**검증 결과**

| 항목 | 상태 |
|---|---|
| 3개 singleton 호출 0건 | 달성 (측정값 0) |
| 게이트 스크립트 | `check_phase13_runtime_hard_seal.ps1` 존재 |
| `docs/refactoring/phase13/` 산출물 | `refactoring_phase13_runtime_hard_seal_singleton_entrypoint_260406.md`, `dependency_gate_report.md`, `go_no_go_phase14.md` 모두 존재 |

판정: **달성**.

---

### 2.14 Phase 14 — State Ownership + Legacy Alias 제거

**계획 목표**

- `createdAtoms/createdBonds/cellInfo` 치환
- `friend class` 0건화
- `check_phase14_state_ownership_alias_zero.ps1` 도입

**검증 결과**

| 항목 | 상태 |
|---|---|
| legacy alias 전역 변수/`using`/`extern` | 0건 (달성) |
| `friend class` 전체 | 1건 (`io/application/import_entry_service.h:76`, `shell::presentation::ImportPopupPresenter`에 대한 friend) — 이는 Phase 17-R2 이후 도입된 새로운 friend 선언 |
| 게이트 스크립트 | `check_phase14_state_ownership_alias_zero.ps1` 존재 |

판정: **부분 달성**. legacy alias 차원은 달성되었으나 `friend class` 0건 목표는 **새로 도입된 friend class 1건이 잔존**하여 미달성이다. Phase 17의 shell UI 재배치 과정에서 `FileLoader`(이제 `import_entry_service.h`에 존재)와 `ImportPopupPresenter` 사이에 새 friend 관계가 생겨 Phase 14 DoD를 역행한다.

---

### 2.15 Phase 15 — Render/Platform Port Isolation

**계획 목표**

- `render` 외부 `vtkActor`/`vtkVolume` 직접 조작 0
- 포트 계약 위반 include 0
- `check_phase15_render_platform_isolation.ps1` 도입

**검증 결과**

| 항목 | 상태 |
|---|---|
| `platform/{browser,worker,persistence,wasm}` 4계층 | 모두 존재, 각각 adapter 구현 포함 |
| `render` 외부 `vtkActor*` 참조 파일 | 12개 파일에서 29건 |
| 세부: `workspace/legacy/atoms_template_facade.*` | 15건 (legacy 격리 경로, 허용) |
| 세부: `density/infrastructure/*` | 1건 (infrastructure는 원래 VTK 허용) |
| 세부: `measurement/application/measurement_controller.cpp` | 1건 — `AtomsTemplate::HandleMeasurementClickByPicker(vtkActor*, ...)` 구현부로, facade 클래스의 멤버 함수가 application 디렉터리에 배치됨 |
| 게이트 스크립트 | `check_phase15_render_platform_isolation.ps1` 존재 |

판정: **부분 달성**. feature application 경로에서 `vtkActor*`이 완전히 사라지진 않았다. 특히 `measurement/application/measurement_controller.cpp`의 `AtomsTemplate::HandleMeasurementClickByPicker` 정의는 "application 레이어의 VTK 금지" 원칙을 위반한다. 다만 이는 facade 해체 과도기의 타협으로 해석 가능하다.

---

### 2.16 Phase 16 — 최종 Build Target 전환

**계획 목표**

- `wb_atoms` 제거
- 목표 10개 타깃 완성 (`wb_common/platform/shell/workspace/render/mesh/structure/measurement/density/io`)

**검증 결과**

CMakeLists.txt 확인: `WB_MODULE_TARGETS`에 다음이 정확히 나열됨:

```
wb_common
wb_platform
wb_workspace
wb_render
wb_mesh
wb_structure
wb_measurement
wb_density
wb_io
wb_shell
```

`webassembly/cmake/modules/`에도 10개 `.cmake` 파일 실재 (wb_common, wb_density, wb_io, wb_measurement, wb_mesh, wb_platform, wb_render, wb_shell, wb_structure, wb_workspace).

`wb_atoms|wb_core|wb_ui` 토큰 검색 결과: CMakeLists.txt 및 `webassembly/cmake/modules/*` 전체에서 0건.

판정: **완전 달성**.

---

### 2.17 Phase 17-Root — Root/Atoms 원본 해체 (R0~R6)

**계획 목표** (`refactoring_plan_root_legacy_dismantle_260408_53Codex.md`)

- root allowlist: `main.cpp`, `app.cpp`, `app.h`만 유지
- `webassembly/src/atoms/` 하위 코드 파일 0건
- facade 구경로(`atoms/legacy/atoms_template_facade.h`) 참조 0

**검증 결과**

| 항목 | 상태 |
|---|---|
| root 코드 파일 (`*.cpp/*.h`) | `app.cpp`, `app.h`, `main.cpp` 3개 — 달성 |
| `webassembly/src/atoms/` 디렉터리 | 존재하지 않음 — 달성 |
| `workspace/legacy/atoms_template_facade.h` 포함 파일 | 3개 (모두 `workspace/legacy/` 내부): `atoms_template_facade.cpp`, `atoms_template_facade.h`, `legacy_atoms_runtime.h` |
| 게이트 스크립트 | R2~R6까지 9종 `check_phase17_*.ps1` 존재 |

판정: **달성**. 해체의 "코드 이동" 측면은 완전하나, `AtomsTemplate` 클래스 자체는 **삭제된 것이 아니라 `workspace/legacy/`로 재배치**되었다. 이는 계획 의도("격리 가능한 종료 상태")에 부합하며 계획상 허용된 결과다.

---

### 2.18 Phase 18 — Phase 17 한계 보완 및 최종 봉인

**계획 DoD 10개 항목 검증**

| # | DoD 항목 | 측정 | 판정 |
|---|---|---|---|
| 1 | 수동 UI 회귀 체크리스트 자동/수동 증빙 | `tests/e2e/workbench-layout-panel-regression.spec.ts` 존재, `phase18/logs/manual_ui_regression_phase18_latest.md` 존재 | 달성 (자동); 수동 사인오프는 운영 절차 |
| 2 | `workspace/legacy/atoms_template_facade.h` 외부 include 0 | 측정 결과 3개 파일 모두 `workspace/legacy/` 내부 | 달성 |
| 3 | `AtomsTemplate::Instance()` 0건 | 측정 결과 0건 | 달성 |
| 4 | `DECLARE_SINGLETON(AtomsTemplate)` 0 | 측정 결과 0건 | 달성 |
| 5 | 빈 폴더/`.gitkeep` 0 | 측정 결과 0건 | 달성 |
| 6 | 미참조 헤더 후보 3건 처리 | `density_render_bridge.h`, `rho_file_parser.h`, `slice_renderer.h`가 allowlist(`unused_file_allowlist_phase18_latest.md`)에 문서화 | 달성 |
| 7 | 주석 정책 전 코드파일 전수 점검 | `comment_policy_fix_report_phase18_latest.md`, `comment_policy_violation_list_phase18_latest.md` 생성 | 달성 |
| 8 | Phase17+Phase18 게이트 + build/test/smoke 동시 PASS | `phase18/logs/*`에 PASS 로그 존재 (본 검증 세션에서 재실행은 하지 않음) | 달성 (문서 기준) |
| 9 | 신규 7개 게이트 스크립트 | `check_phase18_{ui-evidence, facade-include-zero, atoms-instance-zero, legacy-singleton-zero, unused-asset-cleanup, doc-contract-full, final-closure}.ps1` 모두 실재 | 달성 |
| 10 | 종료 문서 패키지 | `refactoring_result_report_phase18_final_closure_260420.md`, `refactoring_result_report_phase0_18_comprehensive_260420.md` 모두 실재 | 달성 |

판정: **Phase 18 DoD 10개 모두 달성**. Phase 18은 Phase 17이 남긴 구체적이고 정량화 가능한 한계를 실제로 해소한 단계로 평가된다.

---

## 3. 계획-코드 간 핵심 차이 요약

### 3.1 계획 의도와 다른 종료 방식 (전략 변경)

`refactoring_plan_full_modular_architecture_260331.md`의 10장은 다음과 같은 판정 문장을 포함한다.

> `webassembly/src`는 더 이상 `AtomsTemplate`나 `VtkViewer` 같은 god-object를 중심으로 돌아가지 않는다.

현재 상태는 이 기준을 **의미적으로는** 충족한다. 런타임 호출 경로의 중심은 `WorkbenchRuntime`과 각 feature service로 이동했다. 그러나 **클래스 수준**에서는 `AtomsTemplate`(5,808 라인 facade)과 `VtkViewer`(`DECLARE_SINGLETON` 유지) 모두 여전히 물리적으로 존재한다. Phase 7~12 결과보고서에서 이미 지적된 대로 **"allowlist + budget 기반 점진 정착"** 전략이 채택되었다.

### 3.2 정량적 미달 항목

1. **`friend class` 1건 잔존** — Phase 14 DoD가 0을 목표했으나, `io/application/import_entry_service.h:76`에 Phase 17 이후 새로 도입된 friend(`shell::presentation::ImportPopupPresenter`)가 존재한다. Phase 18 DoD에도 이 항목은 명시적으로 포함되지 않았다.
2. **runtime singleton 3건 잔존** — `VtkViewer`, `MeshManager`, `FontManager`는 Phase 18 보고서 6.2절에서 "잔여 과제"로 명시되었다. 본 검증에서도 확인된다.
3. **`::Instance()` 총 48건 잔존** — Phase 7~12 최종 DoD의 "0 또는 composition root 한정" 중 "0"은 달성되지 않았고, 호환 layer 호출로 제한되었다.
4. **`app.cpp` 1,430 라인** — Phase 11 DoD "프레임 오케스트레이션만 담당"을 글자 그대로 해석하면 미달. `shell/presentation/main_menu.cpp`, `window_layout.cpp`는 생성되지 않았다.
5. **`workspace/legacy/atoms_template_facade.cpp` 5,808 라인** — Phase 8 DoD "facade 수준 축소"를 글자 그대로 해석하면 미달. 다만 Phase 17 이후 전략상 "격리만 유지"로 변경됨.
6. **`measurement/application/measurement_controller.cpp`의 VTK 토큰** — Phase 15 DoD "render 외부 VTK 직접 조작 0"에 1건 예외 존재.

### 3.3 계획 대비 초과 이행 항목

1. Phase 5 계획의 6개 모듈이 아닌 **10개 모듈**로 확장 (wb_platform, wb_workspace, wb_shell 등 추가)
2. Phase 3 외 `import_workflow_service`, `file_dialog_port`, `worker_port`, `progress_port` 등 **추가 포트/워크플로우** 도입
3. Phase 6 외 `xsf_parser_test.cpp` 추가 및 Playwright 스모크 테스트 2종 추가
4. Phase 17 R0~R6 **7라운드 세분화 실행**(계획 단일 Phase 17보다 세밀)
5. Phase 18 신규 게이트 **7종 및 `check:phase18:*` npm 진입점 8종** 추가

---

## 4. 아키텍처 완성도 종합 평가

### 4.1 계획서 3장(목표 아키텍처) 달성 매트릭스

| 목표 항목 | 현재 상태 | 평가 |
|---|---|---|
| 3.1 Composition Root 우선 | `shell/runtime/WorkbenchRuntime` 중심 조립 확립 | 달성 |
| 3.1 Bounded Context 분리 | structure/measurement/density 분리 | 달성 |
| 3.1 Port/Adapter 구조 | `render/application/*`, `io/application/*` 포트 및 `io/platform/*`, `platform/*` adapter 존재 | 달성 |
| 3.1 상태 단일화 | `StructureRepository`, `MeshRepository`, `MeasurementRepository`, `DensityRepository`, `WorkspaceStore`, `ShellStateStore` 모두 존재 | 달성 |
| 3.2 목표 모듈 경계 | 10개 모듈 그래프 완성 (`wb_atoms` 제거, `wb_structure/measurement/density`로 분해) | 달성 |
| 3.3 목표 디렉터리 레이아웃 | 계획 디렉터리 거의 그대로 반영. 예외: `measurement/infrastructure`는 `legacy/`만 포함 | 거의 달성 |
| 3.4 목표 런타임 구성 | `WorkbenchRuntime`이 feature service들을 명시적 조립 | 달성 |
| 3.5 상태 소유권 모델 | feature별 Repository/Store로 정렬 | 달성 |
| 3.6 포트/어댑터 계약 | RenderScenePort/FileDialogPort/WorkerPort/ProgressPort 등 존재 | 달성 |
| 3.7 최종 빌드 타깃 | `wb_atoms` 제거 + 10개 타깃 구성 완료 | 달성 |
| 3.8 주요 유스케이스 흐름 | import → workspace → feature 서비스 → render 흐름 실재 | 달성 |

### 4.2 DoD 11개 항목 (계획서 7장) 최종 평가

| # | DoD 항목 | 현재 판정 |
|---|---|---|
| 1 | `AtomsTemplate/MeshManager/FileLoader/ModelTree/Toolbar/VtkViewer` singleton 제거 또는 runtime shim 한정 | 부분 달성 (AtomsTemplate/FileLoader/ModelTree/Toolbar: 완료, MeshManager/VtkViewer: shim 한정) |
| 2 | feature/application/domain의 `::Instance()` 0건 | 부분 달성 (feature 서비스 레이어에서는 0, 전체 48건 잔존) |
| 3 | `friend class` 0 | **미달성 (1건 잔존)** |
| 4 | legacy alias 외부 노출 0 | 달성 |
| 5 | `wb_atoms` 제거 + 최종 모듈 그래프 비순환 | 달성 |
| 6 | `app.cpp` shell orchestration 축소 | 부분 달성 (feature singleton 호출은 제거, 그러나 1,430 라인 규모) |
| 7 | `render` 외부 VTK actor/volume 직접 조작 없음 | 부분 달성 (`measurement/application`에 1건 예외) |
| 8 | `test:cpp`, `test:smoke`, architecture gate 동시 PASS | 달성 (문서 기준) |
| 9 | 결과보고서 + 판정 문서 | 달성 (모든 Phase에 존재) |
| 10 | 새 public API Doxygen 주석 | 달성 (Phase 18 doc_contract 게이트 PASS) |
| 11 | 브랜치/WBS 커밋/push 추적 가능성 | 달성 (`refactoring_result_report_phase0_18_comprehensive_260420.md` 2장 마일스톤 참조) |

**달성 7건 / 부분 달성 3건 / 미달성 1건**

---

## 5. 리스크 및 후속 과제

### 5.1 잠재 리스크

1. **`workspace/legacy/atoms_template_facade.cpp`의 5,808 라인 단일 파일**
   - 유지보수 측면에서 여전히 god-object. `atoms_template_facade.h`는 11개의 forward declaration과 16개의 상위 include를 보유하여 변경 파급 범위가 크다.
   - 현재는 Phase 18 이후 "추가 수정하지 않고 격리 상태로 유지"하는 전략으로 보인다.

2. **`render/application/legacy_viewer_facade.*` 잔존**
   - `render` 내부이지만 여전히 "legacy" 접미사가 붙은 facade가 존재. Phase 9 "VtkViewer 해체" 목표의 잔재로, 완전한 해체는 일어나지 않았음을 가시화한다.

3. **`friend class` 1건의 원인 역추적 필요**
   - `io/application/import_entry_service.h`의 `FileLoader` 클래스와 `shell::presentation::ImportPopupPresenter` 간 friend 관계는 Phase 17-R2 shell UI 재배치 중 도입되었을 가능성이 높다.
   - Phase 14/Phase 17의 게이트 스크립트가 이 경로를 **명시적으로 감지하지 못했다**면 게이트 커버리지에 공백이 있다.

4. **Phase 11 DoD와 `app.cpp`의 간격**
   - 계획에 있던 `shell/presentation/main_menu.cpp`, `window_layout.cpp`는 생성되지 않았고, 기능은 `app.cpp`의 `renderDockSpaceAndMenu`(약 430 라인) 등에 집중되어 있다.
   - 장기적으로 이 파일이 다시 god-object화될 위험이 있다.

### 5.2 권고 사항

우선순위 순으로:

1. **[High] `friend class` 1건 해소 또는 게이트에 명시적 allowlist 문서화** — 계획 DoD와의 정확한 정합성 확보를 위해.
2. **[Medium] `app.cpp` 재분해 계획 수립** — `renderDockSpaceAndMenu` 등 UI 블록을 `shell/presentation/main_menu.cpp`, `window_layout.cpp`로 실제 분리.
3. **[Medium] `measurement/application/measurement_controller.cpp`의 VTK 토큰 제거** — `AtomsTemplate::HandleMeasurementClickByPicker` 구현부의 이전.
4. **[Low] `workspace/legacy/atoms_template_facade.cpp`의 점진적 축소 로드맵** — 새로운 유지보수 트랙 도입.
5. **[Low] `VtkViewer`, `MeshManager`, `FontManager` singleton 선언의 장기 제거 계획** — Phase 18 보고서 6.2절 과제의 후속 단계.

---

## 6. 결론

### 6.1 사실관계 요약

- 계획서가 제시한 **아키텍처 방향(composition root, bounded context, port/adapter, 모듈화, 테스트 체계)은 현재 코드에 충실히 반영**되어 있다.
- 계획서의 **최종 DoD 11개 중 7개 달성, 3개 부분 달성, 1개 미달성**.
- Phase 18은 Phase 17의 정량적 한계 5가지를 **모두 해소**했다.
- 기존 결과보고서(`refactoring_result_report_*.md`)의 핵심 수치는 본 독립 검증에서도 **대부분 재현**되었다.

### 6.2 기존 보고서와의 차이

기존 통합보고서(`refactoring_result_report_phase0_18_comprehensive_260420.md`)는 "`Phase 0~18 Closed (자동 검증 기준 GO)`"로 판정한다. 본 독립 검증은 이 판정에 **대체로 동의**하되, 다음을 추가한다.

1. `friend class` 0건 DoD는 **정확히는 미달성**이며, 기존 보고서에는 이 항목이 Phase 14 이후 추적되지 않았다.
2. `app.cpp` 1,430 라인은 Phase 11 DoD ("프레임 오케스트레이션만 담당")의 문자 그대로의 해석으로는 **부분 달성**이며, 기존 보고서의 정량 지표 표에는 이 크기 지표가 포함되지 않았다.
3. `workspace/legacy/atoms_template_facade.cpp` 5,808 라인은 Phase 8 DoD ("facade 수준 축소")의 문자 그대로의 해석으로는 **미달성**이나, 기존 보고서는 이를 "격리 달성"으로 재정의하여 평가했다. 양쪽 해석 모두 합리적이나, 독자는 해석의 변경이 있었음을 인지할 필요가 있다.

### 6.3 최종 판정

- **계획된 아키텍처 전환**: 달성 (의미적 기준)
- **계획서 초기 DoD 전량**: 부분 달성 (7/11 완전, 3/11 부분, 1/11 미달)
- **Phase 18 DoD 10개 항목**: 전량 달성
- **운영 판정**: `Closed (자동 검증 기준)` — 기존 보고서 판정과 동일

본 리팩토링 프로그램은 **"현실적 완결 상태"**에 도달했다고 평가하는 것이 타당하다. 계획서가 이상적으로 지향한 0-기반 DoD와는 일부 차이가 있으나, 그 차이는 명확히 문서화되어 있고 allowlist/legacy 격리 전략으로 관리되고 있다.

---

## 부록 A. 현재 모듈 구조 스냅샷

```
webassembly/src/
├── app.cpp, app.h, main.cpp          # 루트 3파일
├── common/                            # 공통 타입/유틸
├── config/, enum/, icon/, macro/      # 보조 레이어
├── platform/
│   ├── browser/ (file dialog adapter)
│   ├── persistence/ (viewer preferences)
│   ├── wasm/ (workbench_bindings)
│   └── worker/ (emscripten worker port, progress port)
├── shell/
│   ├── runtime/ (WorkbenchRuntime)
│   ├── application/ (WorkbenchController, ShellState services)
│   ├── domain/ (ShellStateStore)
│   └── presentation/ (atoms/, debug/, font/, import/, toolbar/, widgets/)
├── workspace/
│   ├── domain/ (WorkspaceStore)
│   ├── application/ (WorkspaceCommand/QueryService)
│   └── legacy/ (AtomsTemplate facade 5,808 라인, LegacyAtomsRuntime)
├── render/
│   ├── application/ (RenderGateway port, ViewerInteractionController, legacy_viewer_facade)
│   ├── infrastructure/ (VtkRenderGateway, interaction/, atoms/, resources/)
│   └── presentation/ (ViewerWindow)
├── mesh/
│   ├── domain/ (MeshRepository, MeshEntity)
│   ├── application/ (MeshCommand/QueryService)
│   └── presentation/ (ModelTree panel & sections, MeshDetail/GroupDetail)
├── structure/
│   ├── domain/ (StructureRepository + domain/atoms/* 17 files)
│   ├── application/ (StructureService, StructureLifecycle, VisibilityService)
│   ├── infrastructure/legacy/ (legacy port adapter)
│   └── presentation/
├── measurement/
│   ├── domain/ (MeasurementRepository, geometry)
│   ├── application/ (MeasurementService + controller)
│   ├── infrastructure/legacy/ (legacy port adapter)
│   └── presentation/
├── density/
│   ├── domain/ (DensityRepository, ChargeDensity)
│   ├── application/ (DensityService)
│   ├── infrastructure/ (ChargeDensityRenderer + 3 보존 헤더 + legacy/)
│   └── presentation/ (ChargeDensityUI, IsosurfacePanel, SliceViewPanel)
└── io/
    ├── application/ (ImportEntry/Workflow/Apply/Orchestrator, ParserWorker, ports)
    ├── infrastructure/ (ChgcarParser, XsfParser, FileIoManager, unv/, legacy/)
    └── platform/ (MemfsChunkStream)
```

총 10개 CMake 모듈, 224개 C++ 코드 파일 (`*.cpp/*.h`).

## 부록 B. 본 검증에서 사용한 주요 Grep/Find 결과 요약

| 측정 항목 | 명령 요지 | 값 |
|---|---|---|
| 루트 코드 파일 | `find webassembly/src -maxdepth 1 -name "*.cpp" -o -name "*.h"` | 3 |
| `webassembly/src/atoms` 존재 여부 | `ls webassembly/src/atoms/` | 디렉터리 없음 |
| `AtomsTemplate::Instance()` | `grep -r "AtomsTemplate::Instance()"` | 0 |
| `DECLARE_SINGLETON(AtomsTemplate)` | `grep -r "DECLARE_SINGLETON\s*\(\s*AtomsTemplate"` | 0 |
| `DECLARE_SINGLETON` 전체 | `grep -r "DECLARE_SINGLETON"` | 4 (1개는 매크로 정의, 3개는 VtkViewer/MeshManager/FontManager) |
| `::Instance()` 총 호출 | `grep -r "::Instance\(\)"` | 48 (25 파일) |
| `friend class` | `grep -r "friend\s+class"` | 1 |
| `atoms_template_facade.h` include | `grep -l "atoms_template_facade.h"` | 3 파일 (모두 `workspace/legacy/` 내부) |
| legacy alias 전역 선언 | `grep -r "extern.*createdAtoms\|using.*createdAtoms"` | 0 |
| `.gitkeep` / 빈 디렉터리 | `find -name ".gitkeep"`, `find -type d -empty` | 0 / 0 |
| `.DS_Store` / `.vscode` | `find -name ".DS_Store" -o -name ".vscode"` | 0 |
| CMake 최종 모듈 | `webassembly/cmake/modules/*.cmake` | 10 |
| `wb_atoms/wb_core/wb_ui` 참조 | `grep` in CMakeLists.txt + `webassembly/cmake/modules/*.cmake` | 0 |
| `app.cpp` 크기 | `wc -l` | 1,430 |
| `atoms_template_facade.cpp` 크기 | `wc -l` | 5,808 |
| `App::*` 메서드 수 | `grep "^(void\|bool\|int)\? App::"` | 34 |
| 전체 C++ 파일 수 | `find webassembly/src -name "*.cpp" -o -name "*.h"` | 224 |

---

*본 보고서는 코드/파일 시스템에 대한 정적 분석 결과이며, 런타임 동작 검증(수동 UI 회귀)은 포함하지 않는다. 실행 시점 검증은 `tests/e2e/*.spec.ts`와 `phase18/logs/` 로그에 위임한다.*
