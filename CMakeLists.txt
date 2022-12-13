project(TinyBpfLog CXX C)

cmake_minimum_required(VERSION 2.8)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CXX_FLAGS
        -g
        -Wall
        -std=c++17
        -D_PTHREADS
        -Wno-unused-parameter
        -O3
        )

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

option(RUNNING_DAEMON "running as a daemon process")
if (RUNNING_DAEMON)
    add_compile_options(-DRUNNING_DAEMON)
endif()

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")


string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)


add_subdirectory(bpftools)
add_subdirectory(runtime)
add_subdirectory(backend)
add_subdirectory(test)


add_executable(main main.cpp)
target_link_libraries(main backend)

