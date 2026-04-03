target_sources(wb_ui PRIVATE
  webassembly/src/app.cpp
  webassembly/src/custom_ui.cpp
  webassembly/src/font_manager.cpp
  webassembly/src/image.cpp
  webassembly/src/shell/domain/shell_state_store.cpp
  webassembly/src/shell/application/shell_state_query_service.cpp
  webassembly/src/shell/application/shell_state_command_service.cpp
  webassembly/src/shell/application/workbench_controller.cpp
  webassembly/src/shell/runtime/workbench_runtime.cpp
  webassembly/src/test_window.cpp
  webassembly/src/texture.cpp
  webassembly/src/toolbar.cpp
)
