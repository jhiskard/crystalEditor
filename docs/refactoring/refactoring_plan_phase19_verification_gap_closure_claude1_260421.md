# 리팩토링 실행 계획서: Phase 19 (독립 검증 간극 해소 + Legacy 전면 해체)

작성일: `2026-04-21`
개정일: `2026-04-21` (rev.4 — **Legacy 기능별 이관 위치(Migration Target Map) 명시** / rev.3: `webassembly/src` 하위 **모든 `legacy/` 폴더** 해체 포함)
대상 저장소: `vtk-workbench_jclee`
기준 브랜치: `main` (`80c2709`)
상위 근거 문서: [`refactoring_verification_report_independent_260420.md`](refactoring_verification_report_independent_260420.md)

## 0. 문서 개요

### 0.1 위치 및 역할

본 문서는 `refactoring_verification_report_independent_260420.md`에서 식별한 네 가지 간극(G1~G4)을 해소하기 위한 코드 수정 계획이다. rev.3 개정에서는 `webassembly/src` 하위에 존재하는 **5개 `legacy/` 폴더 전부**를 물리적으로 제거하는 것을 범위에 포함한다.

### 0.2 개정 이력

| rev | 일자 | 변경 요지 |
|---|---|---|
| rev.1 | 2026-04-21 | G3를 "legacy quarantine 정식화"로 해소 |
| rev.2 | 2026-04-21 | G3 전략을 "전면 해체"로 전환, `workspace/legacy/` 제거 및 `AtomsTemplate` 해체 |
| rev.3 | 2026-04-21 | `density/io/measurement/structure` 하위 4개 `infrastructure/legacy/` 폴더 + `workspace/legacy/`까지 5개 legacy 폴더 전량 해체 목표로 확장. 4개 Port 인터페이스(`ImportRuntimePort`, `StructureServicePort`, `MeasurementServicePort`, `DensityServicePort`) 정리 포함 |
| **rev.4** | 2026-04-21 | **해체 대상 legacy 각각의 "기능 이관 목표 위치"(Migration Target Map)를 §2.4에 명시**. 슬라이스 S1~S8 8개 묶음·`LegacyAtomsRuntime()` 접근점·4개 Port·friend·`app.cpp`·`vtkActor` 경계 각각에 대해 신규·수정 대상 파일 경로, 수용 서비스·클래스·메서드, `WorkbenchRuntime` getter 대응표 부여 |

### 0.3 rev.2 → rev.3 변경 요지

| 항목 | rev.2 | rev.3 |
|---|---|---|
| 해체 대상 legacy 폴더 | 1 (`workspace/legacy`) | **5** (위 + `{density,io,measurement,structure}/infrastructure/legacy`) |
| 해체 대상 파일 수 | 4 (`atoms_template_facade.{h,cpp}`, `legacy_atoms_runtime.{h,cpp}`) | **12** (위 + 8 포트 어댑터 파일) |
| Port 인터페이스 처리 | 미정 | **Port 인터페이스 4종 dissolve (서비스에 책임 흡수)** |
| 작업 단위 수 | 16 (W0~W15) | **17** (W0~W16; W12가 "Feature Service Internalization"으로 확장) |
| 지표 개수(§3) | 11 | 15 |

### 0.4 rev.3 → rev.4 변경 요지

| 항목 | rev.3 | rev.4 |
|---|---|---|
| 기능 이관 위치 명시 | 슬라이스 카테고리 수준 서술 | **§2.4 "Migration Target Map" 신설** — 9개 서브섹션 표로 이관 대상 파일 경로·수용 서비스 클래스·메서드 단위까지 명시 |
| `AtomsTemplate` 메서드 매핑 | S1~S8 이름 수준 | 슬라이스별 이관 대상 파일 (신규/확장 여부 명시), 수용 서비스, 도메인 상태 소유 열 부여 |
| `LegacyAtomsRuntime()` 전환 | `WorkbenchRuntime` 10 getter 언급 | **getter 10종 × 반환 타입 × 제공 슬라이스 × 기존 호출 예 교차표** |
| Port 처리 | dissolve 결정 | **Port → 서비스 native API 내재화 표** + **어댑터 책임 소멸 표** |
| G1/G2/G4 | W1/W2/W3 상세 | 이관 전/후 심볼·파일 매핑 표 부여 |
| 이관 다이어그램 | 없음 | **§2.4.8 텍스트 다이어그램**으로 legacy → feature 경로 전체 가시화 |
| 이관 불변식 | 암묵 | **§2.4.9 네 가지 원칙 (단일 수용, 단일 도메인 소유, Port 미도입, 경로 존재성)** |

### 0.5 관련 계획 문서

| 문서 | 관계 |
|---|---|
| `refactoring_plan_260324.md` | Phase 1~6 원계획 |
| `refactoring_plan_full_modular_architecture_260331.md` | Phase 7~12 DoD |
| `refactoring_plan_architecture_completion_phase13plus_260406.md` | Phase 13~17 DoD |
| `phase18/refactoring_phase18_phase17_limit_closure_execution_plan_260416.md` | Phase 18 실행 계획 |
| `refactoring_verification_report_independent_260420.md` | 간극 식별 근거 |

## 1. 배경

독립 검증에서 확인된 네 가지 간극과 함께, 본 개정에서는 Phase 17~18 과정에서 임시 브릿지로 도입된 **4개 Legacy Port 어댑터 폴더**도 해체 범위에 포함한다. 이 어댑터들은 도입 당시 Phase18-W6 closeout에서 제거 예정이었으나 실제로는 주석에 `Track: P18-W3-*-PORT` 표시만 남은 채 잔존하고 있다.

| # | 간극 / 부채 | 계획서 DoD 근거 | 현재 상태 |
|---|---|---|---|
| G1 | `friend class` 1건 잔존 | Phase 14 DoD: `friend class = 0` | `io/application/import_entry_service.h:76` |
| G2 | `app.cpp` 미분할 | Phase 11: `main_menu.cpp`/`window_layout.cpp` 신설 | `app.cpp` 1,430 lines / 34 메서드 |
| G3 | `atoms_template_facade.cpp` 거대 파일 | Phase 8: "facade 수준 축소" | 5,808 lines, 190 메서드, 33 include 사이트 |
| G3' | 4개 Legacy Port 어댑터 폴더 잔존 | Phase 18-W6 closeout 주석: "remove in Phase18-W6" | `{density,io,measurement,structure}/infrastructure/legacy/` 8개 파일 / 526 lines |
| G4 | `vtkActor*` 비-render 참조 1건 | Phase 15: render-port isolation | `measurement/application/measurement_controller.cpp:43` |

## 2. 범위 및 목표

### 2.1 최종 종료 상태 (End State)

**구조 해체**

1. `webassembly/src/workspace/legacy/` 디렉터리 **삭제** (4 파일 / 7,317 lines)
2. `webassembly/src/density/infrastructure/legacy/` 디렉터리 **삭제** (2 파일 / 104 lines)
3. `webassembly/src/io/infrastructure/legacy/` 디렉터리 **삭제** (2 파일 / 169 lines)
4. `webassembly/src/measurement/infrastructure/legacy/` 디렉터리 **삭제** (2 파일 / 148 lines)
5. `webassembly/src/structure/infrastructure/legacy/` 디렉터리 **삭제** (2 파일 / 105 lines)

**심볼 제거**

6. `AtomsTemplate` 클래스 및 메서드 전부 제거
7. `workspace::legacy::LegacyAtomsRuntime()` 함수 제거
8. `LegacyStructureServicePort`, `LegacyMeasurementServicePort`, `LegacyDensityServicePort`, `LegacyImportRuntimePort` 클래스 4종 제거
9. 4개 Port 인터페이스(`StructureServicePort`, `MeasurementServicePort`, `DensityServicePort`, `ImportRuntimePort`) **dissolve** — 서비스가 자체 소유

**호출자 전환**

10. 기존 33개 `#include ".../legacy_atoms_runtime.h"` 전부 feature 서비스 include로 치환
11. 모든 호출자는 `WorkbenchRuntime` 기반 서비스 getter만 경유

