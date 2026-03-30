cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME "VTK-Workbench")
set(WINDOW_WIDTH 960)
set(WINDOW_HEIGHT 540)

# C++ 17
set(CMAKE_CXX_STANDARD 17)

# 64-bit WASM memory model and multi-threading with pthread
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -sMEMORY64=1 -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -sMEMORY64=1 -pthread")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sMEMORY64=1 -pthread")
# set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -sMEMORY64=1 -pthread")

# Set project
project(${PROJECT_NAME})

# EMSCRIPTEN only
if (NOT EMSCRIPTEN)
  message("Skipping example: This needs to run inside an Emscripten build environment")
  return ()
endif ()

# Set VTK dependency
find_package(VTK
  COMPONENTS
    # FiltersSources      # VTK pipeline
    InteractionStyle    # Mouse handling
    RenderingOpenGL2    # For Rendering
    RenderingVolume
    RenderingVolumeOpenGL2
    # RenderingUI         # For SDL2 Window
    RenderingAnnotation
    FiltersFlowPaths
    InteractionWidgets  # For OrientationMarkerWidget
)

if (NOT VTK_FOUND)
  message("Skipping example: ${VTK_NOT_FOUND_MESSAGE}")
  return ()
endif ()

# ============================================================================
# Voro++ Configuration
# ============================================================================
set(VOROPP_DIR "${CMAKE_SOURCE_DIR}/webassembly/dependencies/voro++")
set(VOROPP_INCLUDE_DIR "${VOROPP_DIR}/include/voro++")
set(VOROPP_LIB_DIR "${VOROPP_DIR}/lib")
set(VOROPP_LIBRARY "${VOROPP_LIB_DIR}/libvoro++.a")

# ============================================================================
# spglib Configuration
# ============================================================================
# set(SPGLIB_DIR "${CMAKE_SOURCE_DIR}/webassembly/dependencies/spglib")
# set(SPGLIB_INCLUDE_DIR "${SPGLIB_DIR}/include")
# set(SPGLIB_LIB_DIR "${SPGLIB_DIR}/lib")
# set(SPGLIB_LIBRARY "${SPGLIB_LIB_DIR}/libsymspg.a")

# Set source files
add_executable(${PROJECT_NAME}
  webassembly/src/main.cpp
  webassembly/src/app.cpp                     webassembly/src/app.h
  webassembly/src/vtk_viewer.cpp              webassembly/src/vtk_viewer.h
  webassembly/src/mouse_interactor_style.cpp  webassembly/src/mouse_interactor_style.h
  webassembly/src/test_window.cpp             webassembly/src/test_window.h
  webassembly/src/font_manager.cpp            webassembly/src/font_manager.h
  webassembly/src/file_loader.cpp             webassembly/src/file_loader.h
  webassembly/src/io/platform/browser_file_picker.cpp
  webassembly/src/io/platform/browser_file_picker.h
  webassembly/src/io/application/parser_worker_service.cpp
  webassembly/src/io/application/parser_worker_service.h
  webassembly/src/io/application/import_apply_service.cpp
  webassembly/src/io/application/import_apply_service.h
  webassembly/src/io/application/import_orchestrator.cpp
  webassembly/src/io/application/import_orchestrator.h
  webassembly/src/bind_function.cpp
  webassembly/src/lcrs_tree.cpp               webassembly/src/lcrs_tree.h
  webassembly/src/mesh.cpp                    webassembly/src/mesh.h
  webassembly/src/mesh_manager.cpp            webassembly/src/mesh_manager.h
  webassembly/src/model_tree.cpp              webassembly/src/model_tree.h
  webassembly/src/custom_ui.cpp               webassembly/src/custom_ui.h
  webassembly/src/image.cpp                   webassembly/src/image.h
  webassembly/src/texture.cpp                 webassembly/src/texture.h
  webassembly/src/toolbar.cpp                 webassembly/src/toolbar.h
  webassembly/src/mesh_detail.cpp             webassembly/src/mesh_detail.h
  webassembly/src/unv_reader.cpp              webassembly/src/unv_reader.h
  webassembly/src/common/string_utils.cpp     webassembly/src/common/string_utils.h
  webassembly/src/common/colormap.cpp         webassembly/src/common/colormap.h
  webassembly/src/mesh_group.cpp              webassembly/src/mesh_group.h
  webassembly/src/mesh_group_detail.cpp       webassembly/src/mesh_group_detail.h

  # Render
  webassembly/src/render/application/render_gateway.h
  webassembly/src/render/infrastructure/vtk_render_gateway.cpp
  webassembly/src/render/infrastructure/vtk_render_gateway.h

  # Atoms
  webassembly/src/atoms/atoms_template.cpp 
  webassembly/src/atoms/atoms_template.h 
  webassembly/src/atoms/application/structure_read_model.cpp
  webassembly/src/atoms/application/structure_read_model.h
  webassembly/src/atoms/application/measurement_controller.cpp
  webassembly/src/atoms/application/structure_lifecycle_service.cpp
  webassembly/src/atoms/application/visibility_service.cpp
  webassembly/src/atoms/infrastructure/batch_update_system.cpp 
  webassembly/src/atoms/infrastructure/batch_update_system.h 
  webassembly/src/atoms/infrastructure/bond_renderer.cpp
  webassembly/src/atoms/infrastructure/bond_renderer.h
  webassembly/src/atoms/infrastructure/vtk_renderer.cpp 
  webassembly/src/atoms/infrastructure/vtk_renderer.h 
  webassembly/src/atoms/infrastructure/file_io_manager.cpp 
  webassembly/src/atoms/infrastructure/file_io_manager.h 
  webassembly/src/atoms/infrastructure/bz_plot_layer.cpp 
  webassembly/src/atoms/infrastructure/bz_plot_layer.h 
  webassembly/src/atoms/domain/atom_manager.cpp 
  webassembly/src/atoms/domain/atom_manager.h 
  webassembly/src/atoms/domain/bond_manager.cpp 
  webassembly/src/atoms/domain/bond_manager.h 
  webassembly/src/atoms/domain/cell_manager.cpp 
  webassembly/src/atoms/domain/cell_manager.h 
  webassembly/src/atoms/domain/structure_state_store.cpp
  webassembly/src/atoms/domain/structure_state_store.h
  webassembly/src/atoms/domain/element_database.cpp 
  webassembly/src/atoms/domain/element_database.h 
  webassembly/src/atoms/domain/surrounding_atom_manager.cpp
  webassembly/src/atoms/domain/surrounding_atom_manager.h
  webassembly/src/atoms/domain/bz_plot.cpp 
  webassembly/src/atoms/domain/bz_plot.h 
  webassembly/src/atoms/domain/special_points.h 
  webassembly/src/atoms/domain/crystal_system.cpp
  webassembly/src/atoms/domain/crystal_system.h
  webassembly/src/atoms/domain/crystal_structure.cpp
  webassembly/src/atoms/domain/crystal_structure.h
  webassembly/src/atoms/domain/color.h 
  webassembly/src/atoms/ui/periodic_table_ui.cpp 
  webassembly/src/atoms/ui/periodic_table_ui.h 
  webassembly/src/atoms/ui/bravais_lattice_ui.cpp 
  webassembly/src/atoms/ui/bravais_lattice_ui.h 
  webassembly/src/atoms/ui/atom_editor_ui.cpp 
  webassembly/src/atoms/ui/atom_editor_ui.h 
  webassembly/src/atoms/ui/bond_ui.cpp
  webassembly/src/atoms/ui/bond_ui.h
  webassembly/src/atoms/ui/cell_info_ui.cpp 
  webassembly/src/atoms/ui/cell_info_ui.h 
  webassembly/src/atoms/ui/bz_plot_ui.cpp 
  webassembly/src/atoms/ui/bz_plot_ui.h 
  webassembly/src/atoms/presentation/builder_windows_controller.cpp
  webassembly/src/atoms/presentation/editor_windows_controller.cpp
  webassembly/src/atoms/presentation/isosurface_panel.cpp
  webassembly/src/atoms/presentation/slice_view_panel.cpp
  # webassembly/src/atoms/ui/atoms_template_main_window_ui.cpp 
  # webassembly/src/atoms/ui/atoms_template_main_window_ui.h 
  webassembly/src/atoms/ui/ui_color_utils.h
    # Charge Density 관련 파일 추가
  webassembly/src/atoms/ui/charge_density_ui.cpp
  webassembly/src/atoms/infrastructure/chgcar_parser.cpp
  webassembly/src/atoms/infrastructure/charge_density_renderer.cpp
  webassembly/src/atoms/domain/charge_density.cpp
)

# Add dependencies
set(DEP_INCLUDE_DIR)
set(DEP_LIB_DIR)
set(DEP_LIBS)

include(Dependency.cmake)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${DEP_INCLUDE_DIR}
    ${VOROPP_INCLUDE_DIR}
    # ${SPGLIB_INCLUDE_DIR}
)
target_link_directories(${PROJECT_NAME}
  PRIVATE
    ${DEP_LIB_DIR}
    # ${SPGLIB_LIBRARY}
)
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    ${DEP_LIBS}
    ${VTK_LIBRARIES}
    ${VOROPP_LIBRARY}
    ${CMAKE_SOURCE_DIR}/webassembly/dependencies/VTK/Release/lib/libvtkRenderingVolumeOpenGL2.a
)

