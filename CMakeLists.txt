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

if(UNIX)
    target_compile_options(VirtualDisk PRIVATE -fPIC)
elseif(WIN32)
    target_compile_definitions(VirtualDisk PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

add_library(FileSystem
    include/fs.hpp
    src/fs.cpp
)

if(UNIX)
    target_compile_options(FileSystem PRIVATE -fPIC)
elseif(WIN32)
    target_compile_definitions(FileSystem PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

target_link_libraries(FileSystem LINK_PUBLIC VirtualDisk)

add_executable(CLI
    src/main.cpp
    src/prompt.cpp
)

if(WIN32)
    target_compile_definitions(CLI PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

target_link_libraries(CLI PRIVATE FileSystem)