**간극 해소**

12. G1, G2, G4 동시 해소 (§2.2 `In Scope`)

### 2.2 In Scope

1. G1~G4 + G3' 다섯 항목 코드 수정
2. `AtomsTemplate` 190개 메서드 기능 슬라이스 추출(S1~S8)
3. 33개 include 사이트 마이그레이션
4. `WorkbenchRuntime` 서비스 getter 10종 신설
5. 4개 Port 인터페이스 dissolve + 서비스 internalization
6. 5개 `legacy/` 폴더 + 12개 파일 물리 삭제
7. Phase 19 게이트 16종 + 종료 문서 패키지

### 2.3 Out of Scope

1. 잔존 singleton 3건(`VtkViewer`, `MeshManager`, `FontManager`) 전면 제거 — Phase 20
2. 전체 `::Instance()` 48건 추가 감축 — Phase 20
3. 10 모듈 타깃 그래프 변경 — **불변**
4. UI/렌더링 동작 결과물 변경 — **불변**

### 2.4 기능 이관 매핑 (Migration Target Map) — rev.4 신규

본 절은 해체 대상 legacy 자산이 **어디로 이관되는지**를 단일 표 집합으로 명시한다. 작업 단위(§4, §5)와 기능 슬라이스(§2.4.1 S1~S8)는 본 매핑을 근거로 설계된다. 모든 "이관 대상 경로"는 `webassembly/src/` 기준 상대 경로이며, 존재하지 않는 경로는 W4~W11 단계에서 **신설**된다.

#### 2.4.1 `AtomsTemplate` (190 메서드) 기능 슬라이스 S1~S8 이관 표

`workspace/legacy/atoms_template_facade.{h,cpp}`에 정의된 190개 `AtomsTemplate::*` 메서드를 다음 8개 기능 슬라이스로 절단하여 feature 모듈로 이관한다. 각 슬라이스는 W5~W11의 개별 작업 단위에 매핑된다.

| 슬라이스 | 작업 단위 | 기능 카테고리(원 AtomsTemplate 메서드군) | 이관 대상 모듈 | 이관 대상 파일 (신규/확장) | 수용 서비스 클래스 | 소유 도메인 상태 |
|---|---|---|---|---|---|---|
| **S1** | W5 | Measurement (거리·각도·이면각·측정 click handler) | `measurement` | `measurement/application/measurement_controller.cpp` (확장) · `measurement/application/measurement_session_service.{h,cpp}` (신규) | `measurement::application::MeasurementService` / `MeasurementSessionService` | `measurement::domain::MeasurementSession` |
| **S2** | W6 | Bond (본드 그래프 구성·bond actor 생성·bond visibility) | `structure` + `render` | `structure/application/bond_service.{h,cpp}` (신규) · `render/infrastructure/atoms/bond_group_vtk.{h,cpp}` (신규) | `structure::application::BondService` | `structure::domain::BondGraph` |
| **S3** | W7 | Charge Density / Slice Viewer (등위면·슬라이스·볼륨 맵핑) | `density` | `density/application/charge_density_service.{h,cpp}` (신규) · `density/application/slice_viewer_service.{h,cpp}` (신규) · `density/presentation/charge_density_view.{h,cpp}` (확장) | `density::application::ChargeDensityService` / `SliceViewerService` | `density::domain::DensityVolume`, `DensitySlice` |
| **S4** | W8 | Structure Metadata / Lifecycle (원소 테이블·공간군·셀 벡터·로드/클리어/리셋) | `structure` | `structure/application/structure_metadata_service.{h,cpp}` (신규) · `structure/application/structure_lifecycle_service.{h,cpp}` (확장) | `structure::application::StructureMetadataService` / `StructureLifecycleService` | `structure::domain::AtomGroups`, `UnitCell` |
| **S5** | W9 | Advanced Grid Visibility (sub-cell, super-cell, 격자 프레임 가시성) | `structure` | `structure/application/advanced_grid_visibility_service.{h,cpp}` (신규) · `structure/application/visibility_service.cpp` (확장) | `structure::application::AdvancedGridVisibilityService` | `structure::domain::GridVisibilityState` |
| **S6** | W10 | Picking / Hover / Tooltip (vtkActor → atom 선택·툴팁 렌더링) | `render` + `structure` + `shell` | `render/application/atom_picking_service.{h,cpp}` (신규) · `render/infrastructure/vtk_atom_picking_service.{h,cpp}` (신규) · `structure/application/atom_selection_service.{h,cpp}` (신규) · `shell/presentation/atoms/atom_tooltip_renderer.{h,cpp}` (신규) | `render::application::AtomPickingService`, `structure::application::AtomSelectionService`, `shell::presentation::AtomTooltipRenderer` | `structure::domain::AtomSelection` |
| **S7** | W11 | AtomsTemplate UI 렌더링 (builder/editor/advanced/template imgui 창) | `shell` | `shell/presentation/atoms/builder_view.{h,cpp}` (신규) · `editor_view.{h,cpp}` (신규) · `advanced_view.{h,cpp}` (신규) · `template_window.{h,cpp}` (신규) | `shell::presentation::atoms::{BuilderView,EditorView,AdvancedView,TemplateWindow}` | (UI 상태만 — domain 무) |
| **S8** | W9 | File I/O (XSF/CHGCAR 로드·구조 → 파서 연동) | `io` | `io/application/xsf_load_service.{h,cpp}` (신규) · `io/application/chgcar_load_service.{h,cpp}` (신규) | `io::application::XsfLoadService` / `ChgcarLoadService` | (무 — domain 상태 소유는 S4가 담당) |

**슬라이스 불변식**: 각 메서드는 정확히 한 슬라이스에만 귀속된다. W4에서 작성되는 `atoms_template_method_inventory.csv`는 `[method, slice, targetFile, targetClass, targetMethod]` 5열을 가진다.

#### 2.4.2 `LegacyAtomsRuntime()` 접근점 → `WorkbenchRuntime` 서비스 getter 10종 이관 표

현재 `workspace::legacy::LegacyAtomsRuntime()` 단일 함수가 33개 호출 사이트의 진입점이다. Phase 19에서는 `shell/runtime/workbench_runtime.{h,cpp}`에 다음 10개 getter를 신설하여 각 서비스 singleton 접근을 대체한다.

| 신규 getter (WorkbenchRuntime 멤버) | 반환 타입 | 제공 슬라이스 | 기존 대체 호출 예 |
|---|---|---|---|
| `bondService()` | `structure::application::BondService&` | S2 | `LegacyAtomsRuntime().UpdateBondList()` |
| `chargeDensityService()` | `density::application::ChargeDensityService&` | S3 | `LegacyAtomsRuntime().ShowIsoSurface(...)` |
| `sliceViewerService()` | `density::application::SliceViewerService&` | S3 | `LegacyAtomsRuntime().UpdateSlicePlane(...)` |
| `structureMetadataService()` | `structure::application::StructureMetadataService&` | S4 | `LegacyAtomsRuntime().GetCellVectors()` |
| `advancedGridVisibilityService()` | `structure::application::AdvancedGridVisibilityService&` | S5 | `LegacyAtomsRuntime().SetSubCellVisible(...)` |
| `atomPickingService()` | `render::application::AtomPickingService&` | S6 | `LegacyAtomsRuntime().PickActor(...)` |
| `atomSelectionService()` | `structure::application::AtomSelectionService&` | S6 | `LegacyAtomsRuntime().GetSelectedAtomIndex()` |
| `atomTooltipRenderer()` | `shell::presentation::AtomTooltipRenderer&` | S6 | `LegacyAtomsRuntime().RenderTooltip(...)` |
| `xsfLoadService()` | `io::application::XsfLoadService&` | S8 | `LegacyAtomsRuntime().LoadXsf(...)` |
| `chgcarLoadService()` | `io::application::ChgcarLoadService&` | S8 | `LegacyAtomsRuntime().LoadChgcar(...)` |

