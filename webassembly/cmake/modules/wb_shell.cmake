target_sources(wb_shell PRIVATE
  webassembly/src/app.cpp
  webassembly/src/shell/domain/shell_state_store.cpp
  webassembly/src/shell/application/shell_state_query_service.cpp
  webassembly/src/shell/application/shell_state_command_service.cpp
  webassembly/src/shell/application/workbench_controller.cpp
  webassembly/src/shell/runtime/workbench_runtime.cpp
  webassembly/src/shell/presentation/main_menu.cpp
  webassembly/src/shell/presentation/window_layout.cpp
  webassembly/src/shell/presentation/window_registry.cpp
  webassembly/src/shell/presentation/popup_presenter.cpp
  webassembly/src/shell/presentation/font/font_scale_controller.cpp
  webassembly/src/shell/presentation/theme/color_style_controller.cpp
  webassembly/src/shell/presentation/debug/test_window_panel.cpp
  webassembly/src/shell/presentation/font/font_registry.cpp
  webassembly/src/shell/presentation/import/import_popup_presenter.cpp
  webassembly/src/shell/presentation/toolbar/viewer_toolbar_presenter.cpp
  webassembly/src/shell/presentation/widgets/icon_button.cpp
)

target_include_directories(wb_shell PRIVATE
  webassembly/src
)
