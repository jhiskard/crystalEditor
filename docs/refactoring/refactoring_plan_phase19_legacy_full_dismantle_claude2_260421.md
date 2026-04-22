# Phase 19 계획서: Legacy 완전 해체 및 후속 권고 이행

작성일: `2026-04-21 (KST)`
작성자: 코드 리팩토링 검증/계획 세션
기준 문서:
- `docs/refactoring/refactoring_verification_report_independent_260420.md` (5장 리스크 및 후속 과제)
- `docs/refactoring/refactoring_plan_architecture_completion_phase13plus_260406.md` (4.1~4.4 정책)
- `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md` (3.1~3.7 목표 아키텍처)
- `docs/refactoring/phase18/refactoring_phase18_phase17_limit_closure_execution_plan_260416.md` (Phase 18 종료 전략)

대상 범위:
- `webassembly/src/**/legacy/**` (모든 legacy 폴더/파일)
- `webassembly/src/workspace/legacy/atoms_template_facade.{h,cpp}` (7,293 LOC)
- `webassembly/src/workspace/legacy/legacy_atoms_runtime.{h,cpp}`
- `webassembly/src/render/application/legacy_viewer_facade.{h,cpp}`
- `webassembly/src/{structure,measurement,density,io}/infrastructure/legacy/`
- `webassembly/src/io/application/import_entry_service.h` (friend class)
- `webassembly/src/measurement/application/measurement_controller.cpp` (VTK 토큰 제거)
- `webassembly/src/app.cpp` + `shell/presentation/` (main_menu/window_layout 분리)
- `webassembly/src/{mesh/domain,shell/presentation/font,render/presentation/viewer_window}*` (singleton 제거)
- `scripts/refactoring/` (신규 Phase 19 게이트)
- `docs/refactoring/phase19/**`

진행 상태: `계획 수립`

---

## 0. 배경과 목적

독립 검증 보고서(`refactoring_verification_report_independent_260420.md`) 5장은 Phase 0~18 리팩토링 프로그램이 **"현실적 완결 상태"** 에 도달했다고 판정하면서도, DoD 11개 중 `1개 미달성 / 3개 부분 달성 / 7개 달성`의 격차를 명시했다. 특히 다음 **잠재 리스크 4건 + 권고 사항 5건**이 식별되었다.

### 0.1 잠재 리스크 (검증보고서 5.1)

| # | 내용 | 정량값 | 현재 전략 | Phase 19 대응 |
|---|---|---:|---|---|
| R1 | `workspace/legacy/atoms_template_facade.cpp` god-object 유지 | 5,808 LOC | Phase 18 이후 "격리 유지" | **완전 해체** |
| R2 | `render/application/legacy_viewer_facade.*` 잔존 | 24 LOC | composition root 한정 유지 | **제거** |
| R3 | `friend class` 1건 잔존 (`import_entry_service.h:76`) | 1 | 게이트 미감지 | **해소** |
| R4 | `app.cpp`와 Phase 11 DoD 간 간격 (1,430 LOC) | 1,430 LOC | `main_menu.cpp`/`window_layout.cpp` 미생성 | **재분해** |

### 0.2 권고 사항 (검증보고서 5.2)

| 우선순위 | 항목 | Phase 19 WBS |
|---|---|---|
| High | `friend class` 1건 해소 또는 allowlist 문서화 | W1 |
| Medium | `app.cpp` 재분해 (main_menu/window_layout) | W7 |
| Medium | `measurement_controller.cpp` VTK 토큰 제거 | W2 |
| Low | `atoms_template_facade.cpp` 점진적 축소 로드맵 | W5 (완전 해체로 승격) |
| Low | `VtkViewer`, `MeshManager`, `FontManager` singleton 장기 제거 | W6 |

### 0.3 Phase 19의 목적

1. **webassembly/src 하위 모든 `legacy/` 폴더/파일 해체** — 검증보고서가 allowlist 전략으로 관리하던 "legacy 격리"를 **물리적 소거**로 전환한다.
2. **검증보고서 5.2 권고 사항 5건 전부 이행** — DoD 11개 전량 달성 상태로 종료한다.
3. **신규 Phase 19 게이트로 최종 봉인** — 이후 legacy 재유입을 구조적으로 차단한다.

본 계획은 "Phase 18이 legacy 격리를 완결했다"는 판정을 존중하되, 그 이후의 자연스러운 연장선으로 **격리 → 흡수/승격 → 소거** 단계를 수행하여 `webassembly/src/**/legacy/**` 경로를 0으로 만드는 것을 **비가역 종료 기준**으로 삼는다.

---

## 1. 준수 기준 (정책/아키텍처 추적)

