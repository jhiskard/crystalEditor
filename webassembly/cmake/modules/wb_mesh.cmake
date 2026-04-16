target_sources(wb_mesh PRIVATE
  webassembly/src/mesh/domain/lcrs_tree.cpp
  webassembly/src/mesh/domain/mesh_entity.cpp
  webassembly/src/mesh/domain/mesh_group_entity.cpp
  webassembly/src/mesh/domain/mesh_repository.cpp
  webassembly/src/mesh/domain/mesh_repository_core.cpp
  webassembly/src/mesh/application/mesh_query_service.cpp
  webassembly/src/mesh/application/mesh_command_service.cpp
  webassembly/src/mesh/presentation/mesh_detail_panel.cpp
  webassembly/src/mesh/presentation/mesh_group_detail_panel.cpp
  webassembly/src/mesh/presentation/model_tree_panel.cpp
  webassembly/src/mesh/presentation/model_tree_dialogs.cpp
  webassembly/src/mesh/presentation/model_tree_mesh_section.cpp
  webassembly/src/mesh/presentation/model_tree_structure_section.cpp
)
