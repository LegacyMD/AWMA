# Global general settings
cmake_minimum_required(VERSION 3.10.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(DATA_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Define top level project and add targets to it
project(AWMA)
add_subdirectory(openmp)
add_subdirectory(data_generation)
set_directory_properties(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTIES VS_STARTUP_PROJECT kmeans_openmp)
