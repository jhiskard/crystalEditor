target_sources(wb_render PRIVATE
  webassembly/src/render/application/camera_align_service.cpp
  webassembly/src/render/application/viewer_interaction_controller.cpp
  webassembly/src/render/presentation/viewer_window.cpp
  webassembly/src/render/infrastructure/atoms/batch_update_system.cpp
  webassembly/src/render/infrastructure/atoms/bond_renderer.cpp
  webassembly/src/render/infrastructure/atoms/bz_plot_layer.cpp
  webassembly/src/render/infrastructure/atoms/vtk_renderer.cpp
  webassembly/src/render/infrastructure/interaction/mouse_interactor_style.cpp
  webassembly/src/render/infrastructure/resources/image_loader.cpp
  webassembly/src/render/infrastructure/resources/texture_resource.cpp
  webassembly/src/render/infrastructure/vtk_render_gateway.cpp
)