Phase 18 이전에 이미 존재하던 getter (`structureService()`, `measurementService()`, `densityService()`, `importEntryService()` 등)는 변경 없이 유지되며, W12 Port dissolve 이후에는 내부 구현만 갱신된다.

**이관 대상 파일**:

- 신규 멤버 선언 추가: `webassembly/src/shell/runtime/workbench_runtime.h`
- getter 정의 추가: `webassembly/src/shell/runtime/workbench_runtime.cpp`
- 서비스 생성자 호출 순서는 W4 `service_construction_order.md`에 고정

#### 2.4.3 4개 Port 인터페이스 → 서비스 native API 내재화 이관 표

| Port 인터페이스 (제거 대상) | 현재 위치 | 책임 흡수 대상 서비스 | 흡수 대상 파일 | 노출 API 형태 |
|---|---|---|---|---|
| `StructureServicePort` | `structure/application/structure_service_port.h` | `structure::application::StructureService` | `structure/application/structure_service.{h,cpp}` | 기존 Port 가상 메서드 → 서비스의 public native 메서드 (`virtual` 제거) |
| `MeasurementServicePort` | `measurement/application/measurement_service_port.h` | `measurement::application::MeasurementService` | `measurement/application/measurement_service.{h,cpp}` | 동일 |
| `DensityServicePort` | `density/application/density_service_port.h` | `density::application::DensityService` | `density/application/density_service.{h,cpp}` | 동일 |
| `ImportRuntimePort` | `io/application/import_runtime_port.h` | `io::application::ImportEntryService` 및 동반 서비스 | `io/application/import_entry_service.{h,cpp}`, `parser_worker_service.{h,cpp}`, `import_apply_service.{h,cpp}`, `import_workflow_service.{h,cpp}` | `LegacyParent()` 류 `AtomsTemplate*` 반환 제거. 필요한 상태는 §2.4.2 신규 getter (`structureMetadataService()`, `chargeDensityService()` 등) 경유로 직접 접근 |

#### 2.4.4 4개 Legacy Port 어댑터 → 어댑터 책임 이관 표

어댑터 구현체가 보유하던 "`AtomsTemplate` 호출 → Port 시그니처 변환" 로직은 Phase 19에서 소멸한다. 구현 로직은 W5~W11 슬라이스 본체 이관 시점에 이미 수용 서비스로 흡수되므로, 어댑터 파일 자체는 W12에서 **빈 파일화 후 W14에서 물리 삭제**된다.

| 어댑터 클래스 (제거 대상) | 현재 위치 | 현재 호출 대상 | Phase 19 이관 후 |
|---|---|---|---|
| `LegacyStructureServicePort` | `structure/infrastructure/legacy/legacy_structure_service_port.{h,cpp}` | `AtomsTemplate::*` (S4/S5) | 책임 소멸 — `StructureService`가 `StructureMetadataService`/`AdvancedGridVisibilityService` 직접 사용 |
| `LegacyMeasurementServicePort` | `measurement/infrastructure/legacy/legacy_measurement_service_port.{h,cpp}` | `AtomsTemplate::*` (S1) | 책임 소멸 — `MeasurementService`가 `MeasurementSessionService` 직접 사용 |
| `LegacyDensityServicePort` | `density/infrastructure/legacy/legacy_density_service_port.{h,cpp}` | `AtomsTemplate::*` (S3) | 책임 소멸 — `DensityService`가 `ChargeDensityService`/`SliceViewerService` 직접 사용 |
| `LegacyImportRuntimePort` | `io/infrastructure/legacy/legacy_import_runtime_port.{h,cpp}` | `AtomsTemplate::*` (S4/S3/S8) | 책임 소멸 — `ImportEntryService`가 §2.4.2 신규 getter 경유 |

#### 2.4.5 G1 `friend class` 제거 — 책임 이관 표

`io/application/import_entry_service.h:76`의 유일한 `friend class shell::presentation::ImportPopupPresenter;` 선언을 제거한다. 현재 `ImportPopupPresenter`가 직접 접근하는 `FileLoader`의 8개 private 멤버·메서드는 다음 대체 API로 이관한다.

| 현재 private 접근 (ImportPopupPresenter 소비) | 이관 후 public API (FileLoader 제공) | 이관 대상 파일 |
|---|---|---|
| `m_ShowStructureReplacePopup`, `m_ShowStructureImportErrorPopup`, `m_ShowXsfGridCellWarningPopup`, `m_StructureImportErrorTitle`, `m_StructureImportErrorMessage`, `m_XsfGridCellWarningText`, `m_DeferredStructureFileName` (읽기) | `ConsumeImportPopupViewState() -> ImportPopupViewState` (POD 스냅샷 반환, 읽기와 동시에 1회성 트리거 플래그 리셋) | `io/application/import_entry_service.{h,cpp}` |
| `m_ReplaceSceneOnNextStructureImport = true` + `handleStructureFile(...)` 또는 `OpenStructureFileBrowser()` 호출 (Yes 버튼) | `ConfirmStructureReplaceImport()` | 동일 |
| `m_ReplaceSceneOnNextStructureImport = false` + `m_DeferredStructureFileName.clear()` + `std::remove(memfsPath)` (No 버튼) | `CancelStructureReplaceImport()` | 동일 |
| `m_StructureImportErrorTitle.clear()` / `m_StructureImportErrorMessage.clear()` | `DismissStructureImportError()` | 동일 |
| `m_XsfGridCellWarningText.clear()` | `DismissXsfGridCellWarning()` | 동일 |
| POD 타입 `ImportPopupViewState` 정의 | 신규 타입: `io/application/import_popup_view_state.h` (신규 파일) | 신규 |

`ImportPopupPresenter.{h,cpp}`는 위 5개 public API만 호출하도록 축소되어, private 멤버 직접 접근이 0건이 된다.

#### 2.4.6 G2 `app.cpp` 분할 — 메서드 이관 표

`webassembly/src/app.cpp` (1,430 lines, `App::*` 34 메서드)를 shell 레이어의 7개 파일로 분할한다. **클래스 멤버 변경 없음** (`app.h` 시그니처 불변). 각 파일은 `App::` 구현체만 이동 수용한다.

| 이관 대상 파일 (신규) | 수용 `App::*` 메서드 범위 (대표) | 이관 사유 |
|---|---|---|
| `shell/presentation/layout/main_menu.cpp` | `renderMainMenuBar`, `renderFileMenu`, `renderEditMenu`, `renderViewMenu`, `renderHelpMenu` | ImGui 상단 메뉴 집합 |
| `shell/presentation/layout/window_layout.cpp` | `renderDockSpaceAndMenu`의 상단 ~330 lines 도킹 레이아웃 | 도킹 루트 |
| `shell/presentation/layout/side_panels.cpp` | 좌/우 사이드 패널 렌더링(`renderStructurePanel`, `renderMeasurementPanel` 등) | 사이드 도킹 구획 |
| `shell/presentation/layout/status_bar.cpp` | `renderStatusBar`, `renderProgressIndicator` | 하단 상태줄 |
| `shell/runtime/app_lifecycle.cpp` | `init`, `shutdown`, `reloadFromPersistence` | 라이프사이클 |
| `shell/runtime/app_frame_tick.cpp` | `render`, `tick`, `onFrameBegin`, `onFrameEnd` | 프레임 루프 |
| `shell/runtime/app_event_dispatch.cpp` | 키/마우스/리사이즈 이벤트 → gateway 분배 | 입력 분기 |

**최종 `app.cpp` 잔존 메서드**: `main()` 진입, `App` 생성자/소멸자, 최상위 서비스 컴포지션 호출 — 총 ≤ 15개, ≤ 400 lines.

**이관 후 CMake 갱신 대상**: `webassembly/cmake/modules/wb_shell.cmake`에 위 7개 소스 추가, `app.cpp`의 소스 엔트리는 유지.

#### 2.4.7 G4 `vtkActor*` 비-render 경계 제거 — 책임 이관 표

현재 `measurement/application/measurement_controller.cpp:43`이 `vtkActor* actor` 인자로 picking 결과를 수신한다. 이를 W3(signature 변경)과 W10(본체 흡수)로 분리 이관한다.

| 현재 상태 | 이관 후 | 이관 대상 파일 |
|---|---|---|
| `MeasurementController::HandleMeasurementClickByPicker(vtkActor* actor, double pickPos[3])` | `MeasurementController::HandleMeasurementClickByPick(const render::application::PickedAtomResult&)` | `measurement/application/measurement_controller.{h,cpp}` (W3 시그니처 교체) |
| `AtomsTemplate::HandleMeasurementClickByPicker(vtkActor*, double[3])` (facade 내 본체) | `render::application::AtomPickingService::PickAtAtomByActor(vtkActor*) -> std::optional<PickedAtomResult>` + MeasurementController가 결과 소비 | `render/application/atom_picking_service.{h,cpp}` (신규, W10) · `render/infrastructure/vtk_atom_picking_service.{h,cpp}` (신규, W10) |
| `PickedAtomResult` 구조체 정의 위치 | 신규 파일 `render/application/picked_atom_result.h` (`PickedAtomResult { int atomIndex; double position[3]; int groupIndex; }`) | 신규 파일 (W3) |

W3 완료 시점: `measurement` 모듈 헤더·구현에서 `<vtkActor.h>` / `vtkActor*` 참조 **0건**. W10 완료 시점: picking 본체가 `render/application/` + `render/infrastructure/`로 완전 이관되어 `AtomsTemplate` 쪽 원본 메서드가 orphan이 됨 → S6 슬라이스 작업이 `workspace/legacy` 측 본체를 삭제.

#### 2.4.8 이관 경로 요약 다이어그램

```text
workspace/legacy/atoms_template_facade.{h,cpp}          (삭제)
  ├─ S1 Measurement         ──▶ measurement/application/{measurement_controller, measurement_session_service}
  ├─ S2 Bond                ──▶ structure/application/bond_service
  │                                render/infrastructure/atoms/bond_group_vtk
  ├─ S3 Charge Density      ──▶ density/application/{charge_density_service, slice_viewer_service}
  │                                density/presentation/charge_density_view
  ├─ S4 Structure metadata  ──▶ structure/application/{structure_metadata_service, structure_lifecycle_service}
  ├─ S5 Grid visibility     ──▶ structure/application/{advanced_grid_visibility_service, visibility_service}
  ├─ S6 Picking/Tooltip     ──▶ render/application/atom_picking_service
  │                                render/infrastructure/vtk_atom_picking_service
  │                                structure/application/atom_selection_service
  │                                shell/presentation/atoms/atom_tooltip_renderer
  ├─ S7 Atoms UI            ──▶ shell/presentation/atoms/{builder_view, editor_view, advanced_view, template_window}
  └─ S8 File I/O            ──▶ io/application/{xsf_load_service, chgcar_load_service}

workspace/legacy/legacy_atoms_runtime.{h,cpp}           (삭제)
  └─ LegacyAtomsRuntime()   ──▶ shell/runtime/workbench_runtime (10 신규 getter)

{structure,measurement,density}/application/*_service_port.h           (삭제)
io/application/import_runtime_port.h                                   (삭제)
  └─ Port 가상 메서드       ──▶ 각 feature 서비스 native 메서드 (virtual 제거, 서비스가 소유)

{structure,measurement,density,io}/infrastructure/legacy/legacy_*_port.{h,cpp}   (삭제)
  └─ 어댑터 변환 로직       ──▶ 책임 소멸 (서비스가 domain 직접 소유)

io/application/import_entry_service.h::friend ImportPopupPresenter     (삭제)
  └─ 8 private 접근         ──▶ FileLoader 5 public API + ImportPopupViewState POD

app.cpp (1,430L, App* 34 method)                                       (축소)
  └─ 7 슬라이스             ──▶ shell/presentation/layout/{main_menu, window_layout, side_panels, status_bar}
                                shell/runtime/{app_lifecycle, app_frame_tick, app_event_dispatch}

measurement::controller(vtkActor*)                                     (교체)
  └─ W3 signature           ──▶ PickedAtomResult 기반 const 참조
  └─ W10 본체               ──▶ render/application/atom_picking_service
```

#### 2.4.9 이관 무결성 검증 포인트

각 작업 단위 완료 시점의 검증은 §5의 DoD와 §8 게이트 스크립트가 담당한다. 본 매핑은 다음 불변식을 강제한다.

1. **단일 수용 서비스 원칙**: 한 개 `AtomsTemplate::*` 메서드의 본체는 정확히 한 개 신규 서비스로 이관된다. 복수 서비스 호출이 필요한 경우 수용 서비스가 외부 협력자로 분해하되, 본체 소유 책임은 하나로 한정.
2. **Domain 단일 소유 원칙**: `structure::domain::AtomGroups` 등 공유 상태는 `StructureLifecycleService`가 단일 소유하고 나머지 서비스는 const 참조 또는 공개 API 경유로 접근.
3. **Port 미도입 원칙**: 본 이관 과정에서 신규 Port 인터페이스를 추가하지 않는다. 테스트 seam은 서비스 생성자 주입(DI)으로 해결.
4. **경로 존재성 원칙**: 표에 기재된 "신규" 경로는 해당 작업 단위(W5~W11) 완료 시점에 실제 파일로 존재해야 한다. W4 산출물 `slice_map.md`가 최종 경로 확정본이며, 본 §2.4 표는 그 초안 겸 DoD 근거가 된다.

## 3. 현재 상태 정량 측정 (2026-04-21 기준)

| 지표 | 현재 값 | 목표 값 | 근거 |
|---|---:|---:|---|
| `friend class` 출현 수(매크로 정의 제외) | 1 | 0 | `rg "friend class " webassembly/src` |
| `app.cpp` 라인 수 | 1,430 | ≤ 400 | `wc -l` |
| `app.cpp` 내 `App::*` 함수 정의 수 | 34 | ≤ 15 | grep |
| `AtomsTemplate` 클래스 선언 수 | 1 | 0 | `rg "class AtomsTemplate"` |
| `AtomsTemplate::` 심볼 정의·참조 수 | 190+ 구현 + 다수 호출 | 0 | grep |
| `LegacyAtomsRuntime(` 호출 수 | 33+ | 0 | grep |
| `#include .*legacy_atoms_runtime.h` | 33 | 0 | grep |
| 비 render/legacy `vtkActor` 참조 수 | 1 | 0 | grep |
| `webassembly/src/workspace/legacy/` | 존재 | 제거 | `Test-Path` |
| `webassembly/src/density/infrastructure/legacy/` | 존재 | 제거 | `Test-Path` |
| `webassembly/src/io/infrastructure/legacy/` | 존재 | 제거 | `Test-Path` |
| `webassembly/src/measurement/infrastructure/legacy/` | 존재 | 제거 | `Test-Path` |
| `webassembly/src/structure/infrastructure/legacy/` | 존재 | 제거 | `Test-Path` |
| `rg "legacy" webassembly/src --files-with-matches` 결과 경로 수 | ≥ 35 | 0 | grep |
| 4개 Port 인터페이스 파일 | 4 | 0 | `Test-Path` |

주의: "4개 Port 인터페이스" = `{structure, measurement, density}/application/*_service_port.h` 3종 + `io/application/import_runtime_port.h` 1종 (총 4개). rev.3 DoD는 이들 dissolve 후 파일 제거를 포함한다.

## 4. 작업 단위(Work Unit) 구성

### 4.1 작업 단위 요약

