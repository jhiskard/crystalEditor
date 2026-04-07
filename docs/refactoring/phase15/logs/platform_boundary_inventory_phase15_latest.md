# Phase 15 W0 Platform Boundary Inventory Baseline

- 측정 시각: `2026-04-07 (KST)`
- 범위: `webassembly/src/platform/*`, `webassembly/src/io/platform/*`, 관련 호출 지점
- 기준 계획서: `docs/refactoring/phase15/refactoring_phase15_render_platform_port_isolation_260407.md`

## 1) 디렉터리 기준선

현재 `webassembly/src/platform` 하위 디렉터리:
1. `wasm`

기준선 메모:
1. `platform/browser`, `platform/worker`, `platform/persistence`는 미구성 상태다.
2. browser adapter는 현재 `io/platform`에 위치한다.

## 2) adapter 파일 분포 기준선

`io/platform` 파일:
1. `webassembly/src/io/platform/browser_file_picker.h`
2. `webassembly/src/io/platform/browser_file_picker.cpp`

파일 수: `2`

## 3) BrowserFilePicker 참조 기준선

검색 패턴:
- `BrowserFilePicker`
- `io::platform::BrowserFilePicker`

총 참조 수: `13`

파일별 분포:
1. `webassembly/src/io/platform/browser_file_picker.cpp`: 6
2. `webassembly/src/file_loader.cpp`: 6
3. `webassembly/src/io/platform/browser_file_picker.h`: 1

## 4) W0 메모

1. Phase 15 W3에서 browser adapter 경계를 `platform` 중심으로 재정렬한다.
2. `FileLoader`의 호출 계약은 유지하면서 위치/의존만 정리한다.
