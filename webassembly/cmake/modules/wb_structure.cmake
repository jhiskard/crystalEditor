target_sources(wb_structure PRIVATE
  webassembly/src/structure/application/structure_lifecycle_service.cpp
  webassembly/src/structure/application/structure_read_model.cpp
  webassembly/src/structure/application/structure_service.cpp
  webassembly/src/structure/application/visibility_service.cpp
  webassembly/src/structure/domain/atoms/atom_manager.cpp
  webassembly/src/structure/domain/atoms/bond_manager.cpp
  webassembly/src/structure/domain/atoms/bz_plot.cpp
  webassembly/src/structure/domain/atoms/cell_manager.cpp
  webassembly/src/structure/domain/atoms/cell_transform.cpp
  webassembly/src/structure/domain/atoms/crystal_structure.cpp
  webassembly/src/structure/domain/atoms/crystal_system.cpp
  webassembly/src/structure/domain/atoms/element_database.cpp
  webassembly/src/structure/domain/atoms/structure_state_store.cpp
  webassembly/src/structure/domain/atoms/surrounding_atom_manager.cpp
  webassembly/src/structure/domain/structure_repository.cpp
)
