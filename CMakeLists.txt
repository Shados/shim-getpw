cmake_minimum_required (VERSION 3.3.0)
project (shim-getpw LANGUAGES C)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)


add_library(shim-getpw SHARED src/getpw.c)
target_link_libraries(shim-getpw dl)

add_executable(shim-test-string tests/test-string.c)
add_executable(shim-test-id tests/test-id.c)

install(TARGETS shim-getpw DESTINATION ${CMAKE_INSTALL_LIBDIR}/)

# Test runner script run-time dependencies
find_program (BASH_BIN bash)
find_program (GETENT_BIN getent)
find_program (CUT_BIN cut)

if(BASH_BIN AND GETENT_BIN AND CUT_BIN)
  enable_testing()
  add_test(NAME all-tests COMMAND ${BASH_BIN} ${CMAKE_CURRENT_SOURCE_DIR}/tests/runner.sh $<TARGET_FILE:shim-getpw> $<TARGET_FILE:shim-test-string> $<TARGET_FILE:shim-test-id>)
endif()
