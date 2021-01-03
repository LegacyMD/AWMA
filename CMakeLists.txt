# Global general settings
cmake_minimum_required(VERSION 3.10.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(DATA_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# GCC optimizations
if (UNIX)
    message(STATUS "Hello gnu")
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release)
    endif()

    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

# Define top level project and add targets to it
project(AWMA)
add_subdirectory(source)
add_subdirectory(data_generation)
set_directory_properties(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTIES VS_STARTUP_PROJECT kmeans_openmp)
