cmake_minimum_required(VERSION 3.28.3)
project(client LANGUAGES C CXX)

add_executable(${PROJECT_NAME} main.cpp Client.cpp)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Release")

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/../Shared)
add_subdirectory(${PROJECT_SOURCE_DIR}/../Shared Shared)
target_link_libraries(${PROJECT_NAME} Shared)