| 기준 | 적용 방식 (Phase 19 맥락) |
|---|---|
| 3.1 핵심 원칙 | composition root(`WorkbenchRuntime`) 외부의 `::Instance()`/`DECLARE_SINGLETON`/`friend class` 전면 금지 |
| 3.2 목표 모듈 경계 | 모든 feature 모듈의 `infrastructure/legacy/`가 소거되고, 동일 책임은 `infrastructure/` 또는 `application/` 기본 경로로 승격 |
| 3.3 목표 디렉터리 레이아웃 | `webassembly/src/**/legacy/**` 경로 0, `legacy_*_port.*` 명명 0, `_facade` 명명은 Render 포트 내부 포함 모두 재명명 |
| 3.4 runtime 구성 | `AtomsTemplate`은 더 이상 단일 파일 facade가 아닌, 분해된 도메인 서비스/컨트롤러 집합으로 `WorkbenchRuntime`이 조립 |
| 3.5 상태 소유권 | `AtomsTemplate`이 보유하던 멤버 상태는 각 feature의 Repository/Store에 귀속되어야 하며, `LegacyAtomsRuntime()`의 정적 인스턴스가 최종 소거 |
| 3.6 포트/어댑터 계약 | feature service들은 `legacy adapter` 경유가 아닌, 도메인 서비스 직접 호출로 재배선. Render 외부의 `vtkActor*` 직접 참조 0 |
| 4.1 주석 정책 v2 | Phase 19 변경 파일 전수에 `@brief/@details/@note` 보강 + "Legacy closure tracking ID: P19-WN-...` 메타 삽입 |
| 4.1 임시 호환 주석 제거 | Phase 18에서 남긴 `Temporary compatibility path` 주석은 Phase 19 종료와 함께 삭제 대상 또는 삭제 사유 문서화 |
| 4.2 Git 정책 v2 | WBS(W1~W8) 단위 커밋, 각 WBS 완료 시 dependency_gate_report 업데이트 |
| 4.3 검증 정책 | 기존 `check:phase7/12/13/14/15/17/18/*` 게이트 전부 PASS 유지 + 신규 `check:phase19:*` 게이트 7종 PASS |
| 4.4 UI 상태 동기화 | `app.cpp` 재분해 전후로 Reset/Layout/메뉴 오픈 회귀 방지 E2E 스펙 재실행 필수 |

---

## 2. 현재 기준선 (2026-04-21 독립 검증 측정값)

### 2.1 Legacy 경로 인벤토리

| 경로 | 파일 수 | LOC | 분류 |
|---|---:|---:|---|
| `workspace/legacy/atoms_template_facade.h` | 1 | 1,485 | **god-object facade** |
| `workspace/legacy/atoms_template_facade.cpp` | 1 | 5,808 | **god-object facade** |
| `workspace/legacy/legacy_atoms_runtime.h` | 1 | 8 | static accessor |
| `workspace/legacy/legacy_atoms_runtime.cpp` | 1 | 16 | static accessor |
| `render/application/legacy_viewer_facade.h` | 1 | 11 | composition bridge |
| `render/application/legacy_viewer_facade.cpp` | 1 | 13 | composition bridge |
| `structure/infrastructure/legacy/legacy_structure_service_port.h` | 1 | 38 | port adapter |
| `structure/infrastructure/legacy/legacy_structure_service_port.cpp` | 1 | 67 | port adapter |
| `measurement/infrastructure/legacy/legacy_measurement_service_port.h` | 1 | 37 | port adapter |
| `measurement/infrastructure/legacy/legacy_measurement_service_port.cpp` | 1 | 111 | port adapter |
| `density/infrastructure/legacy/legacy_density_service_port.h` | 1 | 39 | port adapter |
| `density/infrastructure/legacy/legacy_density_service_port.cpp` | 1 | 65 | port adapter |
| `io/infrastructure/legacy/legacy_import_runtime_port.h` | 1 | 45 | port adapter |
| `io/infrastructure/legacy/legacy_import_runtime_port.cpp` | 1 | 124 | port adapter |
| **합계** | **14** | **7,867** | |

### 2.2 Legacy 참조 분포

| 종류 | 건수 |
|---|---:|
| `workspace/legacy/` include (파일 수, 외부 관점) | 11 (`workspace/legacy` 내부 제외) |
| `LegacyAtomsRuntime()` 호출 파일 수 | 14 |
| 각 feature의 `infrastructure/legacy/` 헤더 include (외부) | 5 (서비스 1 + workbench_runtime) |
| `render/application/legacy_viewer_facade.h` include | 3 (`workbench_runtime.cpp`, `vtk_render_gateway.{h,cpp}`) |

### 2.3 검증 지표 (검증보고서 1.2 재현)

| 지표 | 현재값 | Phase 19 목표 |
|---|---:|---:|
| `webassembly/src/**/legacy/**` 파일 | 14 | **0** |
| `workspace/legacy/atoms_template_facade.cpp` LOC | 5,808 | **파일 소멸** |
| `workspace/legacy/atoms_template_facade.h` LOC | 1,485 | **파일 소멸** |
| `LegacyAtomsRuntime()` 호출 | 14 파일 | **0** |
| `DECLARE_SINGLETON` (VtkViewer/MeshManager/FontManager) | 3 | **0** |
| `::Instance()` 전체 호출 | 48 | **0** (단, `macro/singleton_macro.h` 매크로 정의만 보존 여부 결정) |
| `friend class` 전체 | 1 | **0** |
| `app.cpp` LOC | 1,430 | **≤ 400** (프레임 오케스트레이션만) |
| `shell/presentation/main_menu.cpp` 존재 | 없음 | **존재** |
| `shell/presentation/window_layout.cpp` 존재 | 없음 | **존재** |
| render 외부 `vtkActor*` 참조 | 29건 (12파일) | **0** (legacy 파일 소멸로 자연 해소) |
| CMake 모듈 타깃 수 | 10 | 10 (유지) |

