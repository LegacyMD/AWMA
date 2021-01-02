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
    if (MSVC)
        set_property(TARGET ${TARGET_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:kmeans_ocl>)
    endif()
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
set(SOURCES_OPENCL
    implementation/ocl/updateOcl.h
    implementation/ocl/utils.h
)
set (SOURCES_OPENCL_KERNELS
    implementation/ocl/kernel.cl
)
add_kmeans_executable(kmeans_ocl "${SOURCES_OPENCL};${SOURCES_OPENCL_KERNELS}" OPENCL)
target_link_libraries(kmeans_ocl PRIVATE OpenCL::OpenCL)
foreach(KERNEL ${SOURCES_OPENCL_KERNELS})
    get_filename_component(FILE_NAME ${KERNEL} NAME)
    add_custom_command(
        TARGET kmeans_ocl
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${KERNEL} $<TARGET_FILE_DIR:kmeans_ocl>/${FILE_NAME}
        COMMENT "Copying kernels"
        VERBATIM
    )
endforeach()
