# Global general settings
cmake_minimum_required(VERSION 3.10.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(DATA_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data/)

# Define top level project and add targets to it
project(AWMA)
add_subdirectory(openmp)
add_subdirectory(data_generation)
set_directory_properties(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTIES VS_STARTUP_PROJECT kmeans_openmp)
