cmake_minimum_required(VERSION 3.10)

project(jfind)

set(CMAKE_BUILD_TYPE Release)

file(GLOB SOURCES "src/*.cpp")

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_STANDARD 20)

add_executable(jfind ${SOURCES})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(jfind PRIVATE Threads::Threads)
