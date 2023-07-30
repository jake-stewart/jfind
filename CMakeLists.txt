cmake_minimum_required(VERSION 3.10)

project(jfind)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
option(ENABLE_UBSAN "Enable undefined behavior sanitizer" OFF)
option(ENABLE_ASAN "Enable address sanitizer" OFF)
option(ENABLE_TSAN "Enable thread sanitizer" OFF)
if(ENABLE_ASAN AND ENABLE_TSAN)
    message(FATAL_ERROR "ASAN and TSAN are not compatible")
endif()
if(ENABLE_UBSAN OR ENABLE_ASAN OR ENABLE_TSAN)
    add_compile_options(-fno-omit-frame-pointer -fno-optimize-sibling-calls)
endif()
if(ENABLE_UBSAN)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=undefined)
endif()
if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()
if(ENABLE_TSAN)
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
endif()

file(GLOB SOURCES "src/*.cpp")

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_STANDARD 20)

add_executable(jfind ${SOURCES})

install(TARGETS jfind DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
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
