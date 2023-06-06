cmake_minimum_required(VERSION 3.10)

project(jfind)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_INSTALL_PREFIX "/usr/local/bin")

file(GLOB SOURCES "src/*.cpp")

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_STANDARD 20)

add_executable(jfind ${SOURCES})

install(TARGETS jfind DESTINATION ${CMAKE_INSTALL_PREFIX})

add_custom_target("uninstall" COMMENT "Uninstall installed files")
add_custom_command(
    TARGET "uninstall"
    POST_BUILD
    COMMENT "Uninstall files with install_manifest.txt"
    COMMAND xargs rm -vf < install_manifest.txt || echo Nothing in
            install_manifest.txt to be uninstalled!
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(jfind PRIVATE Threads::Threads)
