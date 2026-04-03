# Phase 10 Inventory Snapshot (W0~W5)

- Snapshot date: `2026-04-03 (KST)`
- Branch: `refactor/phase10-workspace-mesh-io-decouple`
- Scope: W0 baseline + W1 workspace + W2 mesh repository + W3 import workflow + W4 panel context sync + W5 static gate

## 1) File/Module Presence

| Item | Value |
|---|---|
| `webassembly/src/workspace/domain/workspace_store.h` | `True` |
| `webassembly/src/workspace/application/workspace_query_service.h` | `True` |
| `webassembly/src/workspace/application/workspace_command_service.h` | `True` |
| `webassembly/src/mesh/domain/mesh_repository.h` | `True` |
| `webassembly/src/mesh/application/mesh_query_service.h` | `True` |
| `webassembly/src/mesh/application/mesh_command_service.h` | `True` |
| `webassembly/src/io/application/import_workflow_service.h` | `True` |
| `scripts/refactoring/check_phase10_workspace_mesh_io_decouple.ps1` | `True` |

## 2) Singleton Call Baseline (cpp 기준)

| Metric | Count |
|---|---:|
| `MeshManager::Instance()` in `webassembly/src/**/*.cpp` | 66 |
| `FileLoader::Instance()` in `webassembly/src/**/*.cpp` | 3 |
| `MeshManager::Instance()` in `model_tree + mesh_group_detail + model_tree sections` | 0 |
| `MeshManager::Instance()` in `mesh_detail.cpp` | 32 |
| `MeshManager::Instance()` in `import_orchestrator.cpp + import_workflow_service.cpp` | 0 |
| `FileLoader::Instance()` in panel 파일(`app/toolbar/model_tree/mesh_detail/mesh_group_detail`) | 0 |

## 3) W3/W4 전환 포인트

1. `workspace` context store/query/command 계약을 신규 도입했다.
2. `mesh` repository/query/command 계약을 신규 도입했다.
3. `ImportWorkflowService`를 도입해 replace-scene 트랜잭션을 `FileLoader`에서 분리했다.
4. 아래 호출 경로를 `MeshManager::Instance()` 직접 호출에서 `mesh::application` 서비스 경유로 전환했다.
   - `webassembly/src/io/application/import_orchestrator.cpp`
   - `webassembly/src/io/application/import_workflow_service.cpp`
   - `webassembly/src/model_tree.cpp` (workspace selected mesh 문맥 동기화)
   - `webassembly/src/mesh_group_detail.cpp`
   - `webassembly/src/mesh/presentation/model_tree_mesh_section.cpp`
   - `webassembly/src/mesh/presentation/model_tree_structure_section.cpp`
   - `webassembly/src/mesh/presentation/model_tree_dialogs.cpp`

## 4) Build/Test/Static Gate (W3~W5 범위)

- Command: `powershell -ExecutionPolicy Bypass -File scripts/refactoring/check_phase10_workspace_mesh_io_decouple.ps1`
- Result: `PASS`
- Command: `npm run test:cpp`
- Result: `PASS` (`1/1` tests passed)
- Command: `npm run build-wasm:release`
- Result: `PASS`

## 5) Notes

- W5 시점 기준 `model_tree` 경로와 import orchestration 경로의 `MeshManager::Instance()` 직접 호출은 제거되었다.
- 남은 direct singleton 호출의 대부분은 `mesh_detail.cpp`, `atoms_template.cpp`, `import_apply_service.cpp`, `file_loader.cpp`에 집중되어 있으며 W6 이후 단계에서 추가 축소 대상으로 유지한다.
