# Global general settings
cmake_minimum_required(VERSION 3.12.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Define top level project and add targets to it
project(AWMA)
add_subdirectory(openmp)
set_directory_properties(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTIES VS_STARTUP_PROJECT kmeans_openmp)