| W | 유형 | 제목 | 대상 | 선행 |
|---|---|---|---|---|
| W0 | 준비 | 안전장치 및 측정 베이스라인 | 공통 | - |
| W1 | 간극 | Import Popup Presenter friend 제거 | G1 | W0 |
| W2 | 간극 | `app.cpp` 분할 | G2 | W0 |
| W3 | 간극 | measurement 모듈 `vtkActor*` 경계 제거 | G4 | W0 |
| W4 | 해체 준비 | 슬라이스 지도 + 호출자 인벤토리 + 추출 템플릿 | G3, G3' | W0 |
| W5 | 해체 | 슬라이스 S1: Measurement 추출 | G3 | W4 |
| W6 | 해체 | 슬라이스 S2: Bond 추출 | G3 | W4 |
| W7 | 해체 | 슬라이스 S3: Charge Density + Slice Viewer 추출 | G3 | W4 |
| W8 | 해체 | 슬라이스 S4: Structure metadata & lifecycle 추출 | G3 | W4 |
| W9 | 해체 | 슬라이스 S5/S8: Visibility + File I/O | G3 | W4 |
| W10 | 해체 | 슬라이스 S6: Picking/Hover/Tooltip (W3 흡수) | G3, G4 | W3, W4 |
| W11 | 해체 | 슬라이스 S7: AtomsTemplate UI 렌더링 | G3 | W4 |
| **W12** | **해체 마감** | **Feature Service Internalization — 4개 Port 인터페이스 dissolve + 4개 Legacy 어댑터 제거** | **G3, G3'** | W5~W11 |
| W13 | 해체 마감 | 33개 include 사이트 마이그레이션 및 `legacy_atoms_runtime.h` 폐지 | G3 | W12 |
| W14 | 해체 마감 | **5개 `legacy/` 폴더 전체 물리 삭제** (12 파일) | G3, G3' | W13 |
| W15 | 검증 | Phase 19 게이트/CI 추가 및 Phase 17·18 게이트 호환 검증 | 공통 | W1~W14 |
| W16 | 종료 | 통합 검증 및 종료 문서 패키지 | 공통 | W15 |

### 4.2 병렬성

- W1, W2, W3, W4는 W0 완료 후 상호 독립 — 병렬 PR 가능
- W5~W11은 W4 완료 후 슬라이스별 상호 독립 — 병렬 PR 가능
- **W12(Port dissolution)는 W5~W11 전부 merge 후에 성립** — 서비스 내용이 자체 모듈로 이관된 후에야 포트 인터페이스를 제거할 수 있음
- W13은 W12 성공 후에만 실행
- W14는 W13 성공 후에만 실행 (5개 폴더 동시 삭제는 단일 PR 권장)
- W15, W16은 순차

## 5. 작업 단위 상세

### 5.1 W0 — 안전장치 및 측정 베이스라인

(rev.2와 동일, 측정 지표만 §3 15개 항목으로 확장)

**산출물**

1. `docs/refactoring/phase19/logs/baseline_metrics_latest.txt` — 본 계획서 §3 15개 지표 전부 측정
2. `docs/refactoring/phase19/logs/smoke_phase19_before.txt`
3. `docs/refactoring/phase19/logs/build_phase19_before.txt`
4. `docs/refactoring/phase19/logs/atoms_template_method_inventory.csv` — 190개 메서드의 슬라이스 할당표
5. `docs/refactoring/phase19/logs/legacy_ports_inventory.csv` — 4개 Port 인터페이스의 메서드 서명 및 대응 어댑터·호출자 목록

### 5.2 W1 — Import Popup Presenter friend 제거 (G1)

이관 대상은 **§2.4.5 표**에 고정. 본 W의 DoD는 `ImportPopupPresenter`가 해당 5개 public API + `ImportPopupViewState` POD만 사용하여 `friend` 선언 없이 컴파일·동작하는 것.

### 5.3 W2 — `app.cpp` 분할 (G2)

이관 대상 7개 파일과 각각의 수용 메서드군은 **§2.4.6 표**에 고정. 본 W의 DoD는 `app.cpp` ≤ 400 lines, `App::*` ≤ 15, `app.h` 시그니처 diff = 0.

### 5.4 W3 — measurement 모듈 `vtkActor*` 경계 제거 (G4)

시그니처 교체와 `PickedAtomResult` 신규 타입 위치는 **§2.4.7 표**에 고정. 본체 이관은 W10(S6)에서 수행.

### 5.5 W4 — 슬라이스 지도 및 추출 템플릿

rev.2와 동일. 추가로 다음 산출물을 포함한다.

**Port dissolution 매핑 추가 (rev.3 신규)**

`legacy_ports_inventory.csv`는 다음 7열로 구성한다.

| 열 | 설명 |
|---|---|
| `port` | `StructureServicePort` / `MeasurementServicePort` / `DensityServicePort` / `ImportRuntimePort` |
| `method` | 포트 가상 메서드 서명 |
| `legacyImplFile` | 현재 구현 파일 (예: `structure/infrastructure/legacy/legacy_structure_service_port.cpp`) |
| `targetService` | dissolve 후 책임을 가질 서비스 (예: `structure::application::StructureService`) |
| `targetMethod` | 서비스의 내부 public 메서드 서명 |
| `currentCallers` | 현재 포트 메서드를 호출하는 위치 |
| `migrationPlan` | dissolve 시 caller가 참조할 신규 API |

**예시 행 (MeasurementServicePort):**

| port | method | legacyImpl | targetService | targetMethod | caller | migrationPlan |
|---|---|---|---|---|---|---|
| `MeasurementServicePort` | `GetMode()` | `legacy_measurement_service_port.cpp:49` | `MeasurementService` | `GetMode()` | (내부) | Port 제거, 서비스 native |
| `MeasurementServicePort` | `EnterMode(mode)` | `legacy_measurement_service_port.cpp:57` | `MeasurementService` | `EnterMode(mode)` | shell/measurement UI | 동일 |

### 5.6 W5~W11 — 슬라이스 추출 (S1~S8)

슬라이스별 "이관 대상 파일 / 수용 서비스 / 도메인 소유"는 **§2.4.1 표**에 고정되어 있다. 본 절은 작업 운영 관점의 절차만 기술하며, 경로·심볼 변경 시에는 반드시 §2.4.1을 먼저 갱신한다. 각 W의 DoD는 rev.2와 동일하되, "신규 파일 경로"는 §2.4.1 표의 해당 행과 일치해야 한다.

### 5.7 W12 — Feature Service Internalization (4개 Port dissolve + Legacy 어댑터 제거)

**목표**

Phase 9~14 기간 동안 도입된 Port/Adapter 분리는 `AtomsTemplate` 의존을 격리하기 위한 임시 seam이었다. Phase 19 슬라이스 추출(W5~W11) 완료 시점에는 서비스가 domain 상태를 직접 소유하므로 Port 인터페이스는 추가 가치 없는 간접 계층이 된다. W12에서 이를 정리한다.

**Port 처리 방침**

각 Port에 대해 다음 중 하나를 택한다. 기본 권고는 (A) Port Dissolve.

| 방침 | 설명 | 권고 대상 |
|---|---|---|
| (A) **Port Dissolve** | Port 인터페이스 제거. 서비스 클래스가 domain을 직접 소유하고 메서드를 native 구현. | 4개 모두 |
| (B) Native Adapter | Port 인터페이스 유지. `infrastructure/native/` 하위에 `Native*ServicePort` 신규 구현. | 플러그인 지원 같은 복수 구현이 필요한 경우만 (현재 해당 없음) |

본 계획은 **전 포트 (A) Dissolve**로 진행한다. 복수 구현이 필요한 미래 요구는 해당 시점에 다시 설계한다.

**세부 작업 (W12-P1 ~ W12-P4)**

#### 5.7.1 W12-P1: `StructureServicePort` Dissolve

- 제거 대상
  - `webassembly/src/structure/application/structure_service_port.h` (파일 삭제)
  - `webassembly/src/structure/infrastructure/legacy/legacy_structure_service_port.{h,cpp}`
- 변경 대상
  - `structure/application/structure_service.{h,cpp}` — 생성자에서 `StructureServicePort&` 인자 제거. 내부는 W8(S4) 추출로 확보한 `StructureMetadataService` 등을 직접 사용.
  - `shell/runtime/workbench_runtime.cpp:17, 50-53, 95-97` — `runtimeStructureFeature()` 내부에서 port 제거, 서비스 직접 생성
  - `structure/application/structure_service.cpp:7` 정적 port 제거
- 결과: `structure/infrastructure/legacy/` 폴더가 비게 됨(W14에서 삭제)

