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

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")


string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)


add_subdirectory(bpftools)
add_subdirectory(runtime)
add_subdirectory(src)
add_subdirectory(demos)


add_executable(main main.cpp)
target_link_libraries(main srces)

