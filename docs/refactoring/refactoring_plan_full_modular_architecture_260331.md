# webassembly/src 완전한 모듈 아키텍처 전환 계획서 (2026-03-31)

## 1. 목적과 범위
- 목적: Phase 1~6에서 확보한 경계 정리, 상태 저장소, 빌드 모듈화, 테스트 체계를 기반으로 남아 있는 singleton/god-object/legacy alias를 제거하고 `완전한 모듈 아키텍처`로 전환한다.
- 범위: `webassembly/src` 전체 C++ 코드, `webassembly/tests`, `CMakeLists.txt`, `webassembly/cmake/modules`, 필요 시 `/app/workbench/page.tsx` 및 wasm binding seam
- 선행 상태:
  - `docs/refactoring/refactoring_result_report_phase1_6_260331.md` 작성 완료
  - Phase 1~6 종료 판정 문서 및 게이트 로그 확보
- 산출물:
  - 목표 아키텍처 설계안
  - Phase 7 이후 실행 로드맵
  - 모듈/계층/상태 소유권 규칙
  - 완료 기준(DoD) 및 정적 게이트 규칙

## 2. 현재 구조 진단

### 2.1 모듈 구성(현행)
- 엔트리/플랫폼:
  - `main.cpp`, `bind_function.cpp`
  - 브라우저 연동: `io/platform/browser_file_picker.cpp`
- 앱 셸/UI 배치:
  - `app.cpp/h`, `toolbar.cpp/h`, `model_tree.cpp/h`, `mesh_detail.cpp/h`, `mesh_group_detail.cpp/h`, `test_window.cpp/h`
- 렌더:
  - `vtk_viewer.cpp/h`, `mouse_interactor_style.cpp/h`
  - `render/application/*`, `render/infrastructure/*`
- 메쉬:
  - `mesh.cpp/h`, `mesh_manager.cpp/h`, `mesh_group*.cpp/h`
  - `mesh/presentation/*`
- 원자/구조:
  - `atoms/atoms_template.cpp/h`
  - `atoms/application/*`, `atoms/domain/*`, `atoms/infrastructure/*`, `atoms/presentation/*`, `atoms/ui/*`
- IO:
  - `file_loader.cpp/h`
  - `io/application/*`, `io/infrastructure/*`, `io/platform/*`

### 2.2 정량 지표(현재 코드 기준)
- 대형 파일
  - `webassembly/src/atoms/atoms_template.cpp`: `5161` lines
  - `webassembly/src/vtk_viewer.cpp`: `1995` lines
  - `webassembly/src/app.cpp`: `1068` lines
  - `webassembly/src/toolbar.cpp`: `472` lines
  - `webassembly/src/file_loader.cpp`: `443` lines
  - `webassembly/src/mesh.cpp`: `880` lines
- singleton 접근 빈도(`::Instance()`)
  - `webassembly/src/atoms/atoms_template.cpp`: `103`
  - `webassembly/src/app.cpp`: `38`
  - `webassembly/src/mesh.cpp`: `30`
  - `webassembly/src/file_loader.cpp`: `17`
  - `webassembly/src/vtk_viewer.cpp`: `11`
  - `webassembly/src/mesh_manager.cpp`: `4`
- 캡슐화 약화 지표
  - `webassembly/src/atoms/atoms_template.h`의 `friend class`: `9`
- 이미 달성된 경계 지표
  - `model_tree.cpp`의 atoms 전역 direct using: `0`
  - `atoms/domain + atoms/infrastructure`의 `VtkViewer::Instance()` 직접 호출: `0`
  - `atoms/domain`의 `extern` 선언: `0`
- 빌드/테스트 기준선
  - 루트 `CMakeLists.txt`: `147` lines
  - Phase 5 모듈 `.cmake`: `6`
  - Phase 6 C++ 테스트 파일: `4`
  - Phase 6 fixture 파일: `3`

### 2.3 구조적 문제
1. Composition root 부재
- `app.cpp`가 `AtomsTemplate`, `VtkViewer`, `FileLoader`, `ModelTree`, `MeshDetail`, `MeshGroupDetail`, `FontManager` 등의 singleton을 직접 조합한다.
- 결과: 런타임 객체 그래프가 파일 전역 singleton 호출에 숨겨져 있고 교체/테스트/추적이 어렵다.

2. `AtomsTemplate`의 god-object 상태 지속
- 구조 편집, 원자/결합 가시성, 측정, charge-density, BZ plot, overlay actor 제어, 일부 패널 UI까지 한 클래스에 집중돼 있다.
- Phase 4에서 파일 분해는 진행됐지만, 객체 경계와 상태 소유권은 완전히 나뉘지 않았다.

