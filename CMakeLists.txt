# Disable in-source builds to prevent source tree corruption.
if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "FATAL: In-source builds are not allowed.You should create a separate directory for build files.")
endif()

cmake_minimum_required(VERSION 3.10)

project(vkc VERSION 1.0.0 DESCRIPTION "Simple GPU compute framework using vulkan compute shaders")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_library(vkc
  SHARED
  src/vk.cc
  src/api/vkbuffer.cc
  src/api/vkcommandbuffer.cc
  src/api/vkcommandpool.cc
  src/api/vkdescriptorpool.cc
  src/api/vkdescriptorset.cc
  src/api/vkdevice.cc
  src/api/vkshader.cc
  src/api/vkutils.cc
)

target_compile_options(vkc PRIVATE -Wall -Wextra  -Wunreachable-code -Wpedantic)
target_link_libraries(vkc vulkan pthread)

set_target_properties(vkc PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(vkc PROPERTIES SOVERSION 1)

add_custom_target(tests_shaders COMMAND ${CMAKE_SOURCE_DIR}/build_shaders.sh ${CMAKE_SOURCE_DIR}/tests/unittests/fixtures/shaders)

add_executable(vk_tests
  tests/unittests/arraybuffer.test.cc
  tests/unittests/device.test.cc
  tests/unittests/program.test.cc
  tests/main.cc
)

find_package(Catch2 REQUIRED)
target_link_libraries(vk_tests Catch2::Catch2)

target_include_directories(vk_tests PRIVATE tests/googletest/include tests/googletest)
target_link_libraries(vk_tests vkc)

add_dependencies(vk_tests tests_shaders)

enable_testing()
# add_test(NAME build_shaders COMMAND ./build_shaders)
add_test(NAME default_tests COMMAND vk_tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})