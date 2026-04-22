# Phase 19 / W4 — IO/Render legacy 제거

작성일: `2026-04-21 (KST)`
상위 계획서: `docs/refactoring/refactoring_plan_phase19_legacy_full_dismantle_claude2_260421.md`
검증보고서 근거: 5.1 R2 (`render/application/legacy_viewer_facade.*` 잔존), legacy 경로 일반
우선순위: **Medium**
예상 소요: 1 ~ 2일
선행 의존: W3 (feature 포트 내재화 완료)
후속 작업: W5 (AtomsTemplate 본체 해체)

---

## 1. 배경

`io/infrastructure/legacy/legacy_import_runtime_port.{h,cpp}` (169 LOC)와 `render/application/legacy_viewer_facade.{h,cpp}` (24 LOC)는 Phase 9/10에서 임시 도입된 **브릿지 파일**이다. 독립 검증보고서는 이들을 "composition root 한정 유지"로 판정했으나, 본 Phase 19에서는 물리 소거한다.

두 갈래(W4.1 = io, W4.2 = render)는 서로 독립이며 병렬 수행 가능하다.

## 2. 현재 상태

### 2.1 IO 사이드

| 파일 | LOC | 역할 |
|---|---:|---|
| `io/infrastructure/legacy/legacy_import_runtime_port.h` | 45 | `ImportRuntimePort` 추상 인터페이스 구현체 선언 |
| `io/infrastructure/legacy/legacy_import_runtime_port.cpp` | 124 | `LegacyAtomsRuntime()`을 통해 structure/density 등록 브릿지 |

사용처: `io/application/import_*_service.cpp`가 `ImportRuntimePort&`를 생성자 주입으로 받고, composition root에서 `LegacyImportRuntimePort` 구체를 주입.

### 2.2 Render 사이드

| 파일 | LOC | 역할 |
|---|---:|---|
| `render/application/legacy_viewer_facade.h` | 11 | `GetLegacyViewerFacade(VtkViewer&)` 함수 선언 |
| `render/application/legacy_viewer_facade.cpp` | 13 | `VtkViewer&` → 얇은 래퍼 반환 |

사용처:
- `shell/runtime/workbench_runtime.cpp` — composition root에서 `GetLegacyViewerFacade(viewer)` 1회 호출
- `render/infrastructure/vtk_render_gateway.{h,cpp}` — 2 파일에서 함수 호출

## 3. W4.1 — `io/infrastructure/legacy/` 제거

### 3.1 재배선 설계

현재:
```
io/application/import_workflow_service
    ↓ (ctor inject: ImportRuntimePort&)
io/application/import_runtime_port   (추상)
    ↑ (구현)
io/infrastructure/legacy/legacy_import_runtime_port  → LegacyAtomsRuntime() → AtomsTemplate::AddStructure 등
```

목표:
```
io/application/import_workflow_service
    ↓ (ctor inject: StructureRepository&, DensityRepository&)
structure/domain/structure_repository     (도메인)
density/domain/density_repository         (도메인)
```

### 3.2 세부 절차

1. **인벤토리**: `ImportRuntimePort`의 메서드 전수 리스트화 → `docs/refactoring/phase19/logs/import_runtime_port_surface_phase19_latest.md`
2. 각 메서드의 목표 호출처 매핑:
   - `AddStructure(Structure&&)` → `structure::domain::StructureRepository::add(Structure&&)`
   - `ReplaceStructure(int, Structure&&)` → `StructureRepository::replaceAt(int, Structure&&)`
   - `AttachChargeDensity(...)` → `density::domain::DensityRepository::attach(...)`
   - 기타 등
3. **`io/application/import_*_service.cpp`의 생성자 시그니처 변경**: `ImportRuntimePort&` → `StructureRepository&` + `DensityRepository&` (+필요 시 추가)
4. **`ImportRuntimePort` 추상 인터페이스 처리 결정**: Port 자체가 불필요해지면 `io/application/import_runtime_port.h` 삭제, 테스트 double 수요 있으면 유지. 결정 내용을 `docs/refactoring/phase19/logs/port_retention_decisions_W4_latest.md`에 기록.
5. **파일 삭제**:
   - `io/infrastructure/legacy/legacy_import_runtime_port.h`
   - `io/infrastructure/legacy/legacy_import_runtime_port.cpp`
   - 디렉터리 `io/infrastructure/legacy/` 제거
6. **CMake**: `webassembly/cmake/modules/wb_io.cmake`에서 엔트리 제거.
7. **`workbench_runtime.cpp` 조립 갱신**: `LegacyImportRuntimePort` 생성/주입 코드를 삭제.

## 4. W4.2 — `render/application/legacy_viewer_facade.*` 제거

### 4.1 재배선 설계

현재:
```
workbench_runtime.cpp     → GetLegacyViewerFacade(viewer) → VtkViewerLegacyFacade
vtk_render_gateway.{h,cpp}→ GetLegacyViewerFacade 호출
```

목표:
```
workbench_runtime.cpp     → WorkbenchRuntime::viewer() 직접 사용
vtk_render_gateway.{h,cpp}→ VtkViewer& 참조 직접 사용 (생성자 주입)
```

### 4.2 세부 절차

1. **`vtk_render_gateway.{h,cpp}`의 facade 의존 재배선**:
   - `VtkRenderGateway` 생성자에 `VtkViewer&` 주입
   - 내부에서 `GetLegacyViewerFacade(viewer)` 호출 제거
