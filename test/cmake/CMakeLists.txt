project(MyProject)

cmake_minimum_required(VERSION 3.5)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_executable(a.out ../a.cpp)
