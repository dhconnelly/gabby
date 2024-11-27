cmake_minimum_required(VERSION 3.22)

project(gabby
    DESCRIPTION "an inference server"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES src/*.cc src/*.h)
list(FILTER SOURCES EXCLUDE REGEX ".*(_test|main)\\.cc$")
add_library(${PROJECT_NAME}_lib ${SOURCES})
target_include_directories(${PROJECT_NAME}_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(${PROJECT_NAME} src/main.cc)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)

file(GLOB_RECURSE TEST_SOURCES "src/*_test.cc" "src/test/test.*")
add_executable(${PROJECT_NAME}_test ${TEST_SOURCES} src/test/test_main.cc)
target_link_libraries(${PROJECT_NAME}_test PRIVATE
    ${PROJECT_NAME}_lib)

enable_testing()
add_test(NAME ${PROJECT_NAME}_test
    COMMAND ${PROJECT_NAME}_test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