---

## 3. 해체 전략 (Top-Down 매핑)

### 3.1 `AtomsTemplate` 분해 원칙 — "책임 귀속 테이블"

`workspace/legacy/atoms_template_facade.{h,cpp}`의 186개 공개 메서드(추정)를 다음 4가지 귀속처 중 하나로 **전량 이관**한다. 잔존 코드는 없다.

| 책임 군 | 귀속 경로 | Phase 19 WBS |
|---|---|---|
| Atom/Bond/Cell 상호작용 (Select/Hover/Visibility/Toggle) | `structure/application/structure_interaction_service.{h,cpp}` (신규) | W5.1 |
| Measurement 모드/피킹/오버레이 | `measurement/application/measurement_service.*` + `measurement/presentation/measurement_overlay_renderer.*` (신규) | W2, W5.2 |
| Charge Density 로드/표시/그리드 | `density/application/density_service.*` + `density/presentation/charge_density_controller.*` (신규) | W5.3 |
| Structure 등록/제거/BZ/특수점 | `structure/application/structure_lifecycle_service.*` (기존 + 이관) | W5.4 |
| UI 창 렌더(`RenderPeriodicTableWindow`, `RenderChargeDensityViewerWindow` 등) | `shell/presentation/atoms/*` + `density/presentation/*` | W5.5 |
| XSF/CHGCAR 로드 브릿지 | `io/application/import_*` (기존) | W5.6 |

각 귀속처는 **도메인 Repository/Store가 상태의 단일 소유자**이며, 과거 `AtomsTemplate`의 멤버 변수(`m_MeasurementMode`, `m_MeasurementPickedAtomIds`, `m_ChargeDensity*` 등)는 다음 원칙으로 분산된다:

1. 순수 상태 → `*/domain/*Repository` 또는 `*/domain/*Store`
2. 렌더 캐시/VTK 핸들 → `render/infrastructure/*`
3. UI 일시 상태 (토글/팝업) → `shell/domain/shell_state_store`

### 3.2 Legacy Port 소거 원칙 — "Adapter → Internal Service 승격"

현재 4개 feature의 `infrastructure/legacy/legacy_*_service_port.{h,cpp}`는 **Port 패턴의 임시 구현**으로서 내부에서 `LegacyAtomsRuntime()`을 호출해 facade에 위임하고 있다. Phase 19에서는:

1. 대응하는 `application/*_service.cpp`를 port 주입 구조에서 **직접 도메인 서비스 호출 구조로 승격**한다. 더 이상 port가 필요하지 않다.
2. Port 인터페이스 자체(`*/application/*_service_port.h`)는 **필요 시 유지, 불필요 시 제거**한다. 판단 기준: 해당 port가 composition root에서 테스트 double 주입에 사용되면 유지, 오직 legacy adapter만이 구현한다면 제거.
3. `infrastructure/legacy/` 디렉터리는 **빈 디렉터리 포함 물리적 소거**한다.

### 3.3 `LegacyAtomsRuntime()` 소거 원칙 — "Static 인스턴스 → Runtime 주입"

`workspace/legacy/legacy_atoms_runtime.cpp` 내부의 `static AtomsTemplate runtime;`은 프로세스 수명 static 인스턴스로 Phase 7의 composition root 원칙을 약화시킨다. Phase 19에서는:

1. `AtomsTemplate` 클래스 자체의 **소멸**과 함께 `LegacyAtomsRuntime()`도 소멸한다.
2. 호출 14파일은 각자 필요한 도메인 서비스를 `WorkbenchRuntime`으로부터 **생성자/setter 주입**으로 획득한다.

### 3.4 `VtkViewer`/`MeshManager`/`FontManager` Singleton 소거 원칙

독립 검증보고서 6.2절은 이 3개를 "잔여 과제"로 명시했다. Phase 19에서 다음 순서로 제거한다:

| Singleton | 소유 이관처 | 주입 경로 |
|---|---|---|
| `VtkViewer` | `WorkbenchRuntime::viewer()` (이미 사실상 1개만 생성) | render module 내부에서만 reference 전달 |
| `MeshManager` | `WorkbenchRuntime::meshRepository()` (이미 wrap 존재) | `MeshCommandService`, `MeshQueryService`에 생성자 주입 |
| `FontManager` | `WorkbenchRuntime::fontRegistry()` (신규) | `app.cpp` → shell presentation font 계층에 주입 |

최종 상태에서 `macro/singleton_macro.h`는 **어떤 클래스에서도 사용되지 않아야** 하며, 매크로 정의 자체는 보존 여부를 W6 종료 시 결정한다(호환 매크로로 유지하더라도 **호출부 0**이 DoD).

### 3.5 `app.cpp` 재분해 원칙

현재 1,430 LOC / 34 메서드의 `app.cpp`를 다음 5개 파일로 분해한다.