3. `VtkViewer`의 기능 집중
- viewport, input, camera, picking, overlay, toolbar 연계, feature callback이 한 viewer 객체에 누적되어 있다.
- `render_gateway`가 도입되었지만 상위 계층의 viewer 직접 접근은 아직 많다.

4. `MeshManager` / `FileLoader` / `ModelTree` / `Toolbar`의 singleton 중심 설계
- feature 모듈 간 협력이 repository/query/service 대신 singleton lookup으로 이뤄진다.
- 결과: build 모듈은 나뉘었지만 runtime 의존은 여전히 강결합이다.

5. legacy alias 및 임시 브리지 잔존
- `createdAtoms`, `createdBonds`, `cellInfo` 등은 `StructureStateStore` 기반으로 바뀌었지만 이름은 유지된다.
- `StructureReadModel`도 현재는 legacy alias를 통해 데이터를 읽는다.

6. 런타임/빌드 모듈 불일치
- CMake 모듈화는 완료되었지만 `wb_ui`, `wb_render`, `wb_mesh`, `wb_atoms`, `wb_io` 사이에는 순환 성격의 상호 의존이 남아 있다.
- 최종 아키텍처 관점에서는 `wb_atoms`가 여전히 너무 넓다.

7. 아키텍처 게이트 부족
- 테스트 게이트는 존재하지만, 아직 `::Instance()`, `friend class`, cross-layer include, legacy alias 노출을 막는 최종 구조 게이트는 없다.

## 3. 목표 아키텍처(구체 설계)

### 3.1 핵심 원칙
- Composition root 우선:
  - 객체 생성/연결은 오직 `shell/runtime`에서 수행한다.
  - feature/application/domain/presentation에서는 singleton lookup을 금지한다.
- Bounded context 분리:
  - 현재 `atoms`는 `structure`, `measurement`, `density`로 분해한다.
  - cross-feature 조정은 `workspace`와 `shell`에서 담당한다.
- Port/Adapter 구조:
  - feature application은 `RenderPort`, `DialogPort`, `ProgressPort`, `WorkerPort` 같은 포트에만 의존한다.
  - VTK/Emscripten/Browser는 infrastructure adapter로 격리한다.
- 상태 단일화:
  - 동일 의미의 상태는 하나의 저장소만 소유한다.
  - 화면용 read model은 저장소에서 파생하고, 자체 복제 상태를 갖지 않는다.
- Build seal:
  - 최종 단계에서는 각 모듈 CMake target이 실제 런타임 경계와 일치해야 한다.
  - `wb_atoms` 같은 과대 모듈은 해체한다.

### 3.2 목표 모듈 경계

| 모듈 | 책임 | 소유 상태 | 허용 의존 |
|---|---|---|---|
| `common` | 공통 타입, 결과 모델, utility, port 인터페이스 | 없음 | 없음 |
| `platform` | Emscripten binding, browser file dialog, worker bridge, persistence adapter | 브라우저/OS 자원 핸들 | `common` |
| `shell` | 앱 메뉴, 창 배치, command dispatch, composition root | shell UI 상태 | `common`, `workspace`, 각 feature application |
| `workspace` | 현재 세션의 선택/활성 객체/scene composition/read model | active ids, selection, panel context | `common`, `mesh`, `structure`, `measurement`, `density` |
| `render` | viewport, camera, picking, actor/overlay adapter, render queue | render scene internals | `common` |
| `mesh` | mesh entity, repository, tree, visibility, mesh read models | mesh catalog/tree | `common` |
| `structure` | atom/bond/cell/structure lifecycle 및 편집 use-case | per-structure atomic state | `common`, `render` 포트 |
| `measurement` | 거리/각도/이면각/중심 측정과 overlay descriptor 생성 | measurements | `common`, `render` 포트, `structure` query |
| `density` | CHGCAR/XSF grid, isosurface/slice state와 use-case | density state | `common`, `render` 포트, `mesh` query, `structure` query |
| `io` | import workflow, parser orchestration, transaction, scene apply | import jobs / parsed result | `common`, `mesh`, `structure`, `density`, `workspace` |

목표:
- `atoms/` 디렉터리는 최종적으로 해체하고 `structure/`, `measurement/`, `density/`로 대체한다.
- `app.cpp`는 `shell`로 축소한다.
- `VtkViewer`, `AtomsTemplate`, `MeshManager`, `FileLoader`, `ModelTree`, `Toolbar` singleton은 최종적으로 제거한다.

### 3.3 목표 디렉터리 레이아웃

```text
webassembly/src/
  common/
  platform/
    browser/
    worker/
    wasm/
    persistence/
  shell/
    runtime/
    application/
    presentation/
  workspace/
    domain/
    application/
    presentation/
  render/
    application/
    infrastructure/
    presentation/
  mesh/
    domain/
    application/
    presentation/
  structure/
    domain/
    application/
    infrastructure/
    presentation/
  measurement/
    domain/
    application/
    presentation/
  density/
    domain/
    application/
    infrastructure/
    presentation/
  io/
    application/
    infrastructure/
    platform/
```

이관 규칙:
- `atoms/domain/*` 중 순수 구조 상태는 `structure/domain/`
- 측정 계산/측정 상태는 `measurement/domain/`
- charge-density 및 grid/slice/isosurface는 `density/*`
- `atoms/ui/*`는 feature presentation으로 분산
- `bind_function.cpp`는 `platform/wasm/workbench_bindings.cpp`로 이동
- `app.cpp`의 메뉴/레이아웃 코드는 `shell/presentation/`

### 3.4 목표 런타임 구성(Composition Root)

최종 런타임은 `shell/runtime/WorkbenchRuntime`에서만 조립한다.

```text
WorkbenchRuntime
  ├─ ShellController
  ├─ WorkspaceStore
  ├─ MeshRepository / MeshQueryService
  ├─ StructureRepository / StructureCommandService
  ├─ MeasurementRepository / MeasurementService
  ├─ DensityRepository / DensityService
  ├─ ImportWorkflowService
  ├─ RenderPorts (Vtk adapters)
  ├─ PlatformPorts (dialog, worker, persistence)
  └─ Presentation objects (panels, toolbars, tree, detail views)
```

규칙:
- `main.cpp`는 `WorkbenchRuntime` 생성과 실행만 담당한다.
- wasm binding은 runtime public API만 호출한다.
- 각 panel/presenter는 필요한 service/query만 생성자 인자로 받는다.
- `::Instance()`는 최종적으로 composition root 밖에서 0건이어야 한다.

### 3.5 상태 소유권 모델

| 상태 종류 | 최종 소유자 | 비고 |
|---|---|---|
| 구조(원자/결합/셀) 데이터 | `structure::domain::StructureRepository` | `StructureStateStore`는 여기로 흡수 |
| mesh tree / mesh visibility | `mesh::domain::MeshRepository` | `MeshManager` 대체 |
| 선택/현재 구조/현재 mesh/활성 tool | `workspace::domain::WorkspaceStore` | cross-feature 문맥 상태 |
| 측정 데이터 | `measurement::domain::MeasurementRepository` | overlay descriptor는 파생 데이터 |
| density 설정/활성 grid/isosurface/slice 상태 | `density::domain::DensityRepository` | structure/mesh id 참조만 저장 |
| 창 열림/패널 배치/메뉴 상태 | `shell::domain::ShellStateStore` | 기존 `App` 내부 상태 이전 |
| render actor, overlay actor, camera | `render::infrastructure::*` | feature는 핸들 직접 소유 금지 |

명시 규칙:
- read model은 저장소를 읽어서 생성하며, 장기 보관 mutable state를 갖지 않는다.
- `structure`는 VTK actor를 직접 보유하지 않는다.
- `measurement`는 기하 계산과 표시 descriptor만 만들고 actor 생성은 `render`가 담당한다.
- `density`는 VTK volume/actor 생성을 직접 하지 않고 render port에 요청한다.

### 3.6 포트/어댑터 계약

| 포트 | 사용하는 모듈 | 구현 모듈 | 역할 |
|---|---|---|---|
| `RenderScenePort` | `mesh`, `structure`, `density` application | `render/infrastructure` | actor/volume add-remove, request render |
| `OverlayPort` | `measurement`, `structure` application | `render/infrastructure` | 2D/overlay descriptor 반영 |
| `PickingPort` | `measurement`, `workspace` | `render/infrastructure` | picker 결과 조회 |
| `CameraPort` | `shell`, `render` application | `render/infrastructure` | 카메라 정렬/리셋/투영 |
| `FileDialogPort` | `io` application | `platform/browser` | 파일 선택 |
| `WorkerPort` | `io` application | `platform/worker` | background parse |
| `ProgressPort` | `io`, `shell` | `shell/presentation` 또는 `platform` | 진행률/팝업 |
| `PersistencePort` | `shell`, `platform/wasm` | `platform/persistence` | layout, IDBFS |

금지 규칙:
- feature domain/application에서 `EM_ASM`, `emscripten::*`, `vtk*`, `imgui.h` 직접 include 금지
- presentation에서 repository mutable container 직접 수정 금지
- render infrastructure에서 feature singleton 직접 역참조 금지

### 3.7 빌드 타깃 설계(최종)

최종 CMake 타깃 목표:

- `wb_common`
- `wb_platform`
- `wb_shell`
- `wb_workspace`
- `wb_render`
- `wb_mesh`
- `wb_structure`
- `wb_measurement`
- `wb_density`
- `wb_io`

정책:
- `wb_atoms`는 최종적으로 제거한다.
- `wb_shell`만 여러 feature application을 조합할 수 있다.
- `wb_render`와 `wb_platform`은 adapter 구현만 포함하고 feature state를 포함하지 않는다.
- 타깃 간 순환 의존은 허용하지 않는다.

### 3.8 주요 유스케이스 흐름(목표)

1. Structure import
- `shell` 명령 -> `io::ImportWorkflowService`
- `FileDialogPort`로 파일 선택
- `WorkerPort`/parser 실행
- parsed result를 `structure`/`mesh`/`density` application에 분배
- `workspace`가 active ids를 갱신
- `render`가 scene 변경을 반영

2. Visibility toggle
- panel action -> `workspace` context 확인
- `mesh` 또는 `structure` application service 호출
- service가 repository 갱신
- `RenderScenePort`에 반영 요청
- `workspace` read model invalidation

3. Measurement creation
- viewer pick -> `PickingPort`
- `measurement::MeasurementService`가 구조 query를 사용해 계산
- 결과를 `MeasurementRepository`에 저장
- overlay descriptor를 `OverlayPort`로 전달

### 3.9 주석 작성 정책

목적:
- 모듈 경계가 재편되는 과정에서 새 코드의 의미, 책임, 상태 소유권, 수명 규칙을 코드만 읽고 추론해야 하는 비용을 줄인다.
- 특히 `service`, `repository`, `port`, `adapter`, `read model`, `store` 계층은 이름만으로 역할이 유사해지기 쉬우므로 문서화 규칙을 강제한다.

적용 대상:
- 새로 생성되는 `*.h`, `*.cpp`
- 1개 Phase 안에서 의미 단위가 재구성되는 대규모 정리 파일
- public API를 제공하는 구조체, 클래스, enum, 함수
- 상태 소유권/수명/단위 의미가 중요한 멤버 변수
- 동작이 자명하지 않은 private helper, 변환 함수, lifecycle 함수

기본 원칙:
- 주석 형식은 Doxygen 스타일을 사용한다.
- "무엇을 대입한다" 같은 문법 설명형 주석은 금지하고, 책임/의도/제약/부수효과를 설명한다.
- domain/application/presentation/infrastructure 간 경계 파일은 파일 수준 또는 타입 수준 설명을 반드시 남긴다.
- 상태 저장소와 포트 인터페이스에는 소유권, thread/lifetime, invalidation 규칙을 명시한다.

작성 규칙:
- 타입/함수 주석은 `/** ... */` 또는 `///`를 사용한다.
- 멤버 변수 주석은 `///<` 또는 `/**< ... */`를 사용한다.
- public 타입과 public 함수는 최소 `@brief`를 포함한다.
- 아래 항목에 해당하면 `@details`, `@param`, `@return`, `@note`, `@warning`을 추가한다.
  - orchestration/service entrypoint
  - cross-module contract
  - non-trivial return value
  - 수명/소유권/캐시 무효화 규칙
  - thread/main-thread 제약

주석 위치 지침:
- 헤더 파일(`.h`)에는 "계약"을 둔다.
  - 대상:
    - public/private를 포함한 클래스, 구조체, enum, 인터페이스
    - public 함수, virtual 함수, port 인터페이스
    - 상태 의미가 중요한 멤버 변수
    - inline 함수, template 함수, header-only 구현
  - 내용:
    - 무엇을 담당하는 타입/함수인지
    - 입력/출력 의미
    - 소유권, 수명, thread 제약
    - 호출자가 알아야 하는 전제조건/후조건
- 구현 파일(`.cpp`)에는 "실행 이유와 구현 세부"를 둔다.
  - 대상:
    - anonymous namespace helper
    - file-local static 함수/변수
    - 복잡한 분기, 변환 알고리즘, 임시 compatibility bridge
    - 구현체 내부에서만 중요한 성능/수명/VTK 처리 순서
  - 내용:
    - 왜 이런 순서/알고리즘/우회가 필요한지
    - 어떤 구현 리스크를 피하기 위한 코드인지
    - header 계약만으로는 드러나지 않는 내부 제약
- 동일 함수의 설명을 `.h`와 `.cpp`에 중복 작성하지 않는다.
  - 기본 계약은 `.h`에 둔다.
  - `.cpp`에는 구현상 추가 설명이 필요할 때만 보충한다.
- 선언이 헤더에 없고 구현 파일에만 존재하는 내부 타입/함수는 `.cpp`에 Doxygen 주석을 둔다.
- 대규모 리팩토링 중 임시 facade/shim/bridge는 `.cpp`에 `@note` 또는 `@warning`으로 "임시 계층"임을 명시하고, 최종 제거 대상임을 적는다.

권장 예시:

```cpp
/**
 * @brief Applies parsed structure data to the workspace and updates active context.
 * @param result Parsed import result owned by the workflow.
 * @return Newly activated structure id, or `-1` when apply failed.
 * @note This function must run on the main thread because it mutates UI-visible state.
 */
int32_t ApplyImportedStructure(const ParsedStructureResult& result);
```

Phase 게이트 반영:
- 각 Phase에서 새로 생성되거나 대규모로 정리된 파일은 주석 적용 여부를 함께 점검한다.
- Phase 종료 판정 시 "새 public 타입/함수에 Doxygen 주석 존재"를 코드 리뷰 체크리스트와 gate report에 포함한다.
- 최종적으로 compatibility facade를 제외한 새 아키텍처 계층 파일은 Doxygen 주석이 없는 public API를 남기지 않는다.

### 3.10 Git 저장소 운영 정책

목적:
- 각 Phase의 작업 단위를 `git log`로 추적 가능하게 남기고, 중간 결과가 로컬에만 머무르지 않도록 한다.
- 아키텍처 전환 과정의 회귀 원인을 Phase/WBS 단위로 역추적 가능하게 만든다.

브랜치 정책:
- 작업은 항상 Phase 단위 브랜치에서 시작한다.
- 브랜치 이름은 아래 규칙을 따른다.
  - `refactor/phase7-composition-root`
  - `refactor/phase8-atoms-template-dismantle`
  - `refactor/phase9-render-boundary-complete`
  - `refactor/phase10-workspace-mesh-io-decouple`
  - `refactor/phase11-shell-objectization`
  - `refactor/phase12-architecture-seal`

커밋/동기화 정책:
- 커밋은 최소 WBS 또는 논리 작업 단위 단위로 쪼갠다.
- 1개 커밋은 가능한 한 1개 책임만 담는다.
- 커밋 메시지는 Phase와 작업 단위를 식별 가능하게 작성한다.
  - 예: `phase7 W1 introduce workbench runtime skeleton`
  - 예: `phase8 W3 move measurement state into repository`
- 각 작업 단위 종료 시 로컬 커밋을 생성한다.
- Phase 진행 중에는 로컬 커밋 이력을 우선 유지하고, 원격 저장소 동기화는 수행하지 않는다.
- 원격 저장소 동기화(push)는 해당 Phase의 완료 선언 이후에만 수행한다.
- gate 문서/로그 업데이트도 별도 커밋 또는 같은 작업팩의 마지막 커밋으로 남긴다.

이력 가시성 정책:
- `git log --oneline --decorate <phase-branch>`만으로도 WBS 순서와 변경 의도를 파악할 수 있어야 한다.
- 세부계획서에는 branch 이름, 예상 커밋 묶음, gate 커밋 시점을 명시한다.
- 대형 리팩토링 결과를 한 번에 squash한 뒤 세부 이력을 잃는 방식은 지양한다.

Phase 종료 운영 정책:
- Phase 종료 전에는 해당 Phase 브랜치에서 관련 컴파일 테스트를 반드시 수행한다.
- 컴파일/테스트 결과 로그는 `docs/refactoring/phaseN/logs/`에 남긴다.
- Phase 종료 시 아래 산출물이 모두 준비되어야 한다.
  - 로컬 브랜치에 WBS 단위 커밋 이력 정리 완료
  - 관련 컴파일/테스트 PASS 로그
  - `dependency_gate_report.md`
  - `go_no_go_phase{N+1}.md`
  - 세부계획서 진행 상태 업데이트
- 위 조건이 충족되어야만 "Phase 종료 판정 가능" 상태로 본다.
- 원격 저장소 push는 이 완료 선언 직후 수행하며, Phase 종료 기록을 원격 이력으로 고정한다.

컴파일/검증 정책:
- C++/WASM 빌드 영향이 있는 Phase는 최소 1회 이상 컴파일 테스트를 수행한다.
- 권장 기준:
  - 구조/렌더/메쉬/아키텍처 변경: `npm run build-wasm:release` 또는 동등한 CMake install 빌드
  - 테스트 체계 변경: `npm run test:cpp`, `npm run test:smoke`
  - 빌드 구조 변경: 해당 phase gate 스크립트 + clean rebuild
- Phase 종료 문서에는 어떤 명령을 언제 실행했는지 구체적으로 기록한다.

### 3.11 리팩토링 문서 배치 정책

기본 정책:
- 최초 전체 리팩토링 계획문서는 `docs/refactoring/refactoring_plan_260324.md`를 기준 문서로 사용한다.
- 새로운 Phase에 진입할 때마다 `docs/refactoring/phaseN/` 폴더를 먼저 생성한다.
- 해당 Phase에서 생산되는 세부계획서, gate report, go/no-go 문서, 로그는 모두 그 폴더 내부에 둔다.

표준 폴더 구조:

```text
docs/refactoring/
  refactoring_plan_260324.md
  phaseN/
    refactoring_phaseN_<topic>_<date>.md
    dependency_gate_report.md
    go_no_go_phase{N+1}.md
    logs/
      *.txt
      *.md
```

운영 규칙:
- 새로운 상세계획서는 Phase 시작 시점에 `phaseN` 폴더 안에서 작성한다.
- 이후 생산되는 검증 로그, inventory, 회의/판정 문서는 루트가 아니라 같은 `phaseN` 폴더에 누적한다.
- 문서 형식은 기존 Phase 1~6 산출물 형식을 따른다.
  - 상세계획서
  - `dependency_gate_report.md`
  - `go_no_go_phase{N+1}.md`
  - `logs/` 하위 근거 파일
- 새로운 공통 정책 문서나 전체 결과보고서만 `docs/refactoring/` 루트에 두고, Phase 전용 문서는 루트에 두지 않는다.

## 4. 단계별 리팩토링 실행안

모든 Phase는 `3.9 주석 작성 정책`과 `3.10 Git 저장소 운영 정책`을 공통 적용한다.

공통 규칙:
- Phase 시작 시 전용 브랜치를 생성한다.
- WBS 단위로 로컬 커밋을 남긴다.
- 원격 저장소 push는 각 Phase의 완료 선언 이후에만 수행한다.
- 새로 분리되는 public API와 의미 있는 상태 단위에는 Doxygen 주석을 추가한다.
- Phase 종료 전 컴파일/테스트를 수행하고 로그를 남긴다.
- gate report와 go/no-go 문서가 준비되어야 Phase 종료 판정이 가능하다.

## Phase 7. Composition Root 도입 + singleton quarantine (3~5일)
- 작업
  - `shell/runtime/WorkbenchRuntime` 도입
  - `main.cpp`, `bind_function.cpp`, `app.cpp`가 runtime public API를 통해 기능 호출하도록 정리
  - `::Instance()` 신규 추가 금지 스크립트 도입
  - singleton 접근을 composition root와 호환 facade 내부로 한정
  - 새 runtime API와 port 인터페이스에 Doxygen 주석 추가
- 산출물
  - `webassembly/src/shell/runtime/workbench_runtime.h/.cpp`
  - `webassembly/src/platform/wasm/workbench_bindings.cpp`
  - 정적 검사 스크립트(예: `check_phase7_runtime_composition.ps1`)
- 완료 기준
  - `app.cpp`에서 feature singleton 직접 lookup이 대폭 축소됨
  - wasm binding이 runtime API만 사용함
  - 신규 코드에서 `DECLARE_SINGLETON` 도입 0건
  - 컴파일 로그 + gate 문서 준비 후 Phase 완료 선언 가능
  - 완료 선언 직후 Phase 브랜치 원격 push 수행

## Phase 8. `AtomsTemplate` 해체: structure / measurement / density 분리 (6~8일)
- 작업
  - `AtomsTemplate`의 구조 편집, 구조 lifecycle, 가시성, 측정, density, BZ 책임을 분리
  - `StructureStateStore`를 `structure` repository 중심 API로 치환
  - `friend class` 제거
  - legacy alias(`createdAtoms`, `createdBonds`, `cellInfo`) 노출 축소
  - 새 repository/service/store/read-model API에 Doxygen 주석 추가
- 산출물
  - `structure/*`, `measurement/*`, `density/*` 디렉터리 도입
  - `AtomsTemplate` 임시 facade 또는 compatibility shim
- 완료 기준
  - `atoms_template.cpp`가 facade 수준으로 축소되거나 제거됨
  - `atoms_template.h`의 `friend class` 0건
  - feature 로직이 새 모듈로 이동
  - 컴파일 로그 + gate 문서 준비 후 Phase 완료 선언 가능
  - 완료 선언 직후 Phase 브랜치 원격 push 수행

## Phase 9. `VtkViewer` 해체 + render 포트 완성 (5~7일)
- 작업
  - viewer를 viewport, camera, picking, overlay, scene renderer로 분리
  - `VtkViewer::Instance()` 직접 사용 지점을 render port 호출로 전환
  - feature -> render infrastructure 역참조 제거
  - 새 render port/adapter와 ownership 규칙에 Doxygen 주석 추가
- 산출물
  - `render/application/*` 포트 확장
  - `render/infrastructure/*` adapter 재편
  - viewer interaction/picking/camera 책임 분리
- 완료 기준
  - `render` 외부의 `VtkViewer::Instance()` 호출 0건
  - `vtk_viewer.cpp`는 렌더 infrastructure 구현으로만 남음
  - measurement/density/structure overlay가 render port로만 동작
  - 컴파일 로그 + gate 문서 준비 후 Phase 완료 선언 가능
  - 완료 선언 직후 Phase 브랜치 원격 push 수행

## Phase 10. `MeshManager` / `FileLoader` / workspace 조정 분리 (5~7일)
- 작업
  - `MeshManager`를 repository + query/service로 해체
  - `FileLoader`를 import workflow facade로 축소
  - `workspace` 도입으로 selected mesh / current structure / active density context를 단일화
  - `ModelTree`, `MeshDetail`, `MeshGroupDetail`는 read model 기반 presenter로 전환
  - 새 workflow/query/read-model API에 Doxygen 주석 추가
- 산출물
  - `workspace/domain/workspace_store.*`
  - `mesh/domain/mesh_repository.*`
  - `io/application/import_workflow_service.*`
- 완료 기준
  - feature 코드에서 `MeshManager::Instance()`, `FileLoader::Instance()` 호출 0건
  - panel은 repository 직접 수정 대신 application service 호출
  - import/selection 문맥이 `workspace`를 통해 일관되게 관리됨
  - 컴파일 로그 + gate 문서 준비 후 Phase 완료 선언 가능
  - 완료 선언 직후 Phase 브랜치 원격 push 수행

## Phase 11. Shell / Panel 객체화 + `App` 축소 (4~6일)
- 작업
  - `App`, `Toolbar`, `ModelTree`, `MeshDetail`, `MeshGroupDetail`, `TestWindow` singleton을 plain object/presenter로 전환
  - 창 열림 상태와 레이아웃 상태를 `ShellStateStore`로 이전
  - command dispatch와 menu action을 `shell/application`으로 이동
  - shell controller/presenter/state API에 Doxygen 주석 추가
- 산출물
  - `shell/application/workbench_controller.*`
  - `shell/presentation/main_menu.cpp`
  - `shell/presentation/window_layout.cpp`
- 완료 기준
  - `app.cpp`는 런타임 frame orchestration만 담당
  - panel singleton 0건
  - UI 상태 저장 위치가 명확해짐
  - 컴파일 로그 + gate 문서 준비 후 Phase 완료 선언 가능
  - 완료 선언 직후 Phase 브랜치 원격 push 수행

## Phase 12. Compatibility facade 제거 + 모듈 seal + CI 게이트 (3~5일)
- 작업
  - `AtomsTemplate`, `MeshManager`, `FileLoader`, `ModelTree`, `Toolbar`, `VtkViewer` compatibility facade 제거
  - build target을 최종 모듈 단위로 재정렬
  - architecture gate 스크립트 추가
  - Phase 6 테스트에 architecture regression 게이트 추가
  - 최종 public API 주석 누락분 정리
- 산출물
  - 최종 CMake 모듈 구성
  - 정적 아키텍처 검사 스크립트
  - 결과보고서 / go-no-go 문서
- 완료 기준
  - `::Instance()` 호출 0건 또는 runtime composition root로만 제한
  - legacy alias 외부 노출 0건
  - 최종 모듈 그래프 비순환
  - 기존 smoke + C++ 테스트 + architecture gate 동시 PASS

## 5. 우선순위 백로그

### P0 (즉시)
- `WorkbenchRuntime` 도입
- singleton 신규 유입 금지 규칙
- `workspace` 상태 모델 정의
- `AtomsTemplate` 책임 분해 설계 확정

### P1 (단기)
- `structure/measurement/density` 실분리
- render port 완성
- `MeshManager` repository화
- shell state store 도입

### P2 (중기)
- 디렉터리 rename 및 facade 삭제
- 최종 build target 재배치
- host-native test runner 또는 CI 병행

## 6. 예상 리스크와 대응

1. 리스크: 대형 클래스 해체 과정에서 런타임 회귀 다발
- 대응:
  - Phase 6 smoke를 import/visibility/measurement/density 시나리오로 확장
  - facade를 남긴 채 내부 구현만 단계적으로 이동

2. 리스크: VTK actor/overlay lifetime 정리 중 use-after-free 또는 누락 렌더
- 대응:
  - actor 소유권을 render infrastructure로 단일화
  - overlay descriptor -> actor 변환을 render 내부로 제한

3. 리스크: 구조/mesh/density 상태가 workspace에서 다시 중복될 가능성
- 대응:
  - workspace는 `id/context/state pointer`만 보유하고 실데이터는 각 repository가 보유
  - read model은 derived data만 생성

4. 리스크: 디렉터리 rename이 merge/검색/문서 경로를 크게 흔듦
- 대응:
  - Phase 8~10에서는 logical module 먼저 분리
  - 실제 directory rename은 facade 제거 직전 단계에서 수행

5. 리스크: build target 세분화 후 순환 의존이 다시 생길 수 있음
- 대응:
  - phase별 CMake inventory 로그 유지
  - `target_link_libraries` 방향성을 정적 검사

## 7. 완료(DoD) 기준

아키텍처 전환 완료는 아래 조건을 모두 만족할 때로 정의한다.

