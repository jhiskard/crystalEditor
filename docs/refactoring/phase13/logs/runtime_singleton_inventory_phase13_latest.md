# Phase 13 W0 Runtime/Singleton Inventory Baseline

- 측정 시각: `2026-04-06 (KST)`
- 범위: `webassembly/src/**/*.cpp`, `webassembly/src/**/*.h`
- 기준 계획서: `docs/refactoring/phase13/refactoring_phase13_runtime_hard_seal_singleton_entrypoint_260406.md`

## 1) 핵심 singleton 선언 기준선

`DECLARE_SINGLETON` 실사용 선언:

1. `App` (`webassembly/src/app.h`)
2. `Toolbar` (`webassembly/src/toolbar.h`)
3. `FileLoader` (`webassembly/src/file_loader.h`)
4. `MeshManager` (`webassembly/src/mesh_manager.h`)
5. `AtomsTemplate` (`webassembly/src/atoms/atoms_template.h`)
6. `VtkViewer` (`webassembly/src/vtk_viewer.h`)
7. `FontManager` (`webassembly/src/font_manager.h`)

Count: `7`

## 2) `::Instance()` 호출 기준선 (`*.cpp`)

| 항목 | 값 |
|---|---:|
| 전체 `::Instance()` 호출 | 104 |
| `App::Instance()` | 1 |
| `Toolbar::Instance()` | 1 |
| `FileLoader::Instance()` | 3 |
| `MeshManager::Instance()` | 29 |
| `AtomsTemplate::Instance()` | 24 |
| `VtkViewer::Instance()` | 1 |

## 3) 아키텍처 게이트 연계 기준선

Phase 12 아키텍처 게이트(`check_phase12_architecture_seal_latest.txt`) 기준:

1. feature app/domain core singleton 호출(allowlist 외): `0`
2. legacy alias 외부 노출(`atoms/*` 외): `0`
3. module graph acyclic: `True`
4. bug log status: `Deferred`

## 4) W0 요약

1. Phase 13 핵심 목표는 `App/Toolbar/FileLoader` entrypoint 제거다.
2. 본 인벤토리는 W1~W6 단계별 감소 추적 기준으로 사용한다.

## 5) W1~W2 적용 후 재측정 (`2026-04-06`, KST)

적용 범위:
- `webassembly/src/app.h`
- `webassembly/src/app.cpp`
- `webassembly/src/toolbar.h`
- `webassembly/src/shell/runtime/workbench_runtime.h`
- `webassembly/src/shell/runtime/workbench_runtime.cpp`

재측정 결과:

| 항목 | W0 | W2 |
|---|---:|---:|
| `DECLARE_SINGLETON` 실사용 선언 수 | 7 | 5 |
| 전체 `::Instance()` 호출(`*.cpp`) | 104 | 102 |
| `App::Instance()` | 1 | 0 |
| `Toolbar::Instance()` | 1 | 0 |
| `FileLoader::Instance()` | 3 | 3 |

남은 `DECLARE_SINGLETON` 실사용 선언:
1. `FileLoader`
2. `MeshManager`
3. `AtomsTemplate`
4. `VtkViewer`
5. `FontManager`

WBS 상태:
1. `W1 (App singleton 제거/격리)`: 완료
2. `W2 (Toolbar singleton 제거/격리)`: 완료
3. 다음 단계 `W3`: `FileLoader::Instance()` 경로 제거

## 6) W3~W5 적용 후 재측정 (`2026-04-06`, KST)

적용 범위:
- `webassembly/src/file_loader.h`
- `webassembly/src/file_loader.cpp`
- `webassembly/src/shell/runtime/workbench_runtime.h`
- `webassembly/src/shell/runtime/workbench_runtime.cpp`
- `webassembly/src/platform/wasm/workbench_bindings.cpp`
- `scripts/refactoring/check_phase13_runtime_hard_seal.ps1`

재측정 결과:

| 항목 | W2 | W5 |
|---|---:|---:|
| `DECLARE_SINGLETON` 실사용 선언 수 | 5 | 4 |
| 전체 `::Instance()` 호출(`*.cpp`) | 102 | 99 |
| `App::Instance()` | 0 | 0 |
| `Toolbar::Instance()` | 0 | 0 |
| `FileLoader::Instance()` | 3 | 0 |

남은 `DECLARE_SINGLETON` 실사용 선언:
1. `MeshManager`
2. `AtomsTemplate`
3. `VtkViewer`
4. `FontManager`

게이트 결과:
1. `scripts/refactoring/check_phase13_runtime_hard_seal.ps1`: PASS
2. 로그: `docs/refactoring/phase13/logs/check_phase13_runtime_hard_seal_latest.txt`