# Declare emscripten option variables
set(EMSCRIPTEN_COMPILE_OPTIONS)
set(EMSCRIPTEN_LINK_OPTIONS)
set(EMSCRIPTEN_OPTIMIZATIONS)
set(EMSCRIPTEN_DEBUG_OPTIONS)

list(APPEND EMSCRIPTEN_COMPILE_OPTIONS
  "SHELL:-flto"
  "SHELL:-sSHARED_MEMORY=1"
)

list(APPEND EMSCRIPTEN_LINK_OPTIONS
  "SHELL:--bind"
  "SHELL:-lidbfs.js"                # IndexedDB file system support
  "SHELL:-lworkerfs.js"             # Worker file system support
  "SHELL:-sEXPORTED_FUNCTIONS=['_main','_malloc','_free']"
  "SHELL:-sEXPORTED_RUNTIME_METHODS=['FS','WORKERFS']"
  "SHELL:-sWASM=1"
  "SHELL:-sENVIRONMENT=web,worker"
  "SHELL:-sUSE_WEBGL2=1"
  "SHELL:-sMODULARIZE=1"
  "SHELL:-sEXPORT_NAME='createViewer'"
  "SHELL:-sUSE_GLFW=3"
  "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/webassembly/resources/@/resources/"
  # "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/webassembly/src/imgui.ini@/defaults/imgui.ini"
  "SHELL:-sINITIAL_MEMORY=256MB"
  "SHELL:-sALLOW_MEMORY_GROWTH=1"
  "SHELL:-sMAXIMUM_MEMORY=16GB"
  "SHELL:-sUSE_PTHREADS=1" 
  "SHELL:-sPTHREAD_POOL_SIZE=5"
  "SHELL:-sNO_EXIT_RUNTIME=1"
  "SHELL:-Wno-pthreads-mem-growth"  # Suppress warning (-Wpthreads-mem-growth)
  "SHELL:-sWASM_BIGINT=1"           # Use BigInt for 64-bit integers
  "SHELL:-sSTACK_SIZE=1MB"          # Default stack size is 64KB.
  "SHELL:-flto"                     # Link time optimization
  "SHELL:-sSHARED_MEMORY=1"         # Use shared memory for multi-threading
)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    WINDOW_NAME="${WINDOW_NAME}"
    WINDOW_WIDTH=${WINDOW_WIDTH}
    WINDOW_HEIGHT=${WINDOW_HEIGHT}
    SHOW_IMGUI_DEMO  # Show ImGui demo window. Comment this for production build
    # SHOW_FONT_ICONS  # Show font icons. Comment this for production build
)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "Build type: Release mode")
  list(APPEND EMSCRIPTEN_OPTIMIZATIONS "-O3")
  list(APPEND EMSCRIPTEN_DEBUG_OPTIONS "-g0")
  # list(APPEND EMSCRIPTEN_LINK_OPTIONS
  #   "SHELL:--memoryprofiler"  # If this option turns on, resizing does not work properly.
  # )
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Build type: Debug mode")
  list(APPEND EMSCRIPTEN_OPTIMIZATIONS "-O0")
  list(APPEND EMSCRIPTEN_DEBUG_OPTIONS "-g3")
  list(APPEND EMSCRIPTEN_LINK_OPTIONS
    "SHELL:-sASSERTIONS=1"
    # "SHELL:--memoryprofiler"  # If this option turns on, resizing does not work properly.
    "SHELL:-sSAFE_HEAP=0"
  )
  target_compile_definitions(${PROJECT_NAME}
    PRIVATE
      DEBUG_BUILD
  )
endif()

target_compile_options(${PROJECT_NAME}
  PRIVATE
    ${EMSCRIPTEN_COMPILE_OPTIONS}
    ${EMSCRIPTEN_OPTIMIZATIONS}
    ${EMSCRIPTEN_DEBUG_OPTIONS}
)

target_link_options(${PROJECT_NAME}
  PRIVATE
    ${EMSCRIPTEN_LINK_OPTIONS}
    ${EMSCRIPTEN_OPTIMIZATIONS}
    ${EMSCRIPTEN_DEBUG_OPTIONS}
)

# Copy wasm files to public folder
if (EMSCRIPTEN)
  install(
    FILES
      ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.data
      ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.wasm
      ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.js
    DESTINATION "${CMAKE_SOURCE_DIR}/public/wasm"
  ) 
endif()

# if DEP_LIST is not empty, add dependencies
if (DEP_LIST)
  add_dependencies(${PROJECT_NAME} ${DEP_LIST})
endif()

# VTK modules initialization
vtk_module_autoinit(
  TARGETS ${PROJECT_NAME}
  MODULES ${VTK_LIBRARIES}
)