2. **`workbench_runtime.cpp`의 facade 호출 제거**: `GetLegacyViewerFacade(viewer)` → `viewer` 직접 참조.
3. **파일 삭제**:
   - `render/application/legacy_viewer_facade.h`
   - `render/application/legacy_viewer_facade.cpp`
4. **CMake**: `webassembly/cmake/modules/wb_render.cmake`에서 엔트리 제거.
5. **grep 검증**: `GetLegacyViewerFacade` 심볼 전역 0.

## 5. 파일 변경 명세

| 경로 | 변경 유형 | 비고 |
|---|---|---|
| `webassembly/src/io/infrastructure/legacy/legacy_import_runtime_port.h` | **삭제** | |
| `webassembly/src/io/infrastructure/legacy/legacy_import_runtime_port.cpp` | **삭제** | |
| `webassembly/src/io/infrastructure/legacy/` | **폴더 삭제** | |
| `webassembly/src/io/application/import_*_service.{h,cpp}` | 수정 | 생성자 서명, 구현 |
| `webassembly/src/io/application/import_runtime_port.h` | 수정 or 삭제 | retention 결정 |
| `webassembly/src/render/application/legacy_viewer_facade.h` | **삭제** | |
| `webassembly/src/render/application/legacy_viewer_facade.cpp` | **삭제** | |
| `webassembly/src/render/infrastructure/vtk_render_gateway.h` | 수정 | 생성자 서명 |
| `webassembly/src/render/infrastructure/vtk_render_gateway.cpp` | 수정 | facade 호출 제거 |
| `webassembly/src/workspace/runtime/workbench_runtime.cpp` | 수정 | 조립 갱신 |
| `webassembly/cmake/modules/wb_io.cmake` | 수정 | |
| `webassembly/cmake/modules/wb_render.cmake` | 수정 | |
| `docs/refactoring/phase19/logs/import_runtime_port_surface_phase19_latest.md` | **신규** | |
| `docs/refactoring/phase19/logs/port_retention_decisions_W4_latest.md` | **신규** | |
| `docs/refactoring/phase19/logs/dependency_gate_report_W4.md` | **신규** | |

## 6. 완료 기준 (DoD)

| 항목 | 측정 | 기준 |
|---|---|---|
| `webassembly/src/io/infrastructure/legacy/` 존재 | find | 없음 |
| `webassembly/src/render/application/legacy_viewer_facade.*` 존재 | find | 없음 |
| `GetLegacyViewerFacade` 심볼 | grep | 0 |
| `LegacyImportRuntimePort` 심볼 | grep | 0 |
| `npm run build-wasm:debug` | build | PASS |
| Import (XYZ/XSF/CHGCAR) e2e | Playwright | PASS |
| Render smoke e2e | Playwright | PASS |

## 7. 리스크와 완화

| 리스크 | 영향 | 완화책 |
|---|---|---|
| `ImportRuntimePort`가 사실상 facade(`AtomsTemplate`)의 특정 메서드 조합을 감싸고 있어 도메인 레이어 메서드 하나에 1:1 대응되지 않음 | 재배선 복잡 | 인벤토리 단계에서 이관 불가능한 메서드는 식별 후 `application/import_*_service.cpp`가 `LegacyAtomsRuntime()` 직접 호출하는 과도기 허용 (W5에서 자연 소멸) |
| `legacy_viewer_facade` 삭제 시 빌드 순환 참조 발견 | 빌드 실패 | `render/infrastructure/vtk_render_gateway`는 이미 `VtkViewer`에 의존하므로 직접 참조로 전환 문제 없음. 만약 cyclic 문제 발생 시 interface 분리 방안(`render/domain/render_view.h`) 고려 |
| composition root 조립 순서 변경 | 부팅 실패 | `workbench_runtime.cpp` 조립 변경은 별도 커밋으로 분리해 bisect 가능 |

## 8. 산출물

- 삭제 파일 4개 (io 2 + render 2)
- 삭제 폴더 1개 (`io/infrastructure/legacy/`)
- 수정 파일 8~10개
- 로그 문서 3종

## 9. Git 커밋 템플릿

```
refactor(io): phase19/W4.1 — remove io/infrastructure/legacy/*

- import_*_service.cpp now depends on StructureRepository & DensityRepository directly
- ImportRuntimePort retained as abstract (see retention log) / removed
- Drop LegacyImportRuntimePort + directory
- workbench_runtime.cpp composition updated

metrics:
  io/infrastructure/legacy/ files: 2 -> 0
  LegacyImportRuntimePort references: N -> 0

verif:
  e2e XYZ/XSF/CHGCAR import: PASS
```

```
refactor(render): phase19/W4.2 — remove legacy_viewer_facade bridge

- VtkRenderGateway now takes VtkViewer& via ctor injection
- workbench_runtime.cpp composition passes viewer() directly
- Drop render/application/legacy_viewer_facade.{h,cpp}

metrics:
  GetLegacyViewerFacade references: 3 -> 0
  render/application/legacy_*: 2 files -> 0

verif:
  e2e workbench-smoke: PASS
  render pipeline smoke: PASS
```

---

*W4 완료 시 `webassembly/src/**/legacy/**` 디렉터리 수는 4개(structure/measurement/density/io/infrastructure/legacy + render/application/legacy_viewer_facade는 디렉터리가 아니므로 3개 + workspace/legacy 1개 = 1개)로 축소된다. 남은 `workspace/legacy/`는 W5에서 완전 해체된다.*
