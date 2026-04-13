# Phase 17-R3 세부 작업계획서 (실질 코드 해체/이관 중심)

작성일: `2026-04-13`  
기준 문서: `docs/refactoring/refactoring_plan_root_legacy_dismantle_260408_53Codex.md`  
아키텍처 준수 기준: `docs/refactoring/refactoring_plan_full_modular_architecture_260331.md`의 `3.1~3.7`

## 0. 목적

R3의 목표인 `Render/Platform/IO 파일군 이관`을 실제 코드 해체 단위로 실행한다.

핵심 목표:
1. 루트 파일 `vtk_viewer.*`, `mouse_interactor_style.*`, `file_loader.*`, `unv_reader.*`, `bind_function.cpp`의 책임을 모듈 경계 내부(`render/*`, `platform/*`, `io/*`)로 분산한다.
2. `FileDialogPort`, `WorkerPort`, `ProgressPort`, `PersistencePort` 기준으로 포트/어댑터 경계를 코드 레벨에서 고정한다.
3. R2에서 도입된 shell facade 경로를 유지하면서 render/io/platform 루트 레거시 재유입을 차단한다.

전제:
1. R2에서 `image.*`, `texture.*`는 `render/infrastructure/resources/*`로 이관 완료 상태다.
2. wasm 바인딩 실경로는 이미 `platform/wasm/workbench_bindings.cpp`로 전환되어 있다.

---

## 1. 현재 기준선 (R3 착수 시점)

### 1.1 대상 파일 규모

| 파일 | 라인 수 | 현 위치 | 비고 |
|---|---:|---|---|
| `bind_function.cpp` | 6 | `webassembly/src` 루트 | 레거시 placeholder |
| `file_loader.cpp` | 501 | `webassembly/src` 루트 | io/shell/platform 책임 혼재 |
| `file_loader.h` | 108 | `webassembly/src` 루트 | import facade + UI popup API 포함 |
| `unv_reader.cpp` | 323 | `webassembly/src` 루트 | parser + 진행률 + 런타임 의존 |
| `unv_reader.h` | 77 | `webassembly/src` 루트 | UNV parser 인터페이스 |
| `vtk_viewer.cpp` | 2192 | `webassembly/src` 루트 | render/shell/atoms/persistence 책임 혼재 |
| `vtk_viewer.h` | 177 | `webassembly/src` 루트 | singleton + ImGui + VTK API 집약 |
| `mouse_interactor_style.cpp` | 120 | `webassembly/src` 루트 | interaction adapter |
| `mouse_interactor_style.h` | 16 | `webassembly/src` 루트 | VTK interactor style |

### 1.2 확인된 혼재/위반 포인트

1. `file_loader.cpp`는 `platform/browser`, `render gateway`, `runtime`, `ImGui`, `emscripten threading`을 동시에 직접 사용한다.
2. `unv_reader.cpp`는 parser 책임 외에 `GetWorkbenchRuntime().SetProgress(...)`를 직접 호출한다.
3. `vtk_viewer.cpp`는 `app.h`, `mesh_manager.h`, `atoms/atoms_template.h`, `EM_ASM`를 직접 포함하며 presentation/infra/feature 의존이 결합되어 있다.
4. `mouse_interactor_style.cpp`는 루트 파일로 남아 있고 render interaction adapter가 아직 경계 내부로 이동되지 않았다.
5. `render/infrastructure/vtk_render_gateway.cpp`는 `VtkViewer::Instance()`를 직접 호출하는 레거시 경유를 유지한다.

---

## 2. 파일 단위 해체/이관 매핑

## 2.1 필수 해체군 (R3 완료 기준에 반드시 포함)

