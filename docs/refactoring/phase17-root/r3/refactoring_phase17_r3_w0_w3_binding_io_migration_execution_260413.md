# Phase 17-R3 실행 보고서 (W0~W3)

- 작성일: `2026-04-13`
- 기준 계획서:
  - `docs/refactoring/refactoring_plan_root_legacy_dismantle_260408_53Codex.md`
  - `docs/refactoring/phase17-root/r3/refactoring_phase17_r3_render_platform_io_dismantle_execution_plan_260413.md`

## 1. 실행 범위

본 보고서는 R3의 `W0~W3` 범위만 다룬다.

1. W0: 인벤토리/의존 기준선 고정
2. W1: `bind_function.cpp` 제거 및 wasm 단일 진입점 확인
3. W2: `unv_reader.*` 이관 + 진행률 콜백 주입
4. W3: `file_loader.*` 이관 + chunk I/O/ImGui popup 분리

## 2. 코드 해체/이관 내역

### 2.1 W1

- 삭제:
  - `webassembly/src/bind_function.cpp`
- 확인:
  - wasm 바인딩 진입점은 `webassembly/src/platform/wasm/workbench_bindings.cpp`만 유지

### 2.2 W2 (`unv_reader` 계열)

- 파일 이관:
  - `webassembly/src/unv_reader.h` -> `webassembly/src/io/infrastructure/unv/unv_mesh_reader.h`
  - `webassembly/src/unv_reader.cpp` -> `webassembly/src/io/infrastructure/unv/unv_mesh_reader.cpp`
- 구조 개선:
  - `UnvReader::SetProgressCallback(...)` 추가
  - `unv_mesh_reader.cpp`의 `GetWorkbenchRuntime().SetProgress(...)` 직접 호출 제거
  - 진행률 갱신은 `import_entry_service.cpp`에서 콜백 주입으로 연결
- 정리:
  - `unv_mesh_reader.cpp`의 테스트/뷰어 직접 의존 include 제거 상태 유지

### 2.3 W3 (`file_loader` 계열)

- 파일 이관:
  - `webassembly/src/file_loader.h` -> `webassembly/src/io/application/import_entry_service.h`
  - `webassembly/src/file_loader.cpp` -> `webassembly/src/io/application/import_entry_service.cpp`
- chunk I/O 분리:
  - 신규: `webassembly/src/io/platform/memfs_chunk_stream.h`
  - 신규: `webassembly/src/io/platform/memfs_chunk_stream.cpp`
  - `WriteChunk`, `CloseFile`, `FILE*` 맵 책임을 `MemfsChunkStream`으로 분리
- ImGui popup 분리:
  - 신규: `webassembly/src/shell/presentation/import/import_popup_presenter.h`
  - 신규: `webassembly/src/shell/presentation/import/import_popup_presenter.cpp`
  - `RenderXsfGridImportPopups`의 ImGui 렌더링 책임을 presenter로 이동
  - `import_entry_service.cpp`에서 ImGui 직접 사용 제거

### 2.4 CMake 경로 갱신

- `webassembly/cmake/modules/wb_io.cmake`
  - `import_entry_service.cpp`, `unv_mesh_reader.cpp` 경로 반영
  - `io/platform/memfs_chunk_stream.cpp` 추가
- `webassembly/cmake/modules/wb_ui.cmake`
  - `shell/presentation/import/import_popup_presenter.cpp` 추가

## 3. W0~W3 완료 조건 점검

1. 루트 `bind_function.cpp` 0개: `PASS`
2. 루트 `unv_reader.*` 0개: `PASS`
3. 루트 `file_loader.*` 0개: `PASS`
4. `unv_reader` 진행률 갱신 콜백 주입: `PASS`
5. `file_loader` 계열 ImGui 직접 의존 제거(`import_entry_service.*`): `PASS`
6. R2 회귀 게이트 유지: `PASS`

## 4. 검증 결과

### 4.1 게이트

1. `check_phase17_root_allowlist.ps1`: PASS
2. `check_phase17_root_ownership_map.ps1`: PASS
3. `check_phase17_facade_contract.ps1`: PASS
4. `check_phase17_r2_shell_ui_migration.ps1`: PASS

로그:
1. `docs/refactoring/phase17-root/r3/logs/check_phase17_root_allowlist_r3_w0_w3_latest.txt`
2. `docs/refactoring/phase17-root/r3/logs/check_phase17_root_ownership_map_r3_w0_w3_latest.txt`
3. `docs/refactoring/phase17-root/r3/logs/check_phase17_facade_contract_r3_w0_w3_latest.txt`
4. `docs/refactoring/phase17-root/r3/logs/check_phase17_r2_shell_ui_migration_r3_w0_w3_latest.txt`

### 4.2 빌드/테스트

1. `npm run build-wasm:release`: PASS
2. `npm run test:cpp`: PASS
3. `npm run test:smoke`: PASS

로그:
1. `docs/refactoring/phase17-root/r3/logs/build_phase17_r3_w0_w3_latest.txt`
2. `docs/refactoring/phase17-root/r3/logs/test_cpp_phase17_r3_w0_w3_latest.txt`
3. `docs/refactoring/phase17-root/r3/logs/test_smoke_phase17_r3_w0_w3_latest.txt`

## 5. 결론

- R3 `W0~W3` 범위는 계획서 기준으로 실행 완료.
- 판정: `GO (W4 진행 가능)`

