cmake_minimum_required(VERSION 3.14...3.22)

project(CopperStandalone LANGUAGES CXX)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage(
  GITHUB_REPOSITORY jarro2783/cxxopts
  VERSION 3.0.0
  OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO" "CXXOPTS_ENABLE_INSTALL YES"
)

CPMAddPackage(GITHUB_REPOSITORY ikalnytskyi/termcolor VERSION 2.1.0)

CPMAddPackage(NAME Copper SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

# ---- Create standalone executable ----

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(
  Boost 1.85.0
  COMPONENTS json thread program_options random coroutine
  REQUIRED
)

find_package(OpenSSL REQUIRED)

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

add_executable(${PROJECT_NAME} ../redis.cpp ${sources})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20 OUTPUT_NAME "Copper")

target_link_libraries(
  ${PROJECT_NAME} Copper::Copper OpenSSL::SSL cxxopts termcolor ${Boost_LIBRARIES}
)
