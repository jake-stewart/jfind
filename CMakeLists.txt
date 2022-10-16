cmake_minimum_required(VERSION 3.10)

project(jfind)

file(GLOB SOURCES "src/*.cpp")

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
SET(CMAKE_CXX_STANDARD 20)

add_executable(jfind ${SOURCES})

