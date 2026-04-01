cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME "VTK-Workbench")
set(WINDOW_WIDTH 960)
set(WINDOW_HEIGHT 540)

set(CMAKE_CXX_STANDARD 17)

# 64-bit WASM memory model and multi-threading with pthread
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -sMEMORY64=1 -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -sMEMORY64=1 -pthread")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sMEMORY64=1 -pthread")

project(${PROJECT_NAME})

if (NOT EMSCRIPTEN)
  message("Skipping example: This needs to run inside an Emscripten build environment")
  return ()
endif ()

find_package(VTK
  COMPONENTS
    InteractionStyle
    RenderingOpenGL2
    RenderingVolume
    RenderingVolumeOpenGL2
    RenderingAnnotation
    FiltersFlowPaths
    InteractionWidgets
)

if (NOT VTK_FOUND)
  message("Skipping example: ${VTK_NOT_FOUND_MESSAGE}")
  return ()
endif ()

set(VOROPP_DIR "${CMAKE_SOURCE_DIR}/webassembly/dependencies/voro++")
set(VOROPP_INCLUDE_DIR "${VOROPP_DIR}/include/voro++")
set(VOROPP_LIB_DIR "${VOROPP_DIR}/lib")
set(VOROPP_LIBRARY "${VOROPP_LIB_DIR}/libvoro++.a")

set(DEP_INCLUDE_DIR)
set(DEP_LIB_DIR)
set(DEP_LIBS)
include(Dependency.cmake)

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
  "SHELL:-lidbfs.js"
  "SHELL:-lworkerfs.js"
  "SHELL:-sEXPORTED_FUNCTIONS=['_main','_malloc','_free']"
  "SHELL:-sEXPORTED_RUNTIME_METHODS=['FS','WORKERFS']"
  "SHELL:-sWASM=1"
  "SHELL:-sENVIRONMENT=web,worker"
  "SHELL:-sUSE_WEBGL2=1"
  "SHELL:-sMODULARIZE=1"
  "SHELL:-sEXPORT_NAME='createViewer'"
  "SHELL:-sUSE_GLFW=3"
  "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/webassembly/resources/@/resources/"
  "SHELL:-sINITIAL_MEMORY=256MB"
  "SHELL:-sALLOW_MEMORY_GROWTH=1"
  "SHELL:-sMAXIMUM_MEMORY=16GB"
  "SHELL:-sUSE_PTHREADS=1"
  "SHELL:-sPTHREAD_POOL_SIZE=5"
  "SHELL:-sNO_EXIT_RUNTIME=1"
  "SHELL:-Wno-pthreads-mem-growth"
  "SHELL:-sWASM_BIGINT=1"
  "SHELL:-sSTACK_SIZE=1MB"
  "SHELL:-flto"
  "SHELL:-sSHARED_MEMORY=1"
)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "Build type: Release mode")
  list(APPEND EMSCRIPTEN_OPTIMIZATIONS "-O3")
  list(APPEND EMSCRIPTEN_DEBUG_OPTIONS "-g0")
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Build type: Debug mode")
  list(APPEND EMSCRIPTEN_OPTIMIZATIONS "-O0")
  list(APPEND EMSCRIPTEN_DEBUG_OPTIONS "-g3")
  list(APPEND EMSCRIPTEN_LINK_OPTIONS
    "SHELL:-sASSERTIONS=1"
    "SHELL:-sSAFE_HEAP=0"
  )
endif ()

add_library(wb_build_options INTERFACE)
target_include_directories(wb_build_options INTERFACE ${DEP_INCLUDE_DIR} ${VOROPP_INCLUDE_DIR})
target_compile_definitions(wb_build_options INTERFACE
  WINDOW_NAME="${WINDOW_NAME}"
  WINDOW_WIDTH=${WINDOW_WIDTH}
  WINDOW_HEIGHT=${WINDOW_HEIGHT}
  SHOW_IMGUI_DEMO
)
target_compile_options(wb_build_options INTERFACE
  ${EMSCRIPTEN_COMPILE_OPTIONS}
  ${EMSCRIPTEN_OPTIMIZATIONS}
  ${EMSCRIPTEN_DEBUG_OPTIONS}
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(wb_build_options INTERFACE DEBUG_BUILD)
endif ()

add_library(wb_external_deps INTERFACE)
target_link_directories(wb_external_deps INTERFACE ${DEP_LIB_DIR})
target_link_libraries(wb_external_deps INTERFACE
  ${DEP_LIBS}
  ${VTK_LIBRARIES}
  ${VOROPP_LIBRARY}
  ${CMAKE_SOURCE_DIR}/webassembly/dependencies/VTK/Release/lib/libvtkRenderingVolumeOpenGL2.a
)

set(WB_MODULE_DIR "${CMAKE_SOURCE_DIR}/webassembly/cmake/modules")
set(WB_MODULE_TARGETS wb_core wb_render wb_mesh wb_atoms wb_io wb_ui)

foreach (module IN LISTS WB_MODULE_TARGETS)
  add_library(${module} STATIC)
  include("${WB_MODULE_DIR}/${module}.cmake")
  target_link_libraries(${module} PUBLIC wb_build_options wb_external_deps)
  if (DEP_LIST)
    add_dependencies(${module} ${DEP_LIST})
  endif ()
endforeach ()

target_link_libraries(wb_render PUBLIC wb_core wb_ui wb_mesh wb_atoms)
target_link_libraries(wb_mesh PUBLIC wb_core wb_ui wb_render wb_atoms)
target_link_libraries(wb_atoms PUBLIC wb_core wb_ui wb_render wb_mesh)
target_link_libraries(wb_io PUBLIC wb_core wb_ui wb_render wb_mesh wb_atoms)
target_link_libraries(wb_ui PUBLIC wb_core wb_render wb_mesh wb_atoms wb_io)

add_executable(${PROJECT_NAME}
  webassembly/src/main.cpp
  webassembly/src/platform/wasm/workbench_bindings.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE wb_ui wb_io wb_atoms wb_mesh wb_render wb_core)
target_link_options(${PROJECT_NAME} PRIVATE
  ${EMSCRIPTEN_LINK_OPTIONS}
  ${EMSCRIPTEN_OPTIMIZATIONS}
  ${EMSCRIPTEN_DEBUG_OPTIONS}
)

if (EMSCRIPTEN)
  install(
    FILES
      ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.data
      ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.wasm
      ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.js
    DESTINATION "${CMAKE_SOURCE_DIR}/public/wasm"
  )
endif ()

if (DEP_LIST)
  add_dependencies(${PROJECT_NAME} ${DEP_LIST})
endif ()

vtk_module_autoinit(
  TARGETS ${WB_MODULE_TARGETS} ${PROJECT_NAME}
  MODULES ${VTK_LIBRARIES}
)
