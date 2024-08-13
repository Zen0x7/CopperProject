cmake_minimum_required(VERSION 3.14...3.22)

project(CopperTests LANGUAGES CXX)

# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage("gh:doctest/doctest@2.4.9")
CPMAddPackage("gh:TheLartians/Format.cmake@1.7.3")
CPMAddPackage(GITHUB_REPOSITORY ikalnytskyi/termcolor VERSION 2.1.0)

if(TEST_INSTALLED_VERSION)
  find_package(Copper REQUIRED)
else()
  CPMAddPackage(NAME Copper SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)

find_package(
  Boost 1.85.0
  COMPONENTS json thread program_options random coroutine
  REQUIRED
)

# ---- Create binary ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(
  ${PROJECT_NAME} doctest::doctest Copper::Copper termcolor OpenSSL::SSL ${Boost_LIBRARIES}
)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)

# enable compiler warnings
if(NOT TEST_INSTALLED_VERSION)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(Copper PUBLIC -Wall -Wpedantic -Wextra -Werror)
  elseif(MSVC)
    target_compile_options(Copper PUBLIC /W4 /WX /bigobj)
    target_compile_definitions(${PROJECT_NAME} PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
  endif()
endif()

# ----- Add CopperTests -----

enable_testing()

# Note: doctest and similar testing frameworks can automatically configure CMake tests. For other
# testing frameworks add the tests target instead: add_test(NAME ${PROJECT_NAME} COMMAND
# ${PROJECT_NAME})

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(${PROJECT_NAME})

# ---- code coverage ----

if(ENABLE_TEST_COVERAGE)
  target_compile_options(Copper PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(Copper PUBLIC -fprofile-arcs -ftest-coverage)
endif()
