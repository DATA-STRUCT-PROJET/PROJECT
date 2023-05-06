cmake_minimum_required(VERSION 3.2)
project(ds_filesystem_test)

add_subdirectory(lib/googletest)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(lib/googletest/googletest/include)

set(SOURCE_FILES main.cpp)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${CMAKE_PROJECT_NAME} gtest)
install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION bin)