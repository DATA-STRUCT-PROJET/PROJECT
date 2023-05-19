cmake_minimum_required(VERSION 3.2)

project(ds-filesystem-template)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(VirtualDisk SHARED
    src/vd.cpp
)

add_library(FileSystem SHARED
    src/fs.cpp
)

add_executable(CLI
    src/main.cpp
    src/prompt.cpp
)

target_link_libraries(FileSystem PRIVATE VirtualDisk)

target_link_libraries(CLI PRIVATE FileSystem)