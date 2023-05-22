cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)

project(ds-filesystem-test)

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/bin)

add_subdirectory(lib/googletest)

include_directories(include)
include_directories(../include)
include_directories(lib/googletest/googletest/include)

set(EXEC_FILE
    ../src/prompt.cpp
    ../src/vd.cpp
    ../src/fs.cpp
)

add_executable(Test
    ${EXEC_FILE}
    src/Prompt/cat.cpp
    src/Prompt/cd.cpp
    src/Prompt/echo.cpp
    src/Prompt/ls.cpp
    src/main.cpp
)

target_compile_definitions(Test PRIVATE TEST_ENABLED)

target_link_libraries(Test PRIVATE gtest)