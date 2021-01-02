# Common logic for defining executable
function (add_kmeans_executable TARGET_NAME SOURCES_SPECIFIC DEFINE)
        set(SOURCES_COMMON
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

    source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES_COMMON})
    source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES_SPECIFIC})
    add_executable(${TARGET_NAME} ${SOURCES_COMMON} ${SOURCES_SPECIFIC})
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(${TARGET_NAME} PRIVATE 
        CSV_PATH="${CMAKE_SOURCE_DIR}/matlab_visualization/"
        DATA_DIRECTORY="${DATA_DIRECTORY}"
        ${DEFINE}
    )
endfunction()



# OpenMP
find_package(OpenMP REQUIRED)
set (SOURCES_OPENMP
    implementation/omp/updateOmpAtomics.h
    implementation/omp/updateOmpReduction.h
)
add_kmeans_executable(kmeans_openmp "${SOURCES_OPENMP}" OPENMP)
target_link_libraries(kmeans_openmp PRIVATE OpenMP::OpenMP_CXX)



# OpenCL
find_package(OpenCL REQUIRED)
set (SOURCES_OPENCL
    implementation/ocl/updateOcl.h
)
add_kmeans_executable(kmeans_ocl "${SOURCES_OPENCL}" OPENCL)
target_link_libraries(kmeans_ocl PRIVATE OpenCL::OpenCL)