#### 5.7.2 W12-P2: `MeasurementServicePort` Dissolve

- 제거 대상
  - `webassembly/src/measurement/application/measurement_service_port.h`
  - `webassembly/src/measurement/infrastructure/legacy/legacy_measurement_service_port.{h,cpp}`
- 변경 대상
  - `measurement/application/measurement_service.{h,cpp}` — 생성자 시그니처 변경
  - `shell/runtime/workbench_runtime.cpp:19, 56-58, 99-102`
  - `measurement/application/measurement_service.cpp:7`
- 결과: `measurement/infrastructure/legacy/` 폴더가 비게 됨

#### 5.7.3 W12-P3: `DensityServicePort` Dissolve

- 제거 대상
  - `webassembly/src/density/application/density_service_port.h`
  - `webassembly/src/density/infrastructure/legacy/legacy_density_service_port.{h,cpp}`
- 변경 대상
  - `density/application/density_service.{h,cpp}`
  - `shell/runtime/workbench_runtime.cpp:21, 61-65`
  - `density/application/density_service.cpp:7`
- 결과: `density/infrastructure/legacy/` 폴더가 비게 됨

#### 5.7.4 W12-P4: `ImportRuntimePort` Dissolve

- 제거 대상
  - `webassembly/src/io/application/import_runtime_port.h`
  - `webassembly/src/io/infrastructure/legacy/legacy_import_runtime_port.{h,cpp}`
- 변경 대상
  - `io/application/import_entry_service.{h,cpp}` — `ImportRuntimePort&` 의존성 제거. 대신 W8(S4), W7(S3) 슬라이스에서 추출된 `StructureMetadataService`, `ChargeDensityService`, `StructureLifecycleService` 등을 `WorkbenchRuntime` 경유로 직접 호출
  - `io/application/parser_worker_service.{h,cpp}` — 동일 (필요 시)
  - `io/application/import_apply_service.{h,cpp}` — 동일 (필요 시)
  - `io/application/import_workflow_service.{h,cpp}` — 동일 (필요 시)
  - 주의: `LegacyImportRuntimePort::LegacyParent()`는 `AtomsTemplate*`을 반환 — W5~W11 모든 슬라이스 완료 후에만 제거 가능
- 결과: `io/infrastructure/legacy/` 폴더가 비게 됨

**W12 전체 DoD**

1. 4개 Port 헤더(`*_service_port.h`, `import_runtime_port.h`) 전부 삭제
2. 4개 Legacy 어댑터 (`legacy_*_service_port.{h,cpp}`, `legacy_import_runtime_port.{h,cpp}`) 전부 삭제
3. `rg "ServicePort\b|ImportRuntimePort" webassembly/src` 결과 0건
4. `rg "infrastructure::legacy" webassembly/src` 결과 0건
5. `shell/runtime/workbench_runtime.cpp`가 service를 직접 생성 (Port 없이)
6. `npm run build-wasm:release` / `test:cpp` / `test:smoke` PASS
7. 4개 `infrastructure/legacy/` 폴더가 비어있음 (파일 삭제 완료, 폴더 삭제는 W14)

### 5.8 W13 — 33개 include 사이트 마이그레이션 + `legacy_atoms_runtime.h` 폐지

**전제**: W12 DoD 충족 (서비스가 self-contained 상태)

**작업**

각 호출자 파일의 `#include ".../workspace/legacy/legacy_atoms_runtime.h"` 및 `workspace::legacy::LegacyAtomsRuntime()` 호출을 feature 서비스 API로 치환 (rev.2와 동일).

**매핑 요약** (상세는 §2.4.1 슬라이스 × §2.4.2 getter 교차표 참조)

| 호출자 그룹 | 대체 방향 | 대응 getter (§2.4.2) |
|---|---|---|
| `structure/domain/atoms/*.cpp` | `structure/application/{bond_service,structure_metadata_service,structure_lifecycle_service}.h` | `bondService()`, `structureMetadataService()` |
| `structure/application/*.cpp` | self-contained (서비스 내부에서 동료 서비스 직접 주입) | — |
| `shell/presentation/atoms/*.cpp` | `shell/presentation/atoms/{builder_view,editor_view}.h` 또는 `WorkbenchRuntime` | `atomTooltipRenderer()`, `atomSelectionService()` |
| `shell/application/workbench_controller.cpp` | `WorkbenchRuntime` | 전 getter |
| `render/*/`, `mesh/presentation/*.cpp`, `density/presentation/*.cpp` | 각 모듈 feature 서비스 | `atomPickingService()`, `chargeDensityService()`, `sliceViewerService()` |
| `io/infrastructure/file_io_manager.cpp` | `io/application/xsf_load_service.h` / `chgcar_load_service.h` | `xsfLoadService()`, `chgcarLoadService()` |
| `app.cpp` | `shell/runtime/workbench_runtime.h` | 전 getter |

**DoD**

1. `rg "#include.*legacy_atoms_runtime" webassembly/src` 결과 0건
2. `rg "LegacyAtomsRuntime\(" webassembly/src` 결과 0건 (정의 파일 제외)
3. `rg "AtomsTemplate" webassembly/src --glob '!workspace/legacy/**'` 결과 0건
4. build/smoke/cpp 테스트 PASS

### 5.9 W14 — 5개 `legacy/` 폴더 물리 삭제

**전제**: W12 + W13 DoD 충족 — 5개 폴더가 모두 orphan 상태

**작업**

```text
# 파일 삭제 (이미 W12에서 내용 비워짐; 확정 삭제)
rm webassembly/src/workspace/legacy/atoms_template_facade.h
rm webassembly/src/workspace/legacy/atoms_template_facade.cpp
rm webassembly/src/workspace/legacy/legacy_atoms_runtime.h
rm webassembly/src/workspace/legacy/legacy_atoms_runtime.cpp

rm webassembly/src/structure/infrastructure/legacy/legacy_structure_service_port.h
rm webassembly/src/structure/infrastructure/legacy/legacy_structure_service_port.cpp

rm webassembly/src/measurement/infrastructure/legacy/legacy_measurement_service_port.h
rm webassembly/src/measurement/infrastructure/legacy/legacy_measurement_service_port.cpp

rm webassembly/src/density/infrastructure/legacy/legacy_density_service_port.h
rm webassembly/src/density/infrastructure/legacy/legacy_density_service_port.cpp

rm webassembly/src/io/infrastructure/legacy/legacy_import_runtime_port.h
rm webassembly/src/io/infrastructure/legacy/legacy_import_runtime_port.cpp

# 폴더 삭제 (빈 경우에만)
rmdir webassembly/src/workspace/legacy
rmdir webassembly/src/structure/infrastructure/legacy
rmdir webassembly/src/measurement/infrastructure/legacy
rmdir webassembly/src/density/infrastructure/legacy
rmdir webassembly/src/io/infrastructure/legacy
```

**CMake 영향**

1. `webassembly/cmake/modules/wb_workspace.cmake` — `workspace/legacy/*.cpp` 소스 엔트리 제거
2. `webassembly/cmake/modules/wb_structure.cmake` — `structure/infrastructure/legacy/*.cpp` 엔트리 제거
3. `webassembly/cmake/modules/wb_measurement.cmake` — 동일
4. `webassembly/cmake/modules/wb_density.cmake` — 동일
5. `webassembly/cmake/modules/wb_io.cmake` — 동일
6. `webassembly/CMakeLists.txt`의 `WB_MODULE_TARGETS` = 10 **불변**

**부모 폴더 검토**

- `webassembly/src/workspace/` — `workspace/application/`, `workspace/domain/` 등이 있다면 유지. 비어 있다면 `wb_workspace` 모듈 자체 재검토가 필요하지만, 본 Phase 19 범위 밖 (Phase 20에서 처리).
- `webassembly/src/{density,io,measurement,structure}/infrastructure/` — 각 모듈의 기존 non-legacy infrastructure 파일이 있으므로 폴더 자체는 유지.

**DoD**

