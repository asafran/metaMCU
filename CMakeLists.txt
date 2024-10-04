cmake_minimum_required(VERSION 3.14)
project(metaMCU 
VERSION 0.0.1
DESCRIPTION "C++20 type-safe header-only MCU support library"
LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

option(METAMCU_BUILD_EXAMPLES "Build examples" OFF)

if(DEFINED METAMCU_BUILD_EXAMPLES)
    add_subdirectory(examples/metaMCU_templateF407Project)
endif()
    
set(METAMCU_INCLUDE_DIRECTORIES core utils)
add_library(metaMCU INTERFACE)
target_include_directories(metaMCU INTERFACE ${METAMCU_INCLUDE_DIRECTORIES})
