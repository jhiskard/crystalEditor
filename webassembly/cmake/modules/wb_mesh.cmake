target_sources(wb_mesh PRIVATE
  webassembly/src/mesh.cpp
  webassembly/src/mesh/domain/mesh_repository.cpp
  webassembly/src/mesh/application/mesh_query_service.cpp
  webassembly/src/mesh/application/mesh_command_service.cpp
  webassembly/src/mesh_detail.cpp
  webassembly/src/mesh_group.cpp
  webassembly/src/mesh_group_detail.cpp
  webassembly/src/mesh_manager.cpp
  webassembly/src/model_tree.cpp
  webassembly/src/mesh/presentation/model_tree_dialogs.cpp
  webassembly/src/mesh/presentation/model_tree_mesh_section.cpp
  webassembly/src/mesh/presentation/model_tree_structure_section.cpp
)
