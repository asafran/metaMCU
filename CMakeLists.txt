cmake_minimum_required(VERSION 3.14)
project(metaMCU 
VERSION 0.0.1
DESCRIPTION "C++23 type-safe header-only MCU support library"
LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

option(METAMCU_GENERATE_DOCS "Generate docs" ON)
option(METAMCU_BUILD_EXAMPLES "Build examples" OFF)

if(DEFINED METAMCU_BUILD_EXAMPLES)
    add_subdirectory(examples/metaMCU_templateF407Project)
endif()
    
set(METAMCU_INCLUDE_DIRECTORIES core utils)

add_library(metaMCU INTERFACE)
add_library(metaMCU::metaMCU ALIAS metaMCU)

target_include_directories(metaMCU INTERFACE ${METAMCU_INCLUDE_DIRECTORIES})

if(METAMCU_GENERATE_DOCS)
    find_package(Doxygen REQUIRED dot)

    set(DOXYGEN_HTML_OUTPUT            ${PROJECT_SOURCE_DIR}/docs/html)
    set(DOXYGEN_GENERATE_HTML          YES)
    set(DOXYGEN_HAVE_DOT               YES)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)

    doxygen_add_docs(docs_target
      ${METAMCU_INCLUDE_DIRECTORIES} README.md
      ALL
      COMMENT "Generate HTML documentation"
    )
    target_link_libraries(metaMCU INTERFACE docs_target)
endif()
