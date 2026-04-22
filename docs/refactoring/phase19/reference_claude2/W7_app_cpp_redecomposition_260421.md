# Phase 19 / W7 — `app.cpp` 재분해

작성일: `2026-04-21 (KST)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_full_dismantle_claude2_260421.md`
검증보고서 근거: 5.1 R4 (1,430 LOC 잔존), 5.2 Medium 권고, Phase 11 DoD 부분 달성
우선순위: **Medium**
예상 소요: 3 ~ 4일
선행 의존: W0 (이관 매트릭스). W6과 병행 가능 (브랜치 분리 시).
후속 작업: W8 (게이트 봉인)

---

## 1. 배경

Phase 11 DoD는 `app.cpp`의 책임을 `main_menu.cpp`/`window_layout.cpp`/`window_registry.cpp`로 분해하는 것을 포함했다. 독립 검증보고서에 따르면:

- `app.cpp` LOC: **1,430**
- `main_menu.cpp` 존재: **없음**
- `window_layout.cpp` 존재: **없음**

즉, Phase 11 DoD 4번이 미달성 상태로 남아 있다. W7은 이를 최종 달성시키며, `app.cpp`를 **프레임 오케스트레이션과 IdbFS 라이프사이클만** 담당하는 ≤ 400 LOC 크기로 축소한다.

## 2. 현재 상태 — `app.cpp` 34 메서드 분류

### 2.1 유지 (프레임 오케스트레이션/ IdbFS)

| 메서드 | LOC (추정) |
|---|---:|
| `App::App()` / `~App()` | ~40 |
| `App::initLogger()` | ~30 |
| `App::initImGui()` | ~60 |
| `App::run()` / main loop | ~150 |
| `App::shutdown()` | ~50 |
| IdbFS `syncfs` 콜백 및 라이프사이클 | ~50 |
| **소계** | **~380** |

### 2.2 이관 대상

| 책임 군 | 대상 파일 (신규) | 예상 LOC |
|---|---|---:|
| Menu bar 렌더 | `shell/presentation/main_menu.{h,cpp}` | ~350 |
| DockSpace/ Layout1~3/ Reset | `shell/presentation/window_layout.{h,cpp}` | ~200 |
| ImGui 창 초기화/렌더 | `shell/presentation/window_registry.{h,cpp}` | ~320 |
| 진행률/About 팝업 | `shell/presentation/popup_presenter.{h,cpp}` (신규 또는 기존 `import_popup_presenter` 병합) | ~180 |
| 폰트/컬러/DPI | `shell/presentation/font/font_scale_controller.{h,cpp}` + `shell/presentation/theme/color_style_controller.{h,cpp}` | ~350 |

합계 이관 대상 LOC: ~1,400 → `app.cpp` 잔존 ~380 + shell/presentation 신규 ~1,400 ≈ 1,780 (주석/가독성 보강 반영 시 소폭 증가 허용).

## 3. 서브 단계

### W7.1 — `main_menu.{h,cpp}` 신규

#### 3.1 이관 대상

- `App::renderDockSpaceAndMenu()`의 메뉴 부분 (File/View/Help 등 메뉴 트리 ImGui::BeginMenu/...)
- 각 메뉴 항목의 핸들러 lambda

#### 3.2 클래스 설계

```cpp
// webassembly/src/shell/presentation/main_menu.h
namespace wb::shell::presentation {

class MainMenu {
public:
    MainMenu(ShellStateStore& state, WindowLayout& layout, ImportEntryService& io, ...);
    void render(); // ImGui::BeginMainMenuBar
private:
    ShellStateStore& state_;
    WindowLayout& layout_;
    ImportEntryService& io_;
    // ...
};

} // namespace
```

#### 3.3 App 측 호출

```cpp
// app.cpp
void App::renderFrame() {
    mainMenu_.render();         // MainMenu 인스턴스
    windowLayout_.render();     // WindowLayout 인스턴스
    windowRegistry_.render();   // WindowRegistry 인스턴스
    popupPresenter_.render();
    // ...
}
```

### W7.2 — `window_layout.{h,cpp}` 신규

#### 3.4 이관 대상

- `App::renderDockSpaceAndMenu()`의 DockSpace 부분
- `App::resetLayout()`, `App::applyLayout1()`, `App::applyLayout2()`, `App::applyLayout3()`
- `App::RequestLayout1()` 등 정적 진입점 — **instance 기반으로 전환**

#### 3.5 정적 진입점 처리

현재 `App::RequestLayout1()`은 static 함수로 외부(예: JS embind)에서 호출된다. W7.2는 이를 다음 중 하나로 전환:

| 옵션 | 내용 |
|---|---|
| A | `WorkbenchRuntime::windowLayout().requestLayout1()`을 호출하는 static 진입점을 `app.cpp`에 얇게 유지 (C 링크 호환성 목적) |
| B | embind 바인딩을 `workbench_runtime`에 이관 |

권장: **A** (binding 호환성 최소 변경). 단, static 함수는 1-line delegator로 축소.

### W7.3 — `window_registry.{h,cpp}` 신규

#### 3.6 이관 대상

- `App::InitImGuiWindows()` — 모든 ImGui 창(panels)의 초기 등록
- `App::renderImGuiWindows()` — 프레임당 창 렌더 디스패치

#### 3.7 설계

```cpp
class WindowRegistry {
public:
    void registerWindow(std::unique_ptr<IImGuiWindow>);
    void renderAll();
};
```

각 창은 `IImGuiWindow` 인터페이스 구현. Phase 11에서 `shell/presentation/panels/`에 이미 존재할 수 있는 패턴을 재활용.

### W7.4 — `popup_presenter.{h,cpp}` 신규 또는 병합

#### 3.8 이관 대상

- `App::renderProgressPopup()` / `App::showProgressPopup()` / `App::setProgressPopupText()`
- `App::renderAboutPopup()`

#### 3.9 병합 여부

기존 `shell/presentation/import_popup_presenter`가 있으므로 다음 중 선택:

| 옵션 | 내용 |
|---|---|
| A | 공통 `shell/presentation/popup_presenter.{h,cpp}` 신설, import popup도 이리로 이전 |
| B | Progress/About은 별도 `shell/presentation/progress_popup_presenter.{h,cpp}`, `about_popup_presenter.{h,cpp}` |

권장: **B** — 팝업 종류별 단일책임 presenter가 W1의 `ImportPopupPresenter`와 대칭. `import_popup_presenter`와 병합하지 않음.

### W7.5 — Font/Color Controllers

#### 3.10 이관 대상

- `App::applyFontScale()` / `App::initFontSizes()` → `shell/presentation/font/font_scale_controller.{h,cpp}`
- `App::SetDetailedStyle()` / `App::SetInitColorStyle()` → `shell/presentation/theme/color_style_controller.{h,cpp}`
- `App::SetInitFontSize()` → font controller

#### 3.11 의존

W6.3 (`FontManager` singleton 제거) 이후 `FontScaleController`는 `FontManager&`를 주입받아 조작. `ColorStyleController`는 ImGui style만 조작하므로 의존 없음.

### W7.6 — `app.cpp` 크기 검증

- `wc -l webassembly/src/app.cpp` ≤ 400
- `App::` 메서드 수 ≤ 10
- 남은 메서드: 생성자/소멸자, `initLogger`, `initImGui`, `run`, `shutdown`, IdbFS 콜백, `renderFrame`, `syncShellState*` (선택 유지)

### W7.7 — 회귀 테스트

- `tests/e2e/workbench-layout-panel-regression.spec.ts` 재실행 (Reset/Layout1~3, 패널 open/close, 메뉴 항목 클릭)
- 수동 체크리스트 (`docs/refactoring/phase19/logs/manual_ui_regression_W7_latest.md`)

## 4. 파일 변경 명세

| 경로 | 변경 유형 |
|---|---|
| `webassembly/src/shell/presentation/main_menu.h` | **신규** |
| `webassembly/src/shell/presentation/main_menu.cpp` | **신규** |
| `webassembly/src/shell/presentation/window_layout.h` | **신규** |
| `webassembly/src/shell/presentation/window_layout.cpp` | **신규** |
| `webassembly/src/shell/presentation/window_registry.h` | **신규** |
| `webassembly/src/shell/presentation/window_registry.cpp` | **신규** |
| `webassembly/src/shell/presentation/progress_popup_presenter.{h,cpp}` | **신규** |
| `webassembly/src/shell/presentation/about_popup_presenter.{h,cpp}` | **신규** |
| `webassembly/src/shell/presentation/font/font_scale_controller.{h,cpp}` | **신규** |
| `webassembly/src/shell/presentation/theme/color_style_controller.{h,cpp}` | **신규** |
| `webassembly/src/app.cpp` | 수정 (대폭 축소) |
| `webassembly/src/app.h` | 수정 |
| `webassembly/src/workspace/runtime/workbench_runtime.{h,cpp}` | 수정 (새 컨트롤러 조립) |
| `webassembly/cmake/modules/wb_shell.cmake` | 수정 (신규 파일 추가) |
| `scripts/refactoring/check_phase19_app_cpp_size.ps1` | **신규** |
| `package.json` | 수정 |
| `docs/refactoring/phase19/logs/app_cpp_decomposition_matrix_phase19_latest.md` | **신규** (W0에서 시작, W7 완료시 확정) |
| `docs/refactoring/phase19/logs/dependency_gate_report_W7.md` | **신규** |

## 5. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| `app.cpp` LOC | `wc -l` | ≤ 400 |
| `App::` 메서드 수 | grep `App::` | ≤ 10 |
| `shell/presentation/main_menu.cpp` 존재 | find | 있음 |
| `shell/presentation/window_layout.cpp` 존재 | find | 있음 |
| `shell/presentation/window_registry.cpp` 존재 | find | 있음 |
| `check:phase19:app-cpp-size` | npm script | PASS |
| e2e `workbench-layout-panel-regression.spec.ts` | Playwright | PASS |
| 수동 회귀 (Reset, Layout1~3, 패널 open/close, 메뉴 항목 클릭) | 수작업 | PASS |

## 6. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| ImGui DockSpace ID 체계 변경 시 사용자의 `imgui.ini` 무효화 | 사용자 경험 회귀 | `window_layout.cpp`에서 기존 DockSpace ID 문자열/노드 ID를 보존 (변수명만 변경, ID 해시 동일) |
| `App::RequestLayout1()` 등 embind static 진입점 호환성 깨짐 | JS→wasm 호출 실패 | static 진입점을 app.cpp에 얇은 delegator로 유지 (옵션 A) |
| 팝업 presenter 중복 인스턴스 | 렌더 겹침 | `WorkbenchRuntime`이 각 presenter 단일 인스턴스 소유 |
| Font scale controller와 ImGui context 수명 | 크래시 | W6.3의 FontManager 생성 타이밍과 동기화 |
| 34개 메서드 중 일부가 App 내부 private 상태 공유 | 이관 시 상태 누락 | W0의 app_cpp_decomposition_matrix에서 각 메서드별 상태 의존성 기록 후 이관 |

## 7. 산출물

- 신규 파일 10개 (`main_menu`, `window_layout`, `window_registry`, popup 2종, font/theme 2종)
- 수정 파일 4~5개 (`app.cpp`, `app.h`, `workbench_runtime.{h,cpp}`, CMake)
- 게이트 스크립트 1종
- 로그 문서 2종

## 8. Git 커밋 템플릿

```
refactor(shell): phase19/W7.1 — extract main menu from app.cpp

- New shell/presentation/main_menu.{h,cpp} owns menu bar render
- Ctor deps: ShellStateStore, WindowLayout, ImportEntryService (via WorkbenchRuntime)
- app.cpp::renderFrame() now calls mainMenu_.render()

metrics:
  app.cpp LOC: 1430 -> 1080
  App:: methods: 34 -> 28
```

```
refactor(shell): phase19/W7.2 — extract window layout from app.cpp

- New shell/presentation/window_layout.{h,cpp} owns DockSpace + Layout1~3/Reset
- App::RequestLayoutN() retained as thin delegators for embind compatibility
- Dock IDs preserved to keep imgui.ini valid

metrics:
  app.cpp LOC: 1080 -> 880
  App:: methods: 28 -> 22
```

(유사 커밋을 W7.3, W7.4, W7.5에 반복)

```
refactor(shell): phase19/W7.6 — app.cpp size gate closure

- app.cpp now ≤400 LOC, frame orchestration + IdbFS lifecycle only
- Add scripts/refactoring/check_phase19_app_cpp_size.ps1
- package.json: check:phase19:app-cpp-size

metrics:
  app.cpp LOC: 1430 -> 392
  App:: methods: 34 -> 9
  main_menu/window_layout/window_registry LOC: 0 -> 870
```

---

*W7 완료 시 Phase 11 DoD 4번이 지연 달성된다. `app.cpp`는 "작은 부팅/정리 셸"이 되고, UI 책임은 모두 shell presentation 레이어로 귀속된다.*
