set(TARGET_NAME data_generation)
set(SOURCES main.cpp)

source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(${TARGET_NAME} ${SOURCES})
target_compile_definitions(${TARGET_NAME} PRIVATE DATA_DIRECTORY="${DATA_DIRECTORY}")
