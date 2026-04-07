# Phase 15 Platform Boundary Inventory (W0 Baseline + W3 Update)

- 기준 계획서: `docs/refactoring/phase15/refactoring_phase15_render_platform_port_isolation_260407.md`
- 범위: `webassembly/src/platform/*`, `webassembly/src/io/platform/*`, 관련 호출 지점

## 1) W0 기준선 (`2026-04-07`, KST)

1. `webassembly/src/platform` 하위 디렉터리: `wasm`만 존재
2. browser adapter 위치: `webassembly/src/io/platform/browser_file_picker.*`
3. `BrowserFilePicker` 총 참조 수: `13`
   - `webassembly/src/io/platform/browser_file_picker.cpp`: 6
   - `webassembly/src/file_loader.cpp`: 6
   - `webassembly/src/io/platform/browser_file_picker.h`: 1

## 2) W3 적용 내용 (`2026-04-07`, KST)

1. browser adapter를 `io/platform`에서 `platform/browser`로 이동:
   - 추가: `webassembly/src/platform/browser/browser_file_picker.h`
   - 추가: `webassembly/src/platform/browser/browser_file_picker.cpp`
   - 삭제: `webassembly/src/io/platform/browser_file_picker.h`
   - 삭제: `webassembly/src/io/platform/browser_file_picker.cpp`
2. 네임스페이스를 `io::platform`에서 `platform::browser`로 정렬했다.
3. `FileLoader` 호출부와 빌드 소스 목록을 새 경계로 갱신했다.
   - `webassembly/src/file_loader.cpp`
   - `webassembly/cmake/modules/wb_io.cmake`

## 3) W3 재계측 결과 (`2026-04-07`, KST)

| 항목 | W0 | W3 | Delta |
|---|---:|---:|---:|
| `platform` 하위 디렉터리 수 | 1 (`wasm`) | 2 (`wasm`, `browser`) | +1 |
| `io/platform` adapter 파일 수 | 2 | 0 | -2 |
| `platform/browser` adapter 파일 수 | 0 | 2 | +2 |
| `BrowserFilePicker` 총 참조 수 | 13 | 13 | 0 |
| `io::platform::BrowserFilePicker` 참조 수 | 6 | 0 | -6 |
| `platform::browser::BrowserFilePicker` 참조 수 | 0 | 6 | +6 |

W3 파일별 분포:
1. `webassembly/src/platform/browser/browser_file_picker.cpp`: 6
2. `webassembly/src/file_loader.cpp`: 6
3. `webassembly/src/platform/browser/browser_file_picker.h`: 1

## 4) W3 메모

1. browser adapter 경계는 계획대로 `platform` 중심으로 정렬되었다.
2. `FileLoader`의 런타임 진입 계약은 유지되어 사용자 동작 경로는 변경하지 않았다.
3. `platform/worker`, `platform/persistence`는 Phase 15 후속 작업(W6 이후 개선계획) 대상으로 유지한다.
