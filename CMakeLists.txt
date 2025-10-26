cmake_minimum_required(VERSION 3.21)
project(Project2 VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -O0 -g -w")

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(Project2 SHARED
        receiver.cpp
        fake_receiver.c
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_executable(project_2 main.cpp)

target_link_libraries(project_2 PRIVATE Project2)