| 원본 파일 | 목표 경로(1차) | 실질 해체 단위 | 완료 기준 |
|---|---|---|---|
| `webassembly/src/bind_function.cpp` | 삭제 (코드 0) | placeholder 제거, `platform/wasm/workbench_bindings.cpp` 단일 진입점 고정 | 루트 `bind_function.cpp` 삭제 |
| `webassembly/src/vtk_viewer.h/.cpp` | `render/presentation/viewer_window.h/.cpp` + `render/infrastructure/vtk_scene_adapter.*` + `render/infrastructure/vtk_framebuffer_adapter.*` + `render/application/viewer_input_router.*` | 렌더 윈도우/이벤트/UI/scene 제어 책임 분리 | 루트 `vtk_viewer.*` 삭제 |
| `webassembly/src/mouse_interactor_style.h/.cpp` | `render/infrastructure/interaction/mouse_interactor_style.h/.cpp` | 마우스 인터랙션 어댑터 이동 | 루트 `mouse_interactor_style.*` 삭제 |
| `webassembly/src/file_loader.h/.cpp` | `io/application/import_entry_service.h/.cpp` + `shell/presentation/import/import_popup_presenter.*` + `io/platform/memfs_chunk_stream.*` | import 흐름, UI popup, chunk file I/O 분리 | 루트 `file_loader.*` 삭제 |
| `webassembly/src/unv_reader.h/.cpp` | `io/infrastructure/unv/unv_mesh_reader.h/.cpp` | UNV 파싱/메시 변환 책임 분리 | 루트 `unv_reader.*` 삭제 |

## 2.2 필수 보강군 (R3에서 신규 도입)

| 신규 항목 | 위치 | 목적 |
|---|---|---|
| `FileDialogPort` | `common` 또는 `io/application` 포트 계층 | 파일 선택 호출의 platform 분리 |
| `WorkerPort` | `common` 또는 `io/application` 포트 계층 | 파싱 백그라운드 실행 분리 |
| `ProgressPort` | `common` 또는 `shell/application` 포트 계층 | 진행률/팝업 상태 갱신 분리 |
| `PersistencePort` | `platform/persistence` | viewer 배경색/localStorage 접근 분리 |
| `io/platform` adapter 구현 | `io/platform/*` | application에서 browser/worker 세부 구현 분리 |

---

## 3. 함수/책임 해체 내역 (실질 이관 단위)

### 3.1 `file_loader.cpp` 해체 단위

1. 브라우저 파일 선택 계열:
`OpenFileBrowser`, `OpenStructureFileBrowser`, `OpenXSFFileBrowser`, `OpenXSFGridFileBrowser`, `OpenChgcarFileBrowser`
-> `FileDialogPort` 호출만 남기고 구현은 `platform/browser` 어댑터로 이동.

2. 스트리밍/청크 파일 계열:
`WriteChunk`, `CloseFile`, `s_FileMap`
-> `io/platform/memfs_chunk_stream.*`로 분리.

3. 포맷 판독 계열:
`readVtkFile`, `readVtuFile`, `readUnvFile`
-> `io/infrastructure` 리더 어댑터로 분리 (`unv_mesh_reader` 포함).

4. 파서 워커/콜백 계열:
`process*InBackground`, `OnParserWorkerResult`, `handleParserWorkerResult`
-> `WorkerPort` + `ImportApplyService` 중심 오케스트레이션으로 재구성.

5. ImGui popup 계열:
`RenderXsfGridImportPopups`, 관련 상태 필드
-> `shell/presentation/import/import_popup_presenter.*`로 이동.

### 3.2 `unv_reader.cpp` 해체 단위

1. `ReadUnvFile`, `processNodeData`, `processElementData`, `processGroupData`
-> `io/infrastructure/unv/unv_mesh_reader.*`로 이동.

2. 진행률 갱신:
`GetWorkbenchRuntime().SetProgress(...)`
-> `ProgressPort` 콜백 주입 방식으로 치환.

3. 테스트 잔재 include:
`vtk_viewer.h`, 테스트용 VTK include 주석/잔재 제거.

### 3.3 `vtk_viewer.cpp` 해체 단위

1. Window/UI 렌더 계열:
`Render`, `RenderBgColorPopup`
-> `render/presentation/viewer_window.*`.

2. Scene/Camera/VTK 핸들 계열:
`AddActor/RemoveActor/AddVolume/SetProjectionMode/GetActiveCamera` 등
-> `render/infrastructure/vtk_scene_adapter.*`.

3. Framebuffer 계열:
`initFramebuffer`, `resizeFramebuffer`, `clearFramebuffer`
-> `render/infrastructure/vtk_framebuffer_adapter.*`.

4. Interaction 계열:
`processEvents`, drag-selection, wheel LOD
-> `render/application/viewer_input_router.*` + `render/infrastructure/interaction/*`.

5. LocalStorage/EM_ASM 계열:
`saveBgColorToLocalStorage`, `loadBgColorFromLocalStorage`
-> `platform/persistence/viewer_preferences_store.*`로 이동.

6. 임시 호환 경로:
R3 중간에는 `render/application/legacy_viewer_facade.*`를 유지하되, 직접 `VtkViewer::Instance()` 호출을 단계적으로 제거한다.

### 3.4 `mouse_interactor_style.cpp` 해체 단위

1. 클래스 파일 이동:
루트 -> `render/infrastructure/interaction`.

2. render gateway 호출 계약 유지:
`GetRenderGateway()` 경유만 허용하고 feature 직접 접근 금지.

### 3.5 `bind_function.cpp` 해체 단위

1. placeholder 파일 삭제.
2. wasm 바인딩 단일 진입:
`platform/wasm/workbench_bindings.cpp`만 유지.

---

## 4. 단계별 WBS (실행 순서 고정)

### W0. 인벤토리/의존 스냅샷 고정

작업:
1. R3 대상 파일/라인 수/참조 include 스냅샷 생성.
2. R2 게이트 PASS 기준선 재확인.

완료 기준:
1. `r3_inventory_snapshot_latest.md` 생성.

### W1. 바인딩 엔트리 봉인

작업:
1. `bind_function.cpp` 삭제.
2. `workbench_bindings.cpp` 단일 진입점 검증.

완료 기준:
1. 루트 `bind_function.cpp` 0개.

### W2. UNV 리더 이관

작업:
1. `unv_reader.*` -> `io/infrastructure/unv/unv_mesh_reader.*` 이동.
2. `ProgressPort` 콜백 방식 적용.
3. 테스트성 include 제거.

완료 기준:
1. 루트 `unv_reader.*` 0개.
2. 런타임 직접 의존 제거.

### W3. FileLoader 해체 (IO 중심)

작업:
1. `file_loader.*`를 `import_entry_service` 중심으로 이동.
2. 포맷 판독/청크 I/O/워커 콜백을 하위 모듈로 분해.

완료 기준:
1. 루트 `file_loader.*` 0개.
2. `file_loader` 클래스에서 ImGui 의존 제거.

### W4. Platform/IO 포트-어댑터 고정

작업:
1. `FileDialogPort`, `WorkerPort`, `ProgressPort` 인터페이스 도입.
2. `platform/browser`, `io/platform`, `platform/worker` 어댑터 연결.

완료 기준:
1. io application에서 browser/emscripten 직접 include 0.

### W5. Viewer 해체 1차 (렌더 경계 분리)

작업:
1. `vtk_viewer.*` 책임을 presentation/application/infrastructure로 분할.
2. `mouse_interactor_style.*`를 render/infrastructure 하위로 이동.

완료 기준:
1. 루트 `vtk_viewer.*`, `mouse_interactor_style.*` 0개.
2. render gateway가 루트 경로 include 없이 빌드.

### W6. Persistence/호환 경로 정리

작업:
1. viewer localStorage 접근을 `platform/persistence`로 이동.
2. `legacy_viewer_facade` 경유 경로를 최소화하고 runtime include 정리.

완료 기준:
1. `render/application`에서 `EM_ASM` 직접 사용 0.
2. `shell/runtime/workbench_runtime.cpp`의 루트 render/io include 제거.

### W7. CMake/게이트/문서 패키징

작업:
1. `wb_render.cmake`, `wb_io.cmake`, 관련 모듈 source 경로 업데이트.
2. 신규 게이트 스크립트 도입:
   - `scripts/refactoring/check_phase17_r3_render_platform_io_migration.ps1`
   - `scripts/refactoring/check_phase17_port_adapter_contract.ps1`
3. R3 결과보고서/게이트 보고서/GO-NO-GO 작성.

완료 기준:
1. R3 게이트 PASS.
2. R4 착수 문서 완료.

---

## 5. 코드 변경 파일 목록(예상)

필수 변경:
1. `webassembly/src/shell/runtime/workbench_runtime.h`
2. `webassembly/src/shell/runtime/workbench_runtime.cpp`
3. `webassembly/src/platform/wasm/workbench_bindings.cpp`
4. `webassembly/cmake/modules/wb_render.cmake`
5. `webassembly/cmake/modules/wb_io.cmake`

신규(예상):
1. `webassembly/src/render/presentation/viewer_window.*`
2. `webassembly/src/render/infrastructure/vtk_scene_adapter.*`
3. `webassembly/src/render/infrastructure/vtk_framebuffer_adapter.*`
4. `webassembly/src/render/infrastructure/interaction/mouse_interactor_style.*`
5. `webassembly/src/io/application/import_entry_service.*`
6. `webassembly/src/io/infrastructure/unv/unv_mesh_reader.*`
7. `webassembly/src/io/platform/*` (worker/browser bridge adapter)
8. `webassembly/src/platform/persistence/viewer_preferences_store.*`
9. `webassembly/src/shell/presentation/import/import_popup_presenter.*`

삭제(목표):
1. `webassembly/src/bind_function.cpp`
2. `webassembly/src/file_loader.*`
3. `webassembly/src/unv_reader.*`
4. `webassembly/src/vtk_viewer.*`
5. `webassembly/src/mouse_interactor_style.*`

---

## 6. 검증 계획

## 6.1 자동 검증

1. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_root_allowlist.ps1`
2. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_facade_contract.ps1`
3. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_r2_shell_ui_migration.ps1`
4. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_r3_render_platform_io_migration.ps1` (신규)
5. `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase17_port_adapter_contract.ps1` (신규)
6. `npm run build-wasm:release`
7. `npm run test:cpp`
8. `npm run test:smoke`

## 6.2 수동 회귀 포인트

1. `File > Open Structure File` 및 mesh import(`vtk/vtu/unv`) 경로
2. 구조/격자(`xsf`, `CHGCAR`) import 및 팝업 처리
3. Viewer 상호작용(줌/드래그/픽킹/LOD/카메라 정렬)
4. Toolbar 연동(투영, 리셋, 화살표 회전각, density 단순뷰)

---

## 7. 리스크 및 대응

| 리스크 | 수준 | 대응 |
|---|---|---|
| `vtk_viewer.cpp` 대형 파일 분해 중 회귀 | High | W5를 2단계로 분리(렌더링 코어 우선, UI/이벤트 후속) + 매 단계 빌드/스모크 |
| `file_loader.cpp`의 UI popup 분리 시 사용자 흐름 깨짐 | High | popup presenter 도입 후 기존 시나리오 수동 테스트 고정 |
| `unv_reader` 진행률 갱신 경로 변경 회귀 | Medium | `ProgressPort` 목/실구현 2중 테스트 추가 |
| 포트 인터페이스 도입 중 임시 우회 재유입 | High | `check_phase17_port_adapter_contract.ps1`에서 금지 include 강제 |
| `VtkViewer::Instance()` 잔존 | High | R3 게이트에서 render 외 직접 호출 0 강제 |

---

## 8. R3 완료 기준(실행판)

1. 루트 `bind_function.cpp`, `file_loader.*`, `unv_reader.*`, `vtk_viewer.*`, `mouse_interactor_style.*` 파일이 0개다.
2. render/io/platform 관련 루트 실질 구현 파일이 0개다.
3. `check_phase17_r3_render_platform_io_migration.ps1` PASS.
4. `check_phase17_port_adapter_contract.ps1` PASS.
5. `build-wasm`, `test:cpp`, `test:smoke` PASS.
6. R3 결과보고서/게이트 보고서/`go_no_go_r4.md`가 작성되어 있다.
