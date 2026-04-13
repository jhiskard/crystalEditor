# Phase 17-R3 실행 보고서 (W4~W6)

- 작성일: `2026-04-13`
- 기준 계획서:
  - `docs/refactoring/refactoring_plan_root_legacy_dismantle_260408_53Codex.md`
  - `docs/refactoring/phase17-root/r3/refactoring_phase17_r3_render_platform_io_dismantle_execution_plan_260413.md`

## 1. 실행 범위

본 보고서는 R3의 `W4~W6` 범위를 다룬다.

1. W4: Platform/IO 포트-어댑터 고정
2. W5: Viewer 해체 1차 (렌더 경계 분리)
3. W6: Persistence/호환 경로 정리

## 2. 코드 변경 내역

### 2.1 W4 (포트/어댑터 고정)

신규 포트:
1. `webassembly/src/io/application/file_dialog_port.h`
2. `webassembly/src/io/application/worker_port.h`
3. `webassembly/src/io/application/progress_port.h`

신규 어댑터:
1. `webassembly/src/platform/browser/browser_file_dialog_adapter.h`
2. `webassembly/src/platform/browser/browser_file_dialog_adapter.cpp`
3. `webassembly/src/platform/worker/emscripten_worker_port.h`
4. `webassembly/src/platform/worker/emscripten_worker_port.cpp`
5. `webassembly/src/platform/worker/runtime_progress_port.h`
6. `webassembly/src/platform/worker/runtime_progress_port.cpp`

적용 파일:
1. `webassembly/src/io/application/import_entry_service.h`
2. `webassembly/src/io/application/import_entry_service.cpp`
3. `webassembly/src/io/application/parser_worker_service.h`
4. `webassembly/src/io/application/parser_worker_service.cpp`
5. `webassembly/src/shell/runtime/workbench_runtime.cpp`
6. `webassembly/cmake/modules/wb_io.cmake`

핵심 변경:
1. `import_entry_service`에서 `BrowserFilePicker` 직접 호출 제거
2. `import_entry_service`, `parser_worker_service`에서 `emscripten/threading` 직접 호출 제거
3. 진행률 업데이트를 `ProgressPort` 경유로 통일
4. 메인 스레드 디스패치를 `WorkerPort` 경유로 통일

### 2.2 W5 (viewer/interactor 파일 해체 이관)

루트 해체 이관:
1. `webassembly/src/vtk_viewer.h` -> `webassembly/src/render/presentation/viewer_window.h`
2. `webassembly/src/vtk_viewer.cpp` -> `webassembly/src/render/presentation/viewer_window.cpp`
3. `webassembly/src/mouse_interactor_style.h` -> `webassembly/src/render/infrastructure/interaction/mouse_interactor_style.h`
4. `webassembly/src/mouse_interactor_style.cpp` -> `webassembly/src/render/infrastructure/interaction/mouse_interactor_style.cpp`

참조 경로 정리:
1. `webassembly/src/app.cpp`
2. `webassembly/src/render/infrastructure/vtk_render_gateway.cpp`
3. `webassembly/src/render/application/camera_align_service.cpp`
4. `webassembly/src/render/application/viewer_interaction_controller.cpp`
5. `webassembly/cmake/modules/wb_render.cmake`

### 2.3 W6 (persistence/호환 경로 정리)

신규 persistence 어댑터:
1. `webassembly/src/platform/persistence/viewer_preferences_store.h`
2. `webassembly/src/platform/persistence/viewer_preferences_store.cpp`

적용 파일:
1. `webassembly/src/render/presentation/viewer_window.cpp`
2. `webassembly/cmake/modules/wb_render.cmake`

핵심 변경:
1. viewer 배경색 저장/로드의 `EM_ASM(localStorage)` 직접 호출을 `ViewerPreferencesStore`로 이관
2. `saveBgColorToLocalStorage`, `loadBgColorFromLocalStorage`는 persistence 어댑터 호출 래퍼로 축소

## 3. 완료 기준 점검

### 3.1 W4 완료 기준

1. `io application`에서 browser/emscripten 직접 include: `PASS (0)`

### 3.2 W5 완료 기준

1. 루트 `vtk_viewer.*`, `mouse_interactor_style.*`: `PASS (0)`
2. render gateway 루트 경로 include 제거 후 빌드 성공: `PASS`

### 3.3 W6 완료 기준

1. `render/application`에서 `EM_ASM` 직접 사용: `PASS (0)`
2. `shell/runtime/workbench_runtime.cpp`의 루트 render/io include: `PASS (0)`

## 4. 검증 결과

게이트:
1. `check_phase17_root_allowlist.ps1`: PASS
2. `check_phase17_root_ownership_map.ps1`: PASS
3. `check_phase17_facade_contract.ps1`: PASS
4. `check_phase17_r2_shell_ui_migration.ps1`: PASS

로그:
1. `docs/refactoring/phase17-root/r3/logs/check_phase17_root_allowlist_r3_w4_w6_latest.txt`
2. `docs/refactoring/phase17-root/r3/logs/check_phase17_root_ownership_map_r3_w4_w6_latest.txt`
3. `docs/refactoring/phase17-root/r3/logs/check_phase17_facade_contract_r3_w4_w6_latest.txt`
4. `docs/refactoring/phase17-root/r3/logs/check_phase17_r2_shell_ui_migration_r3_w4_w6_latest.txt`

빌드/테스트:
1. `npm run build-wasm:release`: PASS
2. `npm run test:cpp`: PASS
3. `npm run test:smoke`: PASS

로그:
1. `docs/refactoring/phase17-root/r3/logs/build_phase17_r3_w4_w6_latest.txt`
2. `docs/refactoring/phase17-root/r3/logs/test_cpp_phase17_r3_w4_w6_latest.txt`
3. `docs/refactoring/phase17-root/r3/logs/test_smoke_phase17_r3_w4_w6_latest.txt`

## 5. 결론

- R3 `W4~W6` 범위는 계획서 기준으로 실행 완료.
- 판정: `GO (W7 진행 가능)`

