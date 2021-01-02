#pragma once

#include "implementation/implementation.h"
#include "implementation/ocl/utils.h"

#include <CL/cl.h>
#include <iostream>
#include <string>

#define ASSERT_TRUE(condition, message)                   \
    if (!(condition)) {                                   \
        std::cerr << "ERROR: " << (message) << std::endl; \
        throw 0;                                          \
    }

#define ASSERT_CL_SUCCESS_WITH_MESSAGE(statement, message)                                                                        \
    {                                                                                                                             \
        const auto retValASDF = statement;                                                                                        \
        ASSERT_TRUE(retValASDF == CL_SUCCESS, std::string(#statement) + " retVal=" + std::to_string(retValASDF) + " " + message); \
    }

#define ASSERT_CL_SUCCESS(statement) ASSERT_CL_SUCCESS_WITH_MESSAGE((statement), "")

struct OclImplementation : Implementation {
    struct OclData {
        cl_platform_id platform;
        cl_device_id device;
        cl_context context;
        cl_command_queue queue;

        cl_program program;
        cl_kernel kernel;

        cl_mem points;
        cl_mem pointLabels[2];
        cl_mem centroids[2];
        cl_uint numberOfPoints;
        cl_uint numberOfClusters;
        int currentBufferIndex = 0;
    };

    void *upload(const std::vector<Point> &points, std::vector<Label> &pointLabels,
                 std::vector<Centroid> &centroids, size_t numberOfPoints, size_t numberOfClusters) override {
        auto data = new OclData();

        cl_uint platformsCount = {};
        ASSERT_CL_SUCCESS(clGetPlatformIDs(0, nullptr, &platformsCount));
        ASSERT_TRUE(platformsCount != 0, "no platforms found");
        ASSERT_CL_SUCCESS(clGetPlatformIDs(1, &data->platform, nullptr));

        cl_uint devicesCount = {};
        ASSERT_CL_SUCCESS(clGetDeviceIDs(data->platform, CL_DEVICE_TYPE_GPU, 0, nullptr, &devicesCount));
        ASSERT_TRUE(devicesCount != 0, "no devices found");
        ASSERT_CL_SUCCESS(clGetDeviceIDs(data->platform, CL_DEVICE_TYPE_GPU, 1u, &data->device, nullptr));

        cl_int retVal{};
        data->context = clCreateContext(nullptr, 1, &data->device, nullptr, nullptr, &retVal);
        ASSERT_CL_SUCCESS(retVal);

        data->queue = clCreateCommandQueue(data->context, data->device, 0, &retVal);
        ASSERT_CL_SUCCESS(retVal);

        const auto programSource = OclUtils::readFile("kernel.cl");
        auto programSourcePtr = programSource.c_str();
        const auto programSourceSize = programSource.size();
        std::ostringstream programOptionsStream{};
        programOptionsStream
            << "-DPOINTS_STRIDE=" << sizeof(points[0]) << " "
            << "-DPOINT_LABELS_STRIDE=" << sizeof(pointLabels[0]) << " "
            << "-DCENTROIDS_STRIDE=" << sizeof(centroids[0]) << " ";
        const auto programOptions = programOptionsStream.str();
        data->program = clCreateProgramWithSource(data->context, 1, &programSourcePtr, &programSourceSize, &retVal);
        ASSERT_CL_SUCCESS(retVal);
        retVal = clBuildProgram(data->program, 1, &data->device, programOptions.c_str(), nullptr, nullptr);
        if (retVal != CL_SUCCESS) {
            size_t logSize{};
            ASSERT_CL_SUCCESS(clGetProgramBuildInfo(data->program, data->device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize));
            auto log = std::make_unique<char[]>(logSize);
            ASSERT_CL_SUCCESS(clGetProgramBuildInfo(data->program, data->device, CL_PROGRAM_BUILD_LOG, logSize, log.get(), nullptr));
            ASSERT_CL_SUCCESS_WITH_MESSAGE(retVal, log.get());
        }
        data->kernel = clCreateKernel(data->program, "main", &retVal);
        ASSERT_CL_SUCCESS(retVal);

        const auto pointsSize = OclUtils::getTotalVectorSizeInBytes(points);
        data->points = clCreateBuffer(data->context, CL_MEM_READ_ONLY, pointsSize, nullptr, &retVal);
        ASSERT_CL_SUCCESS(retVal);
        ASSERT_CL_SUCCESS(clEnqueueWriteBuffer(data->queue, data->points, CL_BLOCKING, 0, pointsSize, points.data(), 0, nullptr, nullptr));

        const auto pointLabelsSize = OclUtils::getTotalVectorSizeInBytes(pointLabels);
        data->pointLabels[0] = clCreateBuffer(data->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, pointLabelsSize, pointLabels.data(), &retVal);
        data->pointLabels[1] = clCreateBuffer(data->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, pointLabelsSize, pointLabels.data(), &retVal);

        const auto centroidsSize = OclUtils::getTotalVectorSizeInBytes(centroids);
        data->centroids[0] = clCreateBuffer(data->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, centroidsSize, centroids.data(), &retVal);
        data->centroids[1] = clCreateBuffer(data->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, centroidsSize, centroids.data(), &retVal);

        data->numberOfPoints = numberOfPoints;

        data->numberOfClusters = numberOfClusters;

        return data;
    }

    void cleanup(void *data) override {
        OclData *oclData = static_cast<OclData *>(data);

        ASSERT_CL_SUCCESS(clReleaseMemObject(oclData->centroids[0]));
        ASSERT_CL_SUCCESS(clReleaseMemObject(oclData->centroids[1]));
        ASSERT_CL_SUCCESS(clReleaseMemObject(oclData->pointLabels[0]));
        ASSERT_CL_SUCCESS(clReleaseMemObject(oclData->pointLabels[1]));
        ASSERT_CL_SUCCESS(clReleaseMemObject(oclData->points));

        ASSERT_CL_SUCCESS(clReleaseKernel(oclData->kernel));
        ASSERT_CL_SUCCESS(clReleaseProgram(oclData->program));

        ASSERT_CL_SUCCESS(clReleaseCommandQueue(oclData->queue));
        ASSERT_CL_SUCCESS(clReleaseContext(oclData->context));

        delete oclData;
    }

    bool update(void *rawData) override {
        OclData &data = *static_cast<OclData *>(rawData);

        const auto inputIndex = data.currentBufferIndex;
        const auto outputIndex = !data.currentBufferIndex;
        data.currentBufferIndex = outputIndex;

        ASSERT_CL_SUCCESS(clSetKernelArg(data.kernel, 0, sizeof(data.points), &data.points));
        ASSERT_CL_SUCCESS(clSetKernelArg(data.kernel, 1, sizeof(data.pointLabels[inputIndex]), &data.pointLabels[inputIndex]));
        ASSERT_CL_SUCCESS(clSetKernelArg(data.kernel, 2, sizeof(data.centroids[inputIndex]), &data.centroids[inputIndex]));
        ASSERT_CL_SUCCESS(clSetKernelArg(data.kernel, 3, sizeof(data.numberOfPoints), &data.numberOfPoints));
        ASSERT_CL_SUCCESS(clSetKernelArg(data.kernel, 4, sizeof(data.numberOfClusters), &data.numberOfClusters));
        ASSERT_CL_SUCCESS(clSetKernelArg(data.kernel, 5, sizeof(data.pointLabels[outputIndex]), &data.pointLabels[outputIndex]));
        ASSERT_CL_SUCCESS(clSetKernelArg(data.kernel, 6, sizeof(data.centroids[outputIndex]), &data.centroids[outputIndex]));

        const size_t *gwo = nullptr;
        const size_t gws = data.numberOfPoints;
        const size_t *lws = nullptr;
        ASSERT_CL_SUCCESS(clEnqueueNDRangeKernel(data.queue, data.kernel, 1, gwo, &gws, lws, 0, nullptr, nullptr));

        ASSERT_CL_SUCCESS(clFinish(data.queue));
    }
};
