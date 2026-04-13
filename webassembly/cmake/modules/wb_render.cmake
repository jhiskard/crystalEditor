target_sources(wb_render PRIVATE
  webassembly/src/render/presentation/viewer_window.cpp
  webassembly/src/render/infrastructure/interaction/mouse_interactor_style.cpp
  webassembly/src/render/application/camera_align_service.cpp
  webassembly/src/render/application/legacy_viewer_facade.cpp
  webassembly/src/render/application/viewer_interaction_controller.cpp
  webassembly/src/render/infrastructure/vtk_render_gateway.cpp
  webassembly/src/platform/persistence/viewer_preferences_store.cpp
)
