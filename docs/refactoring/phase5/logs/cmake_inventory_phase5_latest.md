# Phase 5 CMake Baseline Inventory

작성일: `2026-03-31 (KST)`  
기준 브랜치: `refactor/phase5-build-system-modularization`  
기준 계획: `docs/refactoring/phase5/refactoring_phase5_build_system_modularization_260331.md`

## 1. Baseline Snapshot
- 루트 `CMakeLists.txt` 라인 수(모듈화 전 실측): `271`
- 빌드 등록 C++ 컴파일 유닛 수: `65`
- 루트 엔트리포인트 수: `2`
  - `webassembly/src/main.cpp`
  - `webassembly/src/bind_function.cpp`
- 모듈 CMake 파일 수(모듈화 전): `0`

## 2. Module Ownership Summary
| 소유 모듈 | C++ 유닛 수 | 역할 |
|---|---:|---|
| `wb_core` | 3 | 공통 유틸과 기본 트리 자료구조 |
| `wb_render` | 5 | `VtkViewer`, 입력 처리, 카메라 정렬, 렌더 게이트웨이 |
| `wb_mesh` | 9 | Mesh/ModelTree/Detail 및 mesh presentation |
| `wb_atoms` | 31 | atoms domain/application/presentation/UI/renderer |
| `wb_io` | 8 | file import orchestration, parser worker, UNV/XSF/CHGCAR apply path |
| `wb_ui` | 7 | App shell, toolbar, font/image/texture, test window |
| `root executable` | 2 | main loop, emscripten bindings |

## 3. Module Ownership Detail

### `wb_core`
```text
webassembly/src/common/colormap.cpp
webassembly/src/common/string_utils.cpp
webassembly/src/lcrs_tree.cpp
```

### `wb_render`
```text
webassembly/src/vtk_viewer.cpp
webassembly/src/mouse_interactor_style.cpp
webassembly/src/render/application/camera_align_service.cpp
webassembly/src/render/application/viewer_interaction_controller.cpp
webassembly/src/render/infrastructure/vtk_render_gateway.cpp
```

### `wb_mesh`
```text
webassembly/src/mesh.cpp
webassembly/src/mesh_detail.cpp
webassembly/src/mesh_group.cpp
webassembly/src/mesh_group_detail.cpp
webassembly/src/mesh_manager.cpp
webassembly/src/model_tree.cpp
webassembly/src/mesh/presentation/model_tree_dialogs.cpp
webassembly/src/mesh/presentation/model_tree_mesh_section.cpp
webassembly/src/mesh/presentation/model_tree_structure_section.cpp
```

### `wb_atoms`
```text
webassembly/src/atoms/atoms_template.cpp
webassembly/src/atoms/application/measurement_controller.cpp
webassembly/src/atoms/application/structure_lifecycle_service.cpp
webassembly/src/atoms/application/structure_read_model.cpp
webassembly/src/atoms/application/visibility_service.cpp
webassembly/src/atoms/domain/atom_manager.cpp
webassembly/src/atoms/domain/bond_manager.cpp
webassembly/src/atoms/domain/bz_plot.cpp
webassembly/src/atoms/domain/cell_manager.cpp
webassembly/src/atoms/domain/charge_density.cpp
webassembly/src/atoms/domain/crystal_structure.cpp
webassembly/src/atoms/domain/crystal_system.cpp
webassembly/src/atoms/domain/element_database.cpp
webassembly/src/atoms/domain/structure_state_store.cpp
webassembly/src/atoms/domain/surrounding_atom_manager.cpp
webassembly/src/atoms/infrastructure/batch_update_system.cpp
webassembly/src/atoms/infrastructure/bond_renderer.cpp
webassembly/src/atoms/infrastructure/bz_plot_layer.cpp
webassembly/src/atoms/infrastructure/charge_density_renderer.cpp
webassembly/src/atoms/infrastructure/vtk_renderer.cpp
webassembly/src/atoms/presentation/builder_windows_controller.cpp
webassembly/src/atoms/presentation/editor_windows_controller.cpp
webassembly/src/atoms/presentation/isosurface_panel.cpp
webassembly/src/atoms/presentation/slice_view_panel.cpp
webassembly/src/atoms/ui/atom_editor_ui.cpp
webassembly/src/atoms/ui/bond_ui.cpp
webassembly/src/atoms/ui/bravais_lattice_ui.cpp
webassembly/src/atoms/ui/bz_plot_ui.cpp
webassembly/src/atoms/ui/cell_info_ui.cpp
webassembly/src/atoms/ui/charge_density_ui.cpp
webassembly/src/atoms/ui/periodic_table_ui.cpp
```

### `wb_io`
```text
webassembly/src/file_loader.cpp
webassembly/src/unv_reader.cpp
webassembly/src/io/application/import_apply_service.cpp
webassembly/src/io/application/import_orchestrator.cpp
webassembly/src/io/application/parser_worker_service.cpp
webassembly/src/io/platform/browser_file_picker.cpp
webassembly/src/atoms/infrastructure/chgcar_parser.cpp
webassembly/src/atoms/infrastructure/file_io_manager.cpp
```

### `wb_ui`
```text
webassembly/src/app.cpp
webassembly/src/custom_ui.cpp
webassembly/src/font_manager.cpp
webassembly/src/image.cpp
webassembly/src/test_window.cpp
webassembly/src/texture.cpp
webassembly/src/toolbar.cpp
```

### `root executable`
```text
webassembly/src/main.cpp
webassembly/src/bind_function.cpp
```

## 4. Build-Excluded Legacy / Deferred Units
아래 파일은 Phase 5 기준 빌드 등록 대상에서 제외한다.

```text
webassembly/src/atoms_template_periodic_table.cpp
webassembly/src/atoms_template_bravais_lattice.cpp
webassembly/src/atoms/ui/atoms_template_main_window_ui.cpp
```

## 5. Link Boundary Snapshot
- 루트 executable 역할:
  - `main.cpp`, `bind_function.cpp`만 보유
  - `wb_ui`, `wb_io`, `wb_atoms`, `wb_mesh`, `wb_render`, `wb_core` 링크만 수행
- 모듈 공통:
  - `wb_build_options`, `wb_external_deps`를 통해 compile/link 옵션 공통화
- 명시한 내부 링크 체인:
  - `wb_render -> wb_core, wb_ui, wb_mesh, wb_atoms`
  - `wb_mesh -> wb_core, wb_ui, wb_render, wb_atoms`
  - `wb_atoms -> wb_core, wb_ui, wb_render, wb_mesh`
  - `wb_io -> wb_core, wb_ui, wb_render, wb_mesh, wb_atoms`
  - `wb_ui -> wb_core, wb_render, wb_mesh, wb_atoms, wb_io`

## 6. Remaining Cycle Risks
Phase 5는 빌드 구조 모듈화가 목표이며, 아래 경계는 기존 코드 의존을 보존한 상태다.

1. `wb_ui <-> wb_io`
   - `app.cpp`가 `FileLoader`를 호출하고, `file_loader.cpp`/`parser_worker_service.cpp`가 `App`를 호출함
2. `wb_ui <-> wb_render`
   - `app.cpp`, `toolbar.cpp`, `test_window.cpp`가 `VtkViewer`를 호출하고, `vtk_viewer.cpp`가 `App`, `Toolbar`, `FontManager`를 참조함
3. `wb_mesh <-> wb_render`
   - `mesh*.cpp`가 `VtkViewer`를 직접 호출하고, `vtk_viewer.cpp`가 `MeshManager`를 참조함
4. `wb_atoms <-> wb_render`
   - atoms 계층이 `VtkViewer`/`RenderGateway`를 사용하고, `vtk_viewer.cpp`가 `AtomsTemplate`를 참조함
5. `wb_atoms <-> wb_mesh`
   - `AtomsTemplate`/ModelTree presentation이 `MeshManager`/mesh selection 흐름을 공유함

위 사이클은 Phase 5에서 `STATIC` 모듈 링크로 보존하되, 이후 경계 정리 Phase에서 해소 대상으로 유지한다.