| 대상 책임 | 이관처 | 예상 LOC |
|---|---|---:|
| Menu bar 렌더 (`renderDockSpaceAndMenu`의 메뉴 부분 ~300 LOC) | `shell/presentation/main_menu.{h,cpp}` (신규) | ~350 |
| DockSpace 레이아웃/ Reset/Layout1~3 (`renderDockSpaceAndMenu`의 레이아웃 부분 ~130 LOC) | `shell/presentation/window_layout.{h,cpp}` (신규) | ~200 |
| ImGui 창 초기화/렌더 (`InitImGuiWindows`, `renderImGuiWindows` ~280 LOC) | `shell/presentation/window_registry.{h,cpp}` (신규) | ~320 |
| 진행률/About 팝업 (`renderProgressPopup`, `renderAboutPopup` ~150 LOC) | `shell/presentation/popup_presenter.{h,cpp}` (신규 또는 기존 `import_popup_presenter`와 병합 고려) | ~180 |
| 폰트/컬러 스타일/DPI (`applyFontScale`, `SetDetailedStyle`, `SetInitColorStyle`, `SetInitFontSize` ~300 LOC) | `shell/presentation/font/font_scale_controller.{h,cpp}` + `shell/presentation/theme/color_style_controller.{h,cpp}` (신규) | ~350 |
| 프레임 오케스트레이션 (`main loop`, `initLogger`, `syncShellState*`) | `app.cpp` 잔존 (≤ 400 LOC) | ~380 |

### 3.6 `friend class` 해소 원칙

`io/application/import_entry_service.h:76`의 `friend class shell::presentation::ImportPopupPresenter;`는 `FileLoader`의 private 멤버에 대한 presenter의 직접 접근을 허용한다. Phase 19에서는 다음 중 하나를 선택한다.

1. **옵션 A (권장)**: `FileLoader`가 `ImportPopupPresenter`에게 필요한 정보를 **명시적 public API**로 노출하고, presenter의 `FileLoader*` 멤버를 `std::reference_wrapper<FileLoader>` + 명시 API만 사용하도록 재배선.
2. **옵션 B**: presenter가 접근하는 private 멤버를 **`ImportPopupState` 같은 값 객체로 포장**하여 public API로 노출. presenter는 값 객체만 읽는다.

세부는 W1에서 접근 패턴 인벤토리 후 결정한다.

### 3.7 `measurement_controller.cpp`의 VTK 토큰 제거 원칙

현재 해당 파일은 `AtomsTemplate::HandleMeasurementClickByPicker(vtkActor* actor, double pickPos[3])` **정의부**가 measurement application 디렉터리에 위치하면서 `vtkActor*`을 파라미터로 받는다. 이는 Phase 15 DoD("`render` 외부 VTK 직접 조작 0")의 유일한 위반 경로다.

해소 순서:

1. `render/application/render_gateway.h`에 `PickedAtomInfo` 값 객체(actor 파생 정보: structureId, atomId, position)를 반환하는 `ResolvePickedAtom(double pickPos[3])` API 추가 (또는 이미 존재하면 활용).
2. `measurement_controller.cpp`에서 `vtkActor*` 파라미터를 제거하고 `PickedAtomInfo`를 파라미터로 받도록 서명 변경.
3. 호출 경로(`render/application/viewer_interaction_controller.cpp` → `AtomsTemplate::HandleMeasurementClickByPicker`)에서 `ResolvePickedAtom` 호출 후 값 객체를 measurement service에 전달.
4. W5.2에서 `AtomsTemplate::HandleMeasurementClickByPicker` 자체가 `MeasurementService::handlePickerClick(PickedAtomInfo)`로 이전되며 본 항목은 자연 종료.

---

## 4. WBS (실행 순서 고정)

Phase 19는 **W0~W9** 10단계로 구성된다. 세부 실행계획은 `docs/refactoring/phase19/W{1..8}_*.md`의 독립 sub-plan 문서에 수록한다.

### W0. 기준선 고정 + 인벤토리

1. 본 계획서 및 `docs/refactoring/phase19/` 하위 sub-plan 8종 확정
2. 현재 legacy 경로 14파일/7,867 LOC 수치 동결
3. `LegacyAtomsRuntime()` 호출 14파일 목록 동결
4. `AtomsTemplate` public 메서드 186건 책임 귀속 매트릭스 작성 → `docs/refactoring/phase19/logs/atoms_template_ownership_matrix_phase19_latest.md`
5. `app.cpp` 34 메서드 이관 매트릭스 작성 → `docs/refactoring/phase19/logs/app_cpp_decomposition_matrix_phase19_latest.md`
6. Phase 18 게이트 재실행 기준값(baseline) 캡처

산출물:
- `docs/refactoring/phase19/logs/phase19_inventory_snapshot_latest.md`
- `docs/refactoring/phase19/logs/atoms_template_ownership_matrix_phase19_latest.md`
- `docs/refactoring/phase19/logs/app_cpp_decomposition_matrix_phase19_latest.md`
- `docs/refactoring/phase19/logs/legacy_runtime_caller_inventory_phase19_latest.md`

완료 기준: W1~W8 비교용 기준선 확정

---

### W1. `friend class` 1건 해소 [High]

세부계획: `docs/refactoring/phase19/W1_friend_class_removal_260421.md`

핵심 과업:
1. `io/application/import_entry_service.h:76`의 `friend class shell::presentation::ImportPopupPresenter;` 제거
2. `ImportPopupPresenter`가 `FileLoader` private에 의존하는 지점 인벤토리화
3. 필요한 API를 `FileLoader` public으로 승격 또는 `ImportPopupState` 값 객체로 포장
4. `check_phase19_friend_class_zero.ps1` 게이트 추가

완료 기준:
- `webassembly/src` 전체 `friend\s+class` grep 결과 0
- 기존 `check_phase14_state_ownership_alias_zero.ps1` 정합성 유지(또는 보완)
- smoke/e2e PASS

---

### W2. `measurement_controller.cpp` VTK 토큰 제거 [Medium]

세부계획: `docs/refactoring/phase19/W2_measurement_controller_vtk_removal_260421.md`

핵심 과업:
1. `render/application/render_gateway.h`에 `PickedAtomInfo ResolvePickedAtom(double pickPos[3])` API 도입(또는 기존 API 활용)
2. `measurement_controller.cpp`의 `AtomsTemplate::HandleMeasurementClickByPicker` 서명을 `vtkActor*` 없이 `PickedAtomInfo` 받도록 변경
3. `render/application/viewer_interaction_controller.cpp`의 호출부 조정
4. `check_phase15_render_platform_isolation.ps1` 재실행 PASS

완료 기준:
- `measurement/` 하위 `vtkActor`/`vtkVolume` 참조 0
- Phase 15 게이트 재실행 PASS

---

### W3. Feature 서비스 포트/Adapter 내재화

세부계획: `docs/refactoring/phase19/W3_service_port_internalization_260421.md`

서브 단계:
- W3.1 `StructureService` — `structure/application/structure_service.cpp`가 `legacy_structure_service_port`를 경유하지 않고 도메인 서비스를 직접 호출
- W3.2 `MeasurementService` — 동일 원칙 적용
- W3.3 `DensityService` — 동일 원칙 적용

각 서브 단계 공통 절차:
1. 대응 서비스의 port 주입 지점을 **도메인 Repository/Store 주입**으로 교체
2. `infrastructure/legacy/legacy_*_service_port.{h,cpp}` 파일 삭제
3. `*/application/*_service_port.h`(추상 인터페이스)는 composition root의 테스트 double 주입 수요에 따라 유지/삭제 결정
4. 해당 `infrastructure/legacy/` 디렉터리 물리적 삭제
5. CMake `wb_{structure,measurement,density}.cmake`에서 관련 엔트리 제거

완료 기준:
- `webassembly/src/{structure,measurement,density}/infrastructure/legacy/` 경로 미존재
- `*_service.cpp` 파일의 `workspace/legacy/` include 0
- `*_service.cpp` 파일의 `LegacyAtomsRuntime()` / `AtomsTemplate::` 참조 0
- feature 단위 unit 테스트(`webassembly/tests/*`) PASS

---

### W4. IO/Render legacy 제거

세부계획: `docs/refactoring/phase19/W4_io_render_legacy_removal_260421.md`

두 갈래:

#### W4.1 `io/infrastructure/legacy/`
1. `io/application/import_*`의 `ImportRuntimePort` 의존을 **도메인 Repository(Structure/Density) 직접 주입**으로 교체
2. `legacy_import_runtime_port.{h,cpp}` 파일 삭제 + 디렉터리 삭제
3. `io/application/*_port.h`(추상) 유지/삭제 결정

#### W4.2 `render/application/legacy_viewer_facade.*`
1. `shell/runtime/workbench_runtime.cpp`의 `GetLegacyViewerFacade()` 호출 경로를 `WorkbenchRuntime::viewer()`로 직접 치환
2. `render/infrastructure/vtk_render_gateway.{h,cpp}`에서 동일 호출 경로 치환
3. `render/application/legacy_viewer_facade.{h,cpp}` 삭제
4. CMake `wb_render.cmake`에서 엔트리 제거

완료 기준:
- `webassembly/src/io/infrastructure/legacy/` 미존재
- `webassembly/src/render/application/legacy_viewer_facade.*` 미존재
- `GetLegacyViewerFacade` grep 결과 0

---

### W5. `AtomsTemplate` facade 완전 분해 [최대 작업량]

세부계획: `docs/refactoring/phase19/W5_atoms_template_facade_decomposition_260421.md`

서브 단계 (완료 순서 고정):

- **W5.1 Atom/Bond/Cell interaction** — `structure/application/structure_interaction_service.{h,cpp}` 신규, 관련 메서드 ~40건 이관
- **W5.2 Measurement 피킹/오버레이** — `measurement/application/measurement_service.cpp`로 피킹 로직 흡수, `measurement/presentation/measurement_overlay_renderer.{h,cpp}` 신규, 메서드 ~25건 이관 (W2와 연계)
- **W5.3 Charge density 로드/표시** — `density/application/density_service.cpp`로 로직 흡수, `density/presentation/charge_density_controller.{h,cpp}` 신규, 메서드 ~50건 이관
- **W5.4 Structure 라이프사이클/BZ/특수점** — `structure/application/structure_lifecycle_service.cpp` 확장, 메서드 ~30건 이관
- **W5.5 UI 창 렌더 메서드** — `shell/presentation/atoms/*`로 분산 이관, 메서드 ~25건 이관
- **W5.6 XSF/CHGCAR 브릿지** — `io/application/import_*`로 흡수, 메서드 ~16건 이관
- **W5.7 `workspace/legacy/` 파일 4종 물리 삭제** — `atoms_template_facade.{h,cpp}`, `legacy_atoms_runtime.{h,cpp}` 삭제
- **W5.8 CMake 모듈 갱신** — `wb_workspace.cmake`에서 legacy 엔트리 제거

완료 기준:
- `workspace/legacy/` 디렉터리 미존재
- `AtomsTemplate` 심볼 grep 결과 0 (테스트 포함)
- `LegacyAtomsRuntime` 심볼 grep 결과 0
- e2e 전 시나리오 PASS (Reset/Layout/BZ/Import/Measurement/Charge Density)

**⚠️ 리스크**: W5는 Phase 19 전체 공수의 60% 이상이 예상된다. W3/W4가 완전 선행되어야 하며, 각 서브 단계(W5.1~W5.6)는 개별 PR + 개별 회귀 테스트로 분리 커밋한다.

---

### W6. Runtime Singleton 제거 [Low]

세부계획: `docs/refactoring/phase19/W6_runtime_singleton_elimination_260421.md`

서브 단계:

- **W6.1 `VtkViewer`** — `render/presentation/viewer_window.h`의 `DECLARE_SINGLETON(VtkViewer)` 제거. 단일 인스턴스는 `WorkbenchRuntime::viewer()` 소유. render module 내부에서만 reference 전달.
- **W6.2 `MeshManager`** — `mesh/domain/mesh_repository_core.h`의 `DECLARE_SINGLETON(MeshManager)` 제거. `WorkbenchRuntime::meshRepository()` 경유.
- **W6.3 `FontManager`** — `shell/presentation/font/font_registry.h`의 `DECLARE_SINGLETON(FontManager)` 제거. `WorkbenchRuntime::fontRegistry()` 경유.
- **W6.4 `macro/singleton_macro.h` 처리** — 호출 0 확인 후 매크로 정의 삭제 또는 "deprecated, do not use" 주석 + `check_phase19_singleton_zero.ps1`에서 macro usage 0 검증.

완료 기준:
- `DECLARE_SINGLETON\s*\(` grep 결과 0 (정의 외)
- `::Instance()` grep 결과 0 (composition root 포함 전체)
- e2e PASS

---

### W7. `app.cpp` 재분해 [Medium]

세부계획: `docs/refactoring/phase19/W7_app_cpp_redecomposition_260421.md`

서브 단계:

- **W7.1 `shell/presentation/main_menu.{h,cpp}` 신규** — `renderDockSpaceAndMenu`의 메뉴 부분 이관
- **W7.2 `shell/presentation/window_layout.{h,cpp}` 신규** — DockSpace 및 Layout1~3/Reset 이관, `RequestLayout1()` 정적 진입점을 instance 기반으로 전환
- **W7.3 `shell/presentation/window_registry.{h,cpp}` 신규** — `InitImGuiWindows`, `renderImGuiWindows` 이관
- **W7.4 `shell/presentation/popup_presenter.{h,cpp}` 신규 또는 기존 병합** — `renderProgressPopup`, `renderAboutPopup`, `showProgressPopup`, `setProgressPopupText` 이관
- **W7.5 `shell/presentation/font/font_scale_controller.*` + `shell/presentation/theme/color_style_controller.*` 신규** — 폰트/컬러 관련 메서드 이관
- **W7.6 `app.cpp` 잔존 ≤ 400 LOC 검증** — `check_phase19_app_cpp_size.ps1` 게이트 PASS
- **W7.7 회귀 테스트** — `workbench-layout-panel-regression.spec.ts` + 수동 체크리스트 재실행

완료 기준:
- `app.cpp` 크기 ≤ 400 LOC
- `shell/presentation/main_menu.cpp`, `window_layout.cpp` 존재
- `App::*` 메서드 수 ≤ 10 (프레임 오케스트레이션 및 IdbFS 라이프사이클만)
- Reset/Layout/패널 오픈 회귀 E2E PASS

---

### W8. 최종 봉인 + 게이트/산출물

세부계획: `docs/refactoring/phase19/W8_final_seal_gates_and_artifacts_260421.md`

1. 신규 게이트 스크립트 7종 추가 (scripts/refactoring/):
   - `check_phase19_legacy_directory_zero.ps1` — `**/legacy/**` 경로 0 검증
   - `check_phase19_atoms_template_zero.ps1` — `AtomsTemplate` 심볼 전역 0 검증
   - `check_phase19_legacy_runtime_zero.ps1` — `LegacyAtomsRuntime` 심볼 0 검증
   - `check_phase19_singleton_zero.ps1` — `DECLARE_SINGLETON`/`::Instance()` 전량 0 검증
   - `check_phase19_friend_class_zero.ps1` — `friend\s+class` 0 검증
   - `check_phase19_app_cpp_size.ps1` — `app.cpp` ≤ 400 LOC 검증
   - `check_phase19_final_closure.ps1` — 위 6개 + Phase 7/12/13/14/15/17/18 기존 게이트 모두 PASS 메타 게이트
