cmake_minimum_required(VERSION 3.2)
project(ds-project)

set(SOURCE_FILES main.cpp fs.cpp vd.cpp prompt.cpp)

add_executable(fs ${SOURCE_FILES})
set_target_properties(fs PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

install(TARGETS fs DESTINATION ${CMAKE_INSTALL_PREFIX})