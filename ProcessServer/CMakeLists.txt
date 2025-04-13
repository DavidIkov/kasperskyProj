cmake_minimum_required(VERSION 3.28.3)
project(proc_server LANGUAGES C CXX)

add_executable(${PROJECT_NAME} main.cpp ProcessServer.cpp)
set(CMAKE_BUILD_TYPE "Release")

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/../Shared)
add_subdirectory(${PROJECT_SOURCE_DIR}/../Shared Shared)
target_link_libraries(${PROJECT_NAME} Shared)