2. `package.json`에 `check:phase19:*` 진입점 7종 추가
3. `docs/refactoring/phase19/logs/`에 최종 로그 패키지 생성 (수동 UI 회귀 증빙, smoke/e2e 로그)
4. 종료 보고서 2종 작성:
   - `docs/refactoring/refactoring_result_report_phase19_final_closure_260XXX.md`
   - `docs/refactoring/refactoring_result_report_phase0_19_comprehensive_260XXX.md`
5. 검증보고서 5장의 4개 리스크 모두 "해소됨"으로 문서화
6. 검증보고서 4.2의 DoD 11개 중 미달성/부분 달성 4건 재측정 → 전량 달성 확인

완료 기준:
- 7종 신규 게이트 PASS
- 기존 Phase 7~18 게이트 전부 재실행 PASS
- 종료 보고서 2종 존재
- 독립 검증 재실시 시 DoD 11/11 달성

---

### W9. 릴리스 및 브랜치 정리

1. `main` 병합
2. `refactoring/phase19` 작업 브랜치 아카이브 태그(`tag: refactor-phase19-closed`)
3. Docker/K8s 배포 회귀 확인 (`npm run build:docker` → dev 네임스페이스 테스트 배포)

---

## 5. 완료 정의 (DoD)

### 5.1 정량 DoD

| 지표 | 측정 명령 | 기준값 |
|---|---|---:|
| `webassembly/src/**/legacy/**` 파일 | `find webassembly/src -path "*/legacy/*" -name "*.cpp" -o -path "*/legacy/*" -name "*.h"` | **0** |
| `webassembly/src/**/legacy/**` 디렉터리 | `find webassembly/src -type d -name "legacy"` | **0** |
| `AtomsTemplate` 심볼 | `grep -r "AtomsTemplate" webassembly/src` | **0** |
| `LegacyAtomsRuntime` 심볼 | `grep -r "LegacyAtomsRuntime" webassembly/src` | **0** |
| `GetLegacyViewerFacade` 심볼 | `grep -r "GetLegacyViewerFacade" webassembly/src` | **0** |
| `DECLARE_SINGLETON` 사용(정의 외) | `grep -r "DECLARE_SINGLETON\s*\(" webassembly/src --include=*.h --include=*.cpp \| grep -v macro/singleton_macro.h` | **0** |
| `::Instance()` 전체 호출 | `grep -rn "::Instance()" webassembly/src` | **0** |
| `friend\s+class` 전체 | `grep -rn "friend\s\+class" webassembly/src` | **0** |
| `app.cpp` LOC | `wc -l webassembly/src/app.cpp` | **≤ 400** |
| render 외부 `vtkActor*` 참조 파일 수 | Phase 15 게이트 | **0** |
| CMake 모듈 수 | `webassembly/cmake/modules/*.cmake` 카운트 | **10 유지** |
| 빈 폴더 | `find webassembly/src -type d -empty` | **0** |

### 5.2 정성 DoD

1. `WorkbenchRuntime`은 모든 도메인 서비스/Repository/Store를 **명시적 조립**으로 소유한다.
2. `macro/singleton_macro.h`의 `DECLARE_SINGLETON`는 어떤 프로덕션 코드에서도 사용되지 않는다.
3. `shell/presentation/main_menu.cpp`, `window_layout.cpp`, `window_registry.cpp`는 `app.cpp`로부터 **실제로 책임을 이관**받아 구현되어 있다.
4. `docs/refactoring/refactoring_verification_report_independent_260420.md` 5장 리스크 R1~R4, 권고 High/Medium/Low 5건이 **모두 해소됨**으로 갱신 문서에 기록된다.
5. 독립 검증보고서 4.2절 DoD 11개 항목 재측정 시 전량 "달성" 판정을 받는다.

### 5.3 회귀 DoD

1. `npm run build:full` PASS
2. `npm run type-check` PASS
3. `npm run lint` PASS
4. `webassembly/tests/*` 전체 유닛 테스트 PASS
5. `tests/e2e/workbench-smoke.spec.ts` PASS
6. `tests/e2e/workbench-layout-panel-regression.spec.ts` PASS
7. `npm run build:docker` PASS
8. 수동 UI 회귀 시나리오 전량 PASS (Phase 18 체크리스트 기준)

---

## 6. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| `AtomsTemplate` 분해 중 숨은 사이드이펙트(5,808 LOC) | 렌더/선택/측정 회귀 | W5를 6개 서브 PR로 쪼개고 각 PR마다 e2e 실행. feature 모듈별 `structure_interaction_service_test.cpp` 등 단위 테스트 선행 작성 |
| Port 인터페이스 제거 시 테스트 double 주입 채널 상실 | CI 유닛 테스트 커버리지 하락 | W3에서 port 유지/삭제를 파일별로 결정하고 유지 선택 시 명시 문서화 |
| `app.cpp` 분해 과정에서 ImGui 상태/dock layout 깨짐 | 사용자 UX 회귀 | W7을 서브 단계 단위로 커밋하며 각 단계 완료 시 `window.__VTK_WORKBENCH_TEST__` API로 E2E 강제 회귀 |
| Singleton 제거 시 초기화 순서 변경에 따른 부팅 실패 | 앱 기동 불능 | W6 서브 단계마다 `workbench_runtime.cpp` 조립 로그 + headless smoke test |
| Phase 15의 `measurement/application`에 대한 vtkActor 파라미터 제거 → render_gateway 확장 필요 | 새로운 public API 유입 | W2에서 `PickedAtomInfo` 값 객체 계약을 Doxygen으로 명문화 |
| 테스트 인프라(`@playwright/test`)만 있고 실행 스크립트 미존재 | CI에서 e2e 실행 불가 | W0에 `package.json` `test:e2e` 스크립트 추가 및 Phase 18 로그와 동일 패턴 도입 |

