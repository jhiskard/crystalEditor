include(ExternalProject)

set(DEP_INSTALL_DIR ${PROJECT_BINARY_DIR}/install)
set(DEP_INCLUDE_DIR ${DEP_INSTALL_DIR}/include)
set(DEP_LIB_DIR ${DEP_INSTALL_DIR}/lib)

# # 64-bit WASM memory model and multi-threading with pthread
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -sMEMORY64=1 -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -sMEMORY64=1 -pthread")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sMEMORY64=1 -pthread")

# spdlog: fast logger library
ExternalProject_Add(
  dep-spdlog
  GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
  GIT_TAG "v1.x"
  GIT_SHALLOW 1
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  CMAKE_ARGS
    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
    -DCMAKE_INSTALL_PREFIX=${DEP_INSTALL_DIR}
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DSPDLOG_BUILD_EXAMPLE=OFF
    -DSPDLOG_BUILD_TESTS=OFF
    -DSPDLOG_BUILD_BENCH=OFF
    -DSPDLOG_USE_STD_FORMAT=OFF
  TEST_COMMAND ""
)
set(DEP_LIST ${DEP_LIST} dep-spdlog)
set(DEP_LIBS ${DEP_LIBS} spdlog$<$<CONFIG:Debug>:d>)

# OpenCASCADE-7.9.0
set(OCCT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/webassembly/dependencies/OCCT/Release/include)
set(OCCT_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/webassembly/dependencies/OCCT/Release/lib)
set(OCCT_LIBS 
    TKBin TKBinL TKBinTObj TKBinXCAF TKBO TKBool TKBRep TKCAF TKCDF TKDE
    TKDECascade TKDEGLTF TKDEIGES TKDEOBJ TKDEPLY TKDESTEP TKDESTL TKDEVRML TKernel TKFeat
    TKFillet TKG2d TKG3d TKGeomAlgo TKGeomBase TKHLR TKIVtk TKLCAF TKMath TKMesh
    TKMeshVS TKOffset TKOpenGles TKPrim TKRWMesh TKService TKShHealing TKStd TKStdL TKTObj
    TKTopAlgo TKV3d TKVCAF TKXCAF TKXMesh TKXml TKXmlL TKXmlTObj TKXmlXCAF TKXSBase
)
set(DEP_INCLUDE_DIR ${DEP_INCLUDE_DIR} ${OCCT_INCLUDE_DIR})
set(DEP_LIB_DIR ${DEP_LIB_DIR} ${OCCT_LIBRARY_DIR})
set(DEP_LIBS ${DEP_LIBS} ${OCCT_LIBS})

# imgui
set(IMGUI_SOURCE_DIR ${CMAKE_SOURCE_DIR}/webassembly/dependencies/imgui-1.91.8-docking/)
add_library(imgui
  ${IMGUI_SOURCE_DIR}/imgui_demo.cpp
  ${IMGUI_SOURCE_DIR}/imgui_draw.cpp
  ${IMGUI_SOURCE_DIR}/imgui_impl_glfw.cpp
  ${IMGUI_SOURCE_DIR}/imgui_impl_opengl3.cpp
  ${IMGUI_SOURCE_DIR}/imgui_tables.cpp
  ${IMGUI_SOURCE_DIR}/imgui_widgets.cpp
  ${IMGUI_SOURCE_DIR}/imgui.cpp
)
target_include_directories(imgui PRIVATE ${DEP_INCLUDE_DIR})
add_dependencies(imgui ${DEP_LIST})
set(DEP_INCLUDE_DIR ${DEP_INCLUDE_DIR} ${IMGUI_SOURCE_DIR})
set(DEP_LIST ${DEP_LIST} imgui)
set(DEP_LIBS ${DEP_LIBS} imgui)

# stb image
ExternalProject_Add(
  dep_stb
  GIT_REPOSITORY "https://github.com/nothings/stb"
  GIT_TAG "master"
  GIT_SHALLOW 1
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  TEST_COMMAND ""
  INSTALL_COMMAND ${CMAKE_COMMAND} -E copy
    ${PROJECT_BINARY_DIR}/dep_stb-prefix/src/dep_stb/stb_image.h
    ${DEP_INSTALL_DIR}/include/stb/stb_image.h
)
set(DEP_LIST ${DEP_LIST} dep_stb)