1. 12개 파일 전부 git에서 삭제 (`git status`에 deleted)
2. 5개 legacy 폴더 전부 `Test-Path` 결과 `False`
3. `rg -l "legacy" webassembly/src` 결과 0건 (문서·주석 포함)
4. `npm run build-wasm:release` PASS
5. 10 모듈 타깃 그래프 불변 확인 (`check_phase17_target_graph_final.ps1` PASS)

### 5.10 W15 — Phase 19 게이트/CI 추가

**신규 게이트**

| 스크립트 | 대상 W | 검사 내용 |
|---|---|---|
| `scripts/refactoring/check_phase19_friend_class_zero.ps1` | W1 | `friend class ` 출현 = 0 |
| `scripts/refactoring/check_phase19_app_cpp_split.ps1` | W2 | `app.cpp` 크기/분할 |
| `scripts/refactoring/check_phase19_actor_outside_render.ps1` | W3/W10 | 비 render/legacy `vtkActor` = 0 |
| `scripts/refactoring/check_phase19_slice_S1_complete.ps1` ~ `S7_complete.ps1` | W5~W11 | 슬라이스별 메서드 본체가 legacy에 없음 |
| `scripts/refactoring/check_phase19_ports_dissolved.ps1` | W12 | 4개 Port 인터페이스·4개 Legacy 어댑터 파일 부재 |
| `scripts/refactoring/check_phase19_legacy_runtime_zero.ps1` | W13 | `LegacyAtomsRuntime(`, `legacy_atoms_runtime.h` 참조 = 0 |
| `scripts/refactoring/check_phase19_atoms_template_class_zero.ps1` | W13 | `class AtomsTemplate` = 0 |
| `scripts/refactoring/check_phase19_all_legacy_folders_removed.ps1` | W14 | **5개 legacy 폴더 전부 부재** |
| `scripts/refactoring/check_phase19_no_legacy_tokens.ps1` | W14 | `rg "legacy" webassembly/src` = 0건 (주석·파일명 포함) |
| `scripts/refactoring/check_phase19_final_closure.ps1` | 종합 | 위 16개 + Phase 17/18 게이트 동시 PASS |

**`package.json` 진입점**

```text
"check:phase19:friend-class-zero"
"check:phase19:app-cpp-split"
"check:phase19:actor-outside-render"
"check:phase19:slice-s1" ... "check:phase19:slice-s7"
"check:phase19:ports-dissolved"
"check:phase19:legacy-runtime-zero"
"check:phase19:atoms-template-class-zero"
"check:phase19:all-legacy-folders-removed"
"check:phase19:no-legacy-tokens"
"check:phase19:final-closure"
"test:phase19"
```

**기존 게이트 호환성**

- Phase 17 `check_phase17_target_graph_final.ps1`: 10 모듈 유지 — W14 이후 PASS 재확인
- Phase 18 `check_phase18_facade_include_zero.ps1`: facade 파일이 삭제되므로 자동 PASS
- Phase 18 `check_phase18_atoms_instance_zero.ps1`: 변경 없음 — 유지
- Phase 18 `check_phase18_legacy_singleton_zero.ps1`: `LegacyAtomsRuntime()`이 사라지므로 강화된 의미로 PASS

### 5.11 W16 — 종료 문서 및 통합 검증

**실행 항목**

| 명령 | 증빙 |
|---|---|
| `npm run build-wasm:release` | `phase19/logs/build_phase19_latest.txt` |
| `npm run test:cpp` | `phase19/logs/test_cpp_phase19_latest.txt` |
| `npm run test:smoke` | `phase19/logs/smoke_phase19_latest.txt` |
| `npm run test:phase19` | `phase19/logs/test_phase19_latest.txt` |
| `npm run check:phase19:final-closure` | `phase19/logs/phase19_final_closure_gate_latest.txt` |
| Phase 17+18+19 suite | `phase19/logs/phase17_18_19_gate_suite_latest.txt` |

**문서**

1. `docs/refactoring/phase19/dependency_gate_report.md`
2. `docs/refactoring/phase19/go_no_go_phase19_close.md`
3. `docs/refactoring/phase19/slice_map.md` (W4 → W16 확정)
4. `docs/refactoring/phase19/legacy_dismantle_timeline.md`
5. `docs/refactoring/phase19/port_dissolution_report.md` (rev.3 신규) — 4개 Port dissolve 전후 비교
6. `docs/refactoring/refactoring_result_report_phase19_legacy_dismantle_260XXX.md`
7. `docs/refactoring/phase19/logs/manual_ui_regression_phase19_latest.md`

**DoD**: §9 참조.

## 6. 리스크 및 완화

rev.2의 R1~R8 유지. rev.3에서 추가되는 리스크:

| ID | 리스크 | 심각도 | 확률 | 완화 전략 |
|---|---|---|---|---|
| R9 | Port dissolve 중 서비스 인스턴스화 경로에 사이클 발생 | 중 | 중 | W12는 서비스별 독립 PR로 분리하고, `workbench_runtime.cpp`에서 한 서비스씩 Port 제거·재주입 |
| R10 | `StructureService`/`MeasurementService`/`DensityService`가 동일 domain 상태를 공유해 port 없이 직접 접근 시 경쟁 | 중 | 중 | W4에서 공유 상태(`structure::domain::atomGroups` 등)의 단일 소유자 결정(예: `StructureLifecycleService`), 나머지는 const 참조/공개 API 경유 |
| R11 | Port dissolve 후 재도입 필요성이 확인되면 rollback 비용 큼 | 낮 | 낮 | W16 완료 후 2주 soak 기간 중 재도입 모니터링; 현재는 복수 구현 요구가 전무 |
| R12 | `LegacyImportRuntimePort::LegacyParent()` → `AtomsTemplate*` 반환 의존이 있는 호출자가 있을 경우 W12-P4 블로킹 | 높음 | 중 | W4 단계에서 `LegacyParent()` 호출자 grep, 대체 API(예: `StructureMetadataService`, `ChargeDensityService` 직접 접근)로 사전 치환 |

## 7. 일정 및 의존 관계

| 주차 | 마일스톤 |
|---|---|
| W1주 | W0 + W1/W2/W3 병렬 |
| W2주 | W4 슬라이스 지도 + port 인벤토리 |
| W3~W4주 | W5 (S1) |
| W4~W5주 | W6 + W7 병렬 |
| W5~W6주 | W8 + W9 병렬 |
| W6~W7주 | W10 |
| W7~W8주 | W11 |
| W8~W9주 | **W12 (Port dissolve 4종 순차 PR)** |
| W9~W10주 | W13 (include 마이그레이션) |
| W10주 | W14 (폴더·파일 삭제 단일 PR) |
| W11주 | W15 + W16 |

전체 실행은 8~12주 규모로 추정된다.

## 8. 게이트 스크립트 사양(발췌)

### 8.1 `check_phase19_ports_dissolved.ps1` (rev.3 신규)

- 검사 1: 다음 4개 파일 부재
  - `webassembly/src/structure/application/structure_service_port.h`
  - `webassembly/src/measurement/application/measurement_service_port.h`
  - `webassembly/src/density/application/density_service_port.h`
  - `webassembly/src/io/application/import_runtime_port.h`
- 검사 2: 다음 8개 파일 부재
  - `webassembly/src/structure/infrastructure/legacy/legacy_structure_service_port.{h,cpp}`
  - `webassembly/src/measurement/infrastructure/legacy/legacy_measurement_service_port.{h,cpp}`
  - `webassembly/src/density/infrastructure/legacy/legacy_density_service_port.{h,cpp}`
  - `webassembly/src/io/infrastructure/legacy/legacy_import_runtime_port.{h,cpp}`
- 검사 3: `rg "ServicePort\b|ImportRuntimePort\b" webassembly/src` → 0건
- 검사 4: `rg "infrastructure::legacy" webassembly/src` → 0건

### 8.2 `check_phase19_all_legacy_folders_removed.ps1` (rev.3 신규)

