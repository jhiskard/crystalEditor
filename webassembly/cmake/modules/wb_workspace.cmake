target_sources(wb_workspace PRIVATE
  webassembly/src/workspace/domain/workspace_store.cpp
  webassembly/src/workspace/application/workspace_query_service.cpp
  webassembly/src/workspace/application/workspace_command_service.cpp
  webassembly/src/workspace/legacy/atoms_template_facade.cpp
  webassembly/src/shell/presentation/atoms/atom_editor_ui.cpp
  webassembly/src/shell/presentation/atoms/atoms_template_main_window_ui.cpp
  webassembly/src/shell/presentation/atoms/bond_ui.cpp
  webassembly/src/shell/presentation/atoms/bravais_lattice_ui.cpp
  webassembly/src/shell/presentation/atoms/builder_windows_controller.cpp
  webassembly/src/shell/presentation/atoms/bz_plot_ui.cpp
  webassembly/src/shell/presentation/atoms/cell_info_ui.cpp
  webassembly/src/shell/presentation/atoms/editor_windows_controller.cpp
  webassembly/src/shell/presentation/atoms/periodic_table_ui.cpp
)
