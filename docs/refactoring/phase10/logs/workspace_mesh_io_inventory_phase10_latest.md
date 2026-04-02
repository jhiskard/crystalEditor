# Phase 10 Inventory Snapshot (W0 Baseline, W1-W2 Applied)

- Snapshot date: `2026-04-02 (KST)`
- Branch: `refactor/phase10-workspace-mesh-io-decouple`
- Scope: W0 baseline measurement + W1 workspace contracts + W2 mesh repository transition

## 1) File/Module Presence

| Item | Value |
|---|---|
| `webassembly/src/workspace/domain/workspace_store.h` | `True` |
| `webassembly/src/workspace/application/workspace_query_service.h` | `True` |
| `webassembly/src/workspace/application/workspace_command_service.h` | `True` |
| `webassembly/src/mesh/domain/mesh_repository.h` | `True` |
| `webassembly/src/mesh/application/mesh_query_service.h` | `True` |
| `webassembly/src/mesh/application/mesh_command_service.h` | `True` |

## 2) Singleton Call Baseline (cpp 기준)

| Metric | Count |
|---|---:|
| `MeshManager::Instance()` in `webassembly/src/**/*.cpp` | 71 |
| `FileLoader::Instance()` in `webassembly/src/**/*.cpp` | 3 |
| `MeshManager::Instance()` in `webassembly/src/mesh/presentation/**/*.cpp` | 0 |
| `MeshManager::Instance()` in `model_tree/mesh_detail/mesh_group_detail` panel 파일 | 33 |
| `MeshManager::Instance()` in `webassembly/src/io/application/**/*.cpp` | 5 |
| `FileLoader::Instance()` in panel 파일(`app/toolbar/model_tree/mesh_detail/mesh_group_detail`) | 0 |

## 3) W1/W2 전환 포인트

1. `workspace` context store/query/command 계약을 신규 도입했다.
2. `mesh` repository/query/command 계약을 신규 도입했다.
3. 아래 호출 경로를 `MeshManager::Instance()` 직접 호출에서 `mesh::application` 서비스 경유로 전환했다.
   - `webassembly/src/io/application/import_orchestrator.cpp`
   - `webassembly/src/mesh/presentation/model_tree_mesh_section.cpp`
   - `webassembly/src/mesh/presentation/model_tree_structure_section.cpp`
   - `webassembly/src/mesh/presentation/model_tree_dialogs.cpp`

## 4) Build/Test Smoke (W0~W2 범위)

- Command: `npm run test:cpp`
- Result: `PASS` (`1/1` tests passed)

## 5) Notes

- W2 시점은 `MeshManager` 완전 제거 단계가 아니라 repository/service 경유 전환의 1차 단계다.
- 남은 direct singleton 호출은 W3/W4 범위(특히 `mesh_detail.cpp`, `mesh_group_detail.cpp`, `file_loader.cpp`, `io/application/import_apply_service.cpp`)에서 추가 축소한다.
