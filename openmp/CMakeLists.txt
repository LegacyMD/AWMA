find_package(OpenMP REQUIRED)

set(TARGET_NAME kmeans_openmp)
set(SOURCES 
    definitions.h
    implementation/implementation.cpp
    implementation/implementation.h
    implementation/omp/updateOmpAtomics.h
    implementation/omp/updateOmpReduction.h
    main.cpp
    utils/parameters.cpp
    utils/parameters.h
    utils/random.cpp
    utils/random.h
    utils/timer.h
)

source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(${TARGET_NAME} ${SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${TARGET_NAME} PRIVATE OpenMP::OpenMP_CXX)
target_compile_definitions(${TARGET_NAME} PRIVATE 
    CSV_PATH="${CMAKE_SOURCE_DIR}/matlab_visualization/"
    DATA_DIRECTORY="${DATA_DIRECTORY}"
)
