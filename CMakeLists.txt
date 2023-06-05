cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)

project(ds-filesystem)

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(VirtualDisk
    include/vd.hpp
    src/vd.cpp
)

target_compile_options(VirtualDisk PRIVATE -fPIC)

add_library(FileSystem SHARED
    include/fs.hpp
    src/fs.cpp
)
target_compile_options(FileSystem PRIVATE -fPIC)
target_link_libraries(FileSystem LINK_PUBLIC VirtualDisk)

add_executable(CLI
    src/main.cpp
    src/prompt.cpp
)

target_link_libraries(CLI PRIVATE FileSystem)