1. `AtomsTemplate`, `MeshManager`, `FileLoader`, `ModelTree`, `Toolbar`, `VtkViewer` singleton이 제거되었거나 runtime composition root 내부 shim으로만 제한됨
2. feature/application/domain 코드에서 `::Instance()` 호출 0건
3. `friend class` 0건
4. `createdAtoms`, `createdBonds`, `cellInfo` 등 legacy alias가 compatibility layer 밖으로 노출되지 않음
5. `wb_atoms` 제거 및 최종 모듈 타깃 그래프 비순환
6. `app.cpp`는 shell orchestration 파일로 축소되고 feature 로직을 포함하지 않음
7. `render` 외부에서 VTK actor/volume를 직접 조작하지 않음
8. `npm run test:cpp`, `npm run test:smoke`, architecture gate 스크립트가 모두 PASS
9. 최종 결과보고서와 go/no-go 문서가 작성됨
10. 새로 정리된 public 타입/함수/상태 단위에 Doxygen 스타일 주석이 적용됨
11. 각 Phase가 전용 브랜치에서 수행되었고, WBS 단위 로컬 커밋과 Phase 완료 직후 원격 push 이력이 `git log`와 문서로 추적 가능함

## 8. 착수 권장 순서 (실행안)

1. Phase 7을 먼저 수행해 runtime composition root와 규칙을 만든다.
2. 그 다음 Phase 8에서 `AtomsTemplate`를 분해한다.
3. Phase 9에서 render 경계를 완성해 viewer 직접 의존을 끝낸다.
4. Phase 10에서 mesh/io/workspace 조정을 정리한다.
5. Phase 11에서 shell/panel singletons를 걷어낸다.
6. 마지막으로 Phase 12에서 facade 제거, 빌드 seal, architecture gate를 확정한다.

권장 이유:
- runtime composition root 없이 곧바로 god-object 해체를 시작하면 새 서비스들도 다시 singleton으로 굳을 위험이 크다.
- `AtomsTemplate`와 `VtkViewer`는 서로 얽혀 있으므로 `Phase 8 -> Phase 9` 순서를 유지해야 regression 제어가 쉽다.

### 부록 A. 주요 근거 파일
- `docs/refactoring/refactoring_result_report_phase1_6_260331.md`
- `docs/refactoring/refactoring_plan_260324.md`
- `webassembly/src/atoms/atoms_template.cpp`
- `webassembly/src/vtk_viewer.cpp`
- `webassembly/src/app.cpp`
- `webassembly/src/file_loader.cpp`
- `webassembly/src/mesh.cpp`
- `webassembly/src/mesh_manager.cpp`
- `webassembly/src/render/application/render_gateway.h`
- `webassembly/src/atoms/domain/structure_state_store.h`
- `webassembly/tests/CMakeLists.txt`

## 9. 기존 아티팩트 -> 목표 아키텍처 매핑표

| 현재 아티팩트 | 목표 아티팩트 | 최종 책임 |
|---|---|---|
| `App` | `shell::application::WorkbenchController`, `shell::presentation::*`, `shell::domain::ShellStateStore` | 메뉴/창 배치/command dispatch |
| `AtomsTemplate` | `structure::*`, `measurement::*`, `density::*` + 임시 facade | feature use-case 및 상태 조정 |
| `VtkViewer` | `render::infrastructure::ViewportAdapter`, `CameraAdapter`, `PickingAdapter`, `OverlayRenderer` | 렌더 인프라 |
| `MeshManager` | `mesh::domain::MeshRepository`, `mesh::application::MeshService` | mesh 상태/조회/가시성 |
| `FileLoader` | `io::application::ImportWorkflowService`, `platform::browser::FileDialogAdapter`, `platform::worker::WorkerAdapter` | import orchestration |
| `ModelTree` | `workspace::presentation::SceneTreePanel` | tree read model 렌더링 |
| `MeshDetail` | `mesh::presentation::MeshDetailPanel` | mesh panel 렌더링 |
| `MeshGroupDetail` | `mesh::presentation::MeshGroupPanel` | mesh group panel 렌더링 |
| `Toolbar` | `shell::presentation::CommandBar` | 공용 명령 UI |
| `bind_function.cpp` | `platform/wasm/workbench_bindings.cpp` | wasm entrypoint |

## 10. 최종 판정 기준 문장

이 계획서의 종료 판정은 단순한 파일 분해가 아니라 아래 상태를 의미한다.

- 모듈 경계가 build/runtime/state ownership 차원에서 서로 일치한다.
- feature가 singleton lookup 없이 명시적 의존성으로 조립된다.
- VTK/Emscripten/ImGui 의존은 infrastructure/presentation에 격리된다.
- `webassembly/src`는 더 이상 `AtomsTemplate`나 `VtkViewer` 같은 god-object를 중심으로 돌아가지 않는다.
