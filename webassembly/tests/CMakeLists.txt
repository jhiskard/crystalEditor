cmake_minimum_required(VERSION 3.20)

project(wb_native_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/test_config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/generated/test_config.h
  @ONLY
)

add_executable(wb_native_tests
  test_main.cpp
  io/chgcar_parser_test.cpp
  io/xsf_parser_test.cpp
  atoms/cell_transform_test.cpp
  atoms/measurement_geometry_test.cpp
  ../src/atoms/infrastructure/chgcar_parser.cpp
  ../src/io/infrastructure/xsf_parser.cpp
  ../src/atoms/domain/cell_transform.cpp
  ../src/atoms/domain/measurement_geometry.cpp
)

target_include_directories(wb_native_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/generated
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_compile_definitions(wb_native_tests PRIVATE
  WB_TEST_NO_SPDLOG
)

if (EMSCRIPTEN)
  find_program(NODE_EXECUTABLE node REQUIRED)
  set_property(TARGET wb_native_tests PROPERTY SUFFIX ".cjs")
  target_link_options(wb_native_tests PRIVATE
    "SHELL:-sNODERAWFS=1"
    "SHELL:-sENVIRONMENT=node"
  )
  set_property(TARGET wb_native_tests PROPERTY CROSSCOMPILING_EMULATOR "${NODE_EXECUTABLE}")
endif()

enable_testing()
add_test(NAME wb_native_tests COMMAND wb_native_tests)