- 검사: 다음 5개 경로 전부 `Test-Path` 결과 `$false`
  - `webassembly/src/workspace/legacy`
  - `webassembly/src/structure/infrastructure/legacy`
  - `webassembly/src/measurement/infrastructure/legacy`
  - `webassembly/src/density/infrastructure/legacy`
  - `webassembly/src/io/infrastructure/legacy`

### 8.3 `check_phase19_no_legacy_tokens.ps1` (rev.3 신규)

- 입력: `webassembly/src/**/*.{h,hpp,cpp,cmake,txt}`
- 검사: `rg -i "\blegacy\b" webassembly/src` → 0건
- 예외 없음. 주석·파일명·경로에 "legacy" 토큰이 남아 있어도 FAIL
- 근거: Phase 19 봉인 이후에는 "legacy"라는 표현 자체가 코드베이스에 부정확한 단서가 됨

(rev.2에서 정의한 나머지 게이트 사양은 유지)

## 9. 완료 기준 (DoD)

**간극 해소**

1. G1: `friend class` = 0
2. G2: `app.cpp` ≤ 400 lines, `App::*` ≤ 15, shell 신규 파일 7개 존재
3. G3: `workspace/legacy/` 폴더 제거, `AtomsTemplate`/`LegacyAtomsRuntime` 심볼 0, `#include .*legacy_atoms_runtime.h` 0
4. G3': 4개 `{module}/infrastructure/legacy/` 폴더 제거, 4개 Port 인터페이스 파일 제거, 4개 Legacy 어댑터 파일 제거
5. G4: 비 render/legacy `vtkActor` = 0

**빌드·테스트**

6. `npm run build-wasm:release` PASS
7. `npm run test:cpp` PASS
8. `npm run test:smoke` PASS
9. `npm run test:phase19` PASS

**게이트**

10. Phase 19 게이트 16종 전부 PASS
11. Phase 17+Phase 18+Phase 19 종합 suite PASS

**문서**

12. `refactoring_result_report_phase19_legacy_dismantle_260XXX.md` 발행
13. `slice_map.md`, `legacy_dismantle_timeline.md`, `port_dissolution_report.md` 발행
14. 수동 UI 회귀 시나리오 매트릭스 전부 PASS 로그 첨부

**구조 정합성**

15. `rg -l "legacy" webassembly/src` → 0건
16. `rg "ServicePort\b|ImportRuntimePort\b" webassembly/src` → 0건
17. 10 모듈 타깃 그래프 불변

## 10. 부록

### A. 영향 파일 인벤토리 요약 (rev.3 갱신)

| W | 주요 영향 | 신규/수정/삭제 |
|---|---|---|
| W0 | logs, CSV | 신규 5 |
| W1 | import_entry_service/import_popup 파일 | 수정 3 + 신규 1 |
| W2 | app.cpp + shell 7개 파일 + wb_shell.cmake | 수정 2 + 신규 7 |
| W3 | picked_atom_result.h 외 | 신규 1 + 수정 4 |
| W4 | slice_map.md + legacy_ports_inventory.csv + WorkbenchRuntime getter 10종 | 신규 + 수정 |
| W5 | measurement/application/*.cpp 4~6 + measurement/infrastructure/*.cpp 1~2 | 신규 ~7 + 수정 1 |
| W6 | structure/application/bond_service + render/infrastructure/atoms/bond_group_vtk | 신규 4 |
| W7 | density/application/*.cpp 3 + density/presentation/*.cpp 2 | 신규 5 + 수정 1 |
| W8 | structure/application/structure_metadata_service + lifecycle 확장 | 신규 2 + 수정 1 |
| W9 | structure/application/visibility_service 확장 + io/application/*.cpp 2 + structure_reset_service | 신규 ~6 + 수정 1 |
| W10 | render/application/atom_picking_service + vtk_atom_picking_service + structure/application/atom_selection_service + shell/presentation/atoms/atom_tooltip_renderer | 신규 8 |
| W11 | shell/presentation/atoms/{builder_view,editor_view,advanced_view,template_window} | 신규 8 |
| **W12** | **4개 Port 헤더 삭제(4) + 4개 Legacy 어댑터 삭제(8) + 3개 application/*_service.cpp 수정 + 1개 io/application/import_entry_service.cpp 수정 + workbench_runtime.cpp 수정** | **삭제 12 + 수정 ≥5** |
| W13 | 33개 호출자 include 교체 | 수정 33 |
| W14 | 5개 폴더 물리 제거 + 5개 CMake 모듈 파일 수정 | 삭제 12 파일 + 수정 5 cmake |
| W15 | scripts/refactoring/check_phase19_*.ps1 16종 + package.json | 신규 16 + 수정 1 |
| W16 | phase19 결과 문서 7종 + 로그 6종 | 신규 ~13 |

### B. 호출자 재검증 체크리스트 (rev.3 갱신)

W12 완료 후:

```text
rg "ServicePort\b|ImportRuntimePort\b" webassembly/src   → 0건
rg "infrastructure::legacy" webassembly/src              → 0건
rg "LegacyStructureServicePort|LegacyMeasurementServicePort|LegacyDensityServicePort|LegacyImportRuntimePort" webassembly/src  → 0건
```

W13 완료 후:

```text
rg "#include.*legacy_atoms_runtime" webassembly/src   → 0건
rg "LegacyAtomsRuntime\(" webassembly/src             → 0건
rg "AtomsTemplate" webassembly/src --glob '!workspace/legacy/**'   → 0건
```

W14 완료 후:

```text
Test-Path webassembly/src/workspace/legacy                       → False
Test-Path webassembly/src/structure/infrastructure/legacy        → False
Test-Path webassembly/src/measurement/infrastructure/legacy      → False
Test-Path webassembly/src/density/infrastructure/legacy          → False
Test-Path webassembly/src/io/infrastructure/legacy               → False
rg -i "\blegacy\b" webassembly/src                               → 0건
rg "class AtomsTemplate\b" webassembly/src                       → 0건
```

### C. 참고 기존 자산

- `docs/refactoring/phase18/dependency_gate_report.md` — 게이트 문서 템플릿
- `docs/refactoring/phase18/go_no_go_phase18_close.md` — go/no-go 문서 템플릿
- `scripts/refactoring/check_phase18_final_closure.ps1` — 종합 게이트 패턴
- `webassembly/cmake/modules/wb_shell.cmake` / `wb_render.cmake` / `wb_structure.cmake` / `wb_measurement.cmake` / `wb_density.cmake` / `wb_io.cmake` / `wb_workspace.cmake` — 대상 모듈 빌드 설정

## 11. 최종 판정 기준 요약

| 항목 | 현재(Phase 18 봉인) | Phase 19 봉인 목표 |
|---|---|---|
| `friend class` | 1 | 0 |
| `app.cpp` 라인 / 메서드 | 1,430 / 34 | ≤ 400 / ≤ 15 |
| 신설 shell/presentation 파일 | 0/7 | 7/7 |
| 비 render/legacy `vtkActor` | 1 | 0 |
| `workspace/legacy/` 폴더 | 존재 | **제거** |
| `density/infrastructure/legacy/` | 존재 | **제거** |
| `io/infrastructure/legacy/` | 존재 | **제거** |
| `measurement/infrastructure/legacy/` | 존재 | **제거** |
| `structure/infrastructure/legacy/` | 존재 | **제거** |
| `AtomsTemplate` 클래스 | 1 | **0** |
| `LegacyAtomsRuntime()` 심볼 | 1 | **0** |
| Legacy 포트 어댑터 4종 | 존재 | **제거** |
| Port 인터페이스 4종 | 존재 | **dissolve(파일 제거)** |
| `#include legacy_atoms_runtime.h` 사이트 | 33 | **0** |
| `rg "legacy" webassembly/src` 결과 | 다수 | **0** |
| 10 모듈 타깃 그래프 | 유지 | 유지 |

Phase 19 종료 시점에 위 지표들이 "봉인 목표" 열과 일치하면, 독립 검증 보고서의 G1~G4 + rev.3 신규 G3'가 전부 해소되고 Phase 0~19 프로그램이 **legacy-free 상태로 최종 봉인**된다.
