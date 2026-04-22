# Phase 19 / W6 — Runtime Singleton 제거

작성일: `2026-04-21 (KST)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_full_dismantle_claude2_260421.md`
검증보고서 근거: 6.2 잔여 과제 (`VtkViewer`/`MeshManager`/`FontManager`), 5.2 Low 권고
우선순위: **Low** (단, DoD 봉인을 위해 반드시 필요)
예상 소요: 2 ~ 3일
선행 의존: W5 (AtomsTemplate 해체 완료 — 각 서비스가 Repository/Store를 직접 소유한 상태)
후속 작업: W8 (게이트 봉인)

---

## 1. 배경

독립 검증보고서 6.2절은 `DECLARE_SINGLETON` 매크로 사용 3건을 "잔여 과제"로 남겼다:

1. `VtkViewer` — render 레이어의 VTK 렌더 윈도우
2. `MeshManager` — 메시/구조 저장소
3. `FontManager` — ImGui 폰트 레지스트리

이들은 Phase 7의 composition root 도입 이후에도 `Instance()` 호출을 통해 전역 접근되어 왔으며, `WorkbenchRuntime`이 단일 인스턴스를 소유함에도 매크로 기반 static accessor가 병존한다. Phase 19는 이를 완전 제거한다.

## 2. 현재 상태

| Singleton | 정의 위치 | `Instance()` 호출 수 (추정) |
|---|---|---:|
| `VtkViewer` | `render/presentation/viewer_window.h` | ~15 |
| `MeshManager` | `mesh/domain/mesh_repository_core.h` | ~20 |
| `FontManager` | `shell/presentation/font/font_registry.h` | ~13 |
| **합계** | 3클래스 | ~48 |

## 3. 서브 단계

### W6.1 — `VtkViewer`

#### 3.1 현황

- `DECLARE_SINGLETON(VtkViewer)` 매크로로 static accessor 정의
- `WorkbenchRuntime::viewer()` 이미 단일 인스턴스 소유 (사실상 중복)
- render module 외부에서 `VtkViewer::Instance()` 호출 → render 경계 누설

#### 3.2 설계

```cpp
// webassembly/src/render/presentation/viewer_window.h
// 변경 전
class VtkViewer {
    DECLARE_SINGLETON(VtkViewer)
    // ...
};
// 변경 후
class VtkViewer {
    // singleton 제거, ctor public
public:
    VtkViewer();
    // ...
};
```

- `WorkbenchRuntime`의 `VtkViewer` 멤버 소유는 그대로
- render module 내부에서는 `VtkViewer&` reference로 전달
- render module **외부** 호출은 Gateway 경유로 이미 전환되어 있어야 함 (Phase 15 DoD)

#### 3.3 호출부 조정

- `VtkViewer::Instance()` 호출 각 1건을 `WorkbenchRuntime::viewer()` 또는 인자 주입 `VtkViewer&`로 교체
- 외부 누설이 발견되면 Gateway API에 메서드 추가 (Phase 15 설계 확장)

### W6.2 — `MeshManager`

#### 3.4 현황

- `DECLARE_SINGLETON(MeshManager)` 매크로
- `WorkbenchRuntime::meshRepository()` 기 존재 (wrapper)
- `MeshCommandService`, `MeshQueryService`가 현재 어떤 경로로 MeshManager에 접근하는지 확인 필요 (Phase 10 이후 개선 여지)

#### 3.5 설계

```cpp
// webassembly/src/mesh/domain/mesh_repository_core.h
// 변경 전
class MeshManager {
    DECLARE_SINGLETON(MeshManager)
};
// 변경 후
class MeshManager {
public:
    MeshManager();
};
```

- `MeshCommandService`와 `MeshQueryService`의 생성자에 `MeshManager&` 주입
- `WorkbenchRuntime`이 `MeshManager`를 소유하고 두 서비스에게 참조 전달

### W6.3 — `FontManager`

#### 3.6 현황

- `DECLARE_SINGLETON(FontManager)` 매크로
- `app.cpp`의 font scaling 로직 및 `shell/presentation/font/font_scale_controller`(W7에서 신설)가 접근 대상
- ImGui 폰트 atlas는 ImGui context 수명과 생명주기가 강결합되어 있어 신중한 처리 필요

#### 3.7 설계

```cpp
// webassembly/src/shell/presentation/font/font_registry.h
class FontManager {
public:
    FontManager(); // ImGui::CreateContext() 이후 호출 필요
};
```

- `WorkbenchRuntime::fontRegistry()` 추가 — 생성 타이밍은 App 초기화에서 ImGui context 생성 직후
- `FontManager::Instance()` 호출부를 전부 `WorkbenchRuntime::fontRegistry()`로 교체
- `app.cpp`의 `applyFontScale`이 W7.5에서 `FontScaleController`로 이관되면 해당 컨트롤러가 `FontManager&`를 주입받음

### W6.4 — `macro/singleton_macro.h` 처리

#### 3.8 선택지

| 옵션 | 내용 |
|---|---|
| A | 호출부 0 확인 후 매크로 정의 **삭제** |
| B | 매크로 정의 유지하되 `[[deprecated("do not use — Phase 19 closed")]]` 주석 + `check_phase19_singleton_zero.ps1`에서 매크로 **사용** 0 검증 |

권장: **A (매크로 정의 삭제)**. 기존 `#include "macro/singleton_macro.h"`를 포함한 헤더가 있다면 제거. CI 게이트에서 `DECLARE_SINGLETON\s*\(` grep 결과 0을 검증.

## 4. 실행 순서

1. **인벤토리**: `DECLARE_SINGLETON`/`::Instance()` 호출 전수 리스트
   산출물: `docs/refactoring/phase19/logs/singleton_call_sites_phase19_latest.md`
2. **W6.1 VtkViewer**: 매크로 제거 + 호출부 전수 전환 (render module 내부 reference, render 외부는 Gateway 경유 — 만약 누설 발견 시 W2/W3 잔여분 보강)
3. **W6.2 MeshManager**: 매크로 제거 + `MeshCommandService`/`MeshQueryService` 생성자 주입 전환 + `WorkbenchRuntime` 조립 갱신
4. **W6.3 FontManager**: 매크로 제거 + `WorkbenchRuntime::fontRegistry()` 도입 + App 초기화 순서 확인 (ImGui context 생성 직후 FontManager 생성)
5. **W6.4 매크로 삭제**: `macro/singleton_macro.h` 삭제 또는 deprecated 처리 + 의존 include 제거
6. **게이트 스크립트**: `scripts/refactoring/check_phase19_singleton_zero.ps1` 추가
7. **회귀**:
   - `npm run build:full`
   - e2e smoke
   - 수동: 최초 기동 시 폰트 렌더 확인, VTK 렌더 확인, 파일 임포트 시 메시 저장 확인

## 5. 파일 변경 명세

| 경로 | 변경 유형 |
|---|---|
| `webassembly/src/render/presentation/viewer_window.h` | 수정 (매크로 제거) |
| `webassembly/src/render/presentation/viewer_window.cpp` | 수정 (ctor 공개) |
| `webassembly/src/mesh/domain/mesh_repository_core.h` | 수정 |
| `webassembly/src/mesh/domain/mesh_repository_core.cpp` | 수정 |
| `webassembly/src/mesh/application/mesh_command_service.{h,cpp}` | 수정 (ctor 주입) |
| `webassembly/src/mesh/application/mesh_query_service.{h,cpp}` | 수정 (ctor 주입) |
| `webassembly/src/shell/presentation/font/font_registry.h` | 수정 |
| `webassembly/src/shell/presentation/font/font_registry.cpp` | 수정 |
| `webassembly/src/workspace/runtime/workbench_runtime.h` | 수정 (`fontRegistry()` 추가) |
| `webassembly/src/workspace/runtime/workbench_runtime.cpp` | 수정 (조립) |
| `webassembly/src/macro/singleton_macro.h` | **삭제** (옵션 A) |
| `webassembly/src/app.cpp` | 수정 (호출부 전환, W7에서 더 큰 리팩토링 예정이므로 본 항목은 최소 변경) |
| `scripts/refactoring/check_phase19_singleton_zero.ps1` | **신규** |
| `package.json` | 수정 |
| `docs/refactoring/phase19/logs/singleton_call_sites_phase19_latest.md` | **신규** |
| `docs/refactoring/phase19/logs/dependency_gate_report_W6.md` | **신규** |

기타 `::Instance()` 호출이 있는 개별 파일들 — 인벤토리 로그에 따라 수정.

## 6. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| `DECLARE_SINGLETON\s*\(` 호출 | grep (정의 외) | 0 |
| `::Instance()` 호출 | grep (webassembly/src) | 0 |
| `macro/singleton_macro.h` 파일 | 존재 여부 | (옵션 A) 없음 / (옵션 B) 정의 유지하되 사용 0 |
| `WorkbenchRuntime::fontRegistry()` | 존재 | 있음 |
| `npm run build:full` | build | PASS |
| e2e smoke (font/mesh/render) | Playwright | PASS |
| 수동 UI 폰트 스케일 동작 | 수작업 | PASS |

## 7. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| `FontManager`를 non-singleton으로 바꿀 때 ImGui context 수명과 타이밍 어긋남 | 폰트 크래시 | `WorkbenchRuntime`에서 FontManager는 `std::optional` 또는 지연 생성(`std::unique_ptr` + ImGui context 생성 후 `emplace`) |
| `VtkViewer::Instance()` 호출이 render 외부에 누설된 상태 발견 | Phase 15 게이트 무효화 | W6.1 착수 전 인벤토리에서 render 외부 호출을 모두 식별 + Gateway API 확장으로 선조치 |
| `MeshManager` 생성 타이밍이 파일 임포트 전이어야 함 | 임포트 실패 | `WorkbenchRuntime` 조립 순서를 `MeshManager` → `StructureRepository` → `ImportService` 순으로 유지 |
| 매크로 삭제 후 남은 include가 빌드 에러 유발 | 빌드 실패 | `singleton_macro.h` 삭제 전 grep `#include.*singleton_macro` 전수 제거 |

## 8. 산출물

- 삭제 파일 1개 (`singleton_macro.h` — 옵션 A)
- 수정 파일 ~12개
- 신규 게이트 스크립트 1개
- 로그 문서 2종

## 9. Git 커밋 템플릿

```
refactor(runtime): phase19/W6.1 — remove VtkViewer singleton

- Drop DECLARE_SINGLETON from viewer_window.h
- VtkViewer ctor made public, instance owned by WorkbenchRuntime::viewer()
- Update ~15 call sites to use WorkbenchRuntime::viewer() or injected ref

metrics:
  VtkViewer::Instance() calls: 15 -> 0

verif:
  check:phase15:render-platform-isolation: PASS
  e2e workbench-smoke: PASS
```

```
refactor(mesh): phase19/W6.2 — remove MeshManager singleton
...
```

```
refactor(shell): phase19/W6.3 — remove FontManager singleton
...
```

```
chore(macro): phase19/W6.4 — drop singleton_macro.h (no remaining usage)
```

---

*W6 완료 시 composition root 원칙이 **완전 관철**된 상태가 된다. 이후 코드베이스에서 static 전역 인스턴스는 어떤 형태로도 존재하지 않는다.*