---

## 7. Git 정책 (Phase 19)

1. 브랜치: `refactor/phase19-legacy-full-dismantle`
2. 커밋 단위: WBS 서브 단계(W1, W2, W3.1, W3.2, ..., W8) 당 1커밋 이상
3. 각 커밋 메시지 전문에 **변경 전/후 정량 지표 3행**(legacy 파일 수, `::Instance()` 호출, `AtomsTemplate` 참조) 포함
4. 각 WBS 완료 시 `docs/refactoring/phase19/logs/dependency_gate_report_W{N}.md` 갱신
5. PR 생성 시 "5.1 정량 DoD" 표를 PR description에 복사, 측정값 인라인 기록

---

## 8. 산출물 목록

### 8.1 본 계획서 및 Sub-plan 8종

1. `docs/refactoring/refactoring_plan_phase19_legacy_full_dismantle_260421.md` (본 문서)
2. `docs/refactoring/phase19/W1_friend_class_removal_260421.md`
3. `docs/refactoring/phase19/W2_measurement_controller_vtk_removal_260421.md`
4. `docs/refactoring/phase19/W3_service_port_internalization_260421.md`
5. `docs/refactoring/phase19/W4_io_render_legacy_removal_260421.md`
6. `docs/refactoring/phase19/W5_atoms_template_facade_decomposition_260421.md`
7. `docs/refactoring/phase19/W6_runtime_singleton_elimination_260421.md`
8. `docs/refactoring/phase19/W7_app_cpp_redecomposition_260421.md`
9. `docs/refactoring/phase19/W8_final_seal_gates_and_artifacts_260421.md`

### 8.2 실행 산출물 (Phase 19 종료 시)

- `docs/refactoring/phase19/logs/phase19_inventory_snapshot_latest.md`
- `docs/refactoring/phase19/logs/atoms_template_ownership_matrix_phase19_latest.md`
- `docs/refactoring/phase19/logs/app_cpp_decomposition_matrix_phase19_latest.md`
- `docs/refactoring/phase19/logs/legacy_runtime_caller_inventory_phase19_latest.md`
- `docs/refactoring/phase19/logs/dependency_gate_report_W{1..8}.md`
- `docs/refactoring/phase19/logs/manual_ui_regression_phase19_latest.md`
- `docs/refactoring/refactoring_result_report_phase19_final_closure_260XXX.md`
- `docs/refactoring/refactoring_result_report_phase0_19_comprehensive_260XXX.md`
- `scripts/refactoring/check_phase19_*.ps1` 7종
- `package.json`의 `check:phase19:*` 진입점 7종

---

## 9. 타임라인 (권장)

| WBS | 예상 소요 | 선행 의존 |
|---|---:|---|
| W0 | 1~2일 | - |
| W1 | 0.5~1일 | W0 |
| W2 | 0.5~1일 | W0 |
| W3 (3.1+3.2+3.3) | 2~3일 | W1, W2 |
| W4 (4.1+4.2) | 1~2일 | W3 |
| W5 (5.1~5.8) | 7~10일 | W4 |
| W6 (6.1~6.4) | 2~3일 | W5 |
| W7 (7.1~7.7) | 3~4일 | W6 또는 병행 가능 |
| W8 | 1~2일 | W7 |
| W9 | 0.5일 | W8 |
| **합계** | **18~28일** | |

본 타임라인은 단독 개발자 1명 기준 집중 작업 시간이며, 병행 가능한 트랙(W6과 W7)은 동시 진행 시 2~3일 단축 가능하다.

---

## 10. 결론

본 Phase 19 계획은 `refactoring_verification_report_independent_260420.md`가 식별한 **모든 잠재 리스크 해소 + 모든 권고 사항 이행 + webassembly/src 하위 legacy 완전 해체**를 단일 종결 Phase로 수행한다. Phase 18의 "격리 종료" 판정을 존중하되, 그 격리를 **영구 소거**로 전환하여 리팩토링 프로그램을 **무조건적 0-기반 DoD**로 수렴시킨다.

완료 시 `webassembly/src` 트리에 `legacy`라는 이름의 디렉터리/파일/심볼이 어떤 형태로도 남아있지 않으며, `WorkbenchRuntime`이 모든 객체의 단일 소유자가 된다. 이는 향후 신규 feature 추가 시 "legacy 안에 숨겨놓고 뒷정리한다"는 유혹이 구조적으로 차단되는 상태를 의미한다.

---

*본 계획서는 세부 sub-plan 8종(W1~W8)과 함께 완전한 실행 매뉴얼을 구성한다. 각 sub-plan은 해당 WBS의 파일 단위 변경 명세, 회귀 테스트, 게이트, 완료 기준을 상세히 기술한다.*
