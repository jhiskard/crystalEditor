target_sources(wb_ui PRIVATE
  webassembly/src/app.cpp
  webassembly/src/shell/presentation/widgets/icon_button.cpp
  webassembly/src/shell/presentation/font/font_registry.cpp
  webassembly/src/render/infrastructure/resources/image_loader.cpp
  webassembly/src/shell/domain/shell_state_store.cpp
  webassembly/src/shell/application/shell_state_query_service.cpp
  webassembly/src/shell/application/shell_state_command_service.cpp
  webassembly/src/shell/application/workbench_controller.cpp
  webassembly/src/shell/runtime/workbench_runtime.cpp
  webassembly/src/shell/presentation/debug/test_window_panel.cpp
  webassembly/src/render/infrastructure/resources/texture_resource.cpp
  webassembly/src/shell/presentation/toolbar/viewer_toolbar_presenter.cpp
)
