cmake_minimum_required(VERSION 3.28.3)
project(client LANGUAGES C CXX)

add_executable(${PROJECT_NAME} main.cpp Socket.cpp)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE "Release")