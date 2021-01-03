#include "updateOcl.h"

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

void OclImplementation::uploadOclContext(OclData *data) {
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
}

void OclImplementation::uploadOclProgram(OclData *data) {
    cl_int retVal{};

    const auto programSource = OclUtils::readFile("kernel.cl");
    auto programSourcePtr = programSource.c_str();
    const auto programSourceSize = programSource.size();
    data->program = clCreateProgramWithSource(data->context, 1, &programSourcePtr, &programSourceSize, &retVal);
    ASSERT_CL_SUCCESS(retVal);
    retVal = clBuildProgram(data->program, 1, &data->device, nullptr, nullptr, nullptr);
    if (retVal != CL_SUCCESS) {
        size_t logSize{};
        ASSERT_CL_SUCCESS(clGetProgramBuildInfo(data->program, data->device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize));
        auto log = std::make_unique<char[]>(logSize);
        ASSERT_CL_SUCCESS(clGetProgramBuildInfo(data->program, data->device, CL_PROGRAM_BUILD_LOG, logSize, log.get(), nullptr));
        ASSERT_CL_SUCCESS_WITH_MESSAGE(retVal, log.get());
    }
    data->kernelFindClosestClusters = clCreateKernel(data->program, "findClosestClusters", &retVal);
    data->kernelUpdateCentroids = clCreateKernel(data->program, "updateCentroids", &retVal);
    ASSERT_CL_SUCCESS(retVal);
}

void OclImplementation::uploadOclBuffers(OclData *data, const std::vector<Point> &points, std::vector<Label> &pointLabels,
                                         std::vector<Centroid> &centroids, size_t numberOfPoints, size_t numberOfClusters) {
    cl_int retVal{};

    // Points
    data->pointsSize = OclUtils::getTotalVectorSizeInBytes(points);
    data->points = clCreateBuffer(data->context, CL_MEM_READ_ONLY, data->pointsSize, nullptr, &retVal);
    ASSERT_CL_SUCCESS(retVal);
    ASSERT_CL_SUCCESS(clEnqueueWriteBuffer(data->queue, data->points, CL_BLOCKING, 0, data->pointsSize, points.data(), 0, nullptr, nullptr));

    // Point labels
    data->pointLabelsSize = OclUtils::getTotalVectorSizeInBytes(pointLabels);
    data->pointLabels = clCreateBuffer(data->context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, data->pointLabelsSize, pointLabels.data(), &retVal);

    // CentroidCoords and CentroidLabels (split from Centroids)
    std::vector<Coordinate> centroidCoords = {};
    std::vector<Label> centroidLabels = {};
    for (const auto &centroid : centroids) {
        centroidCoords.push_back(centroid.x);
        centroidCoords.push_back(centroid.y);
        centroidLabels.push_back(centroid.clusterLabel);
    }
    data->centroidCoordsSize = OclUtils::getTotalVectorSizeInBytes(centroidCoords);
    data->centroidCoords[0] = clCreateBuffer(data->context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, data->centroidCoordsSize, centroidCoords.data(), &retVal);
    data->centroidCoords[1] = clCreateBuffer(data->context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, data->centroidCoordsSize, centroidCoords.data(), &retVal);
    data->centroidLabelsSize = OclUtils::getTotalVectorSizeInBytes(centroidLabels);
    data->centroidLabels = clCreateBuffer(data->context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, data->centroidLabelsSize, centroidLabels.data(), &retVal);

    // CentroidSums (temporary)
    data->centroidSumsSize = data->centroidCoordsSize;
    data->centroidSums = clCreateBuffer(data->context, CL_MEM_READ_WRITE, data->centroidSumsSize, nullptr, &retVal);
    ASSERT_CL_SUCCESS(retVal);

    // CentroidDivisors (temporary)
    data->centroidDivisorsSize = sizeof(cl_uint) * centroids.size();
    data->centroidDivisors = clCreateBuffer(data->context, CL_MEM_READ_WRITE, data->centroidDivisorsSize, nullptr, &retVal);
    ASSERT_CL_SUCCESS(retVal);

    // CentroidChange (temporary)
    data->centroidChangedSize = sizeof(cl_uint);
    data->centroidChanged = clCreateBuffer(data->context, CL_MEM_WRITE_ONLY, data->centroidChangedSize, nullptr, &retVal);
    ASSERT_CL_SUCCESS(retVal);

    // Other numeric data
    data->numberOfPoints = static_cast<cl_uint>(numberOfPoints);
    data->numberOfClusters = static_cast<cl_uint>(numberOfClusters);
    data->currentBufferIndex = 0u;
}

void *OclImplementation::upload(const std::vector<Point> &points, std::vector<Label> &pointLabels, std::vector<Centroid> &centroids, size_t numberOfPoints, size_t numberOfClusters) {
    auto data = new OclData();
    uploadOclContext(data);
    uploadOclProgram(data);
    uploadOclBuffers(data, points, pointLabels, centroids, numberOfPoints, numberOfClusters);
    return data;
}

void OclImplementation::cleanup(void *data) {
    OclData *oclData = static_cast<OclData *>(data);

    ASSERT_CL_SUCCESS(clReleaseMemObject(oclData->centroidChanged));
    ASSERT_CL_SUCCESS(clReleaseMemObject(oclData->centroidDivisors));
    ASSERT_CL_SUCCESS(clReleaseMemObject(oclData->centroidSums));

    ASSERT_CL_SUCCESS(clReleaseMemObject(oclData->centroidLabels));
    ASSERT_CL_SUCCESS(clReleaseMemObject(oclData->centroidCoords[1]));
    ASSERT_CL_SUCCESS(clReleaseMemObject(oclData->centroidCoords[0]));
    ASSERT_CL_SUCCESS(clReleaseMemObject(oclData->pointLabels));
    ASSERT_CL_SUCCESS(clReleaseMemObject(oclData->points));

    ASSERT_CL_SUCCESS(clReleaseKernel(oclData->kernelFindClosestClusters));
    ASSERT_CL_SUCCESS(clReleaseKernel(oclData->kernelUpdateCentroids));
    ASSERT_CL_SUCCESS(clReleaseProgram(oclData->program));

    ASSERT_CL_SUCCESS(clReleaseCommandQueue(oclData->queue));
    ASSERT_CL_SUCCESS(clReleaseContext(oclData->context));

    delete oclData;
}

OclImplementation::IterationData OclImplementation::getIterationData(void *rawData) {
    OclData &data = *static_cast<OclData *>(rawData);

    data.pointsToReturn.resize(data.numberOfPoints);
    ASSERT_CL_SUCCESS(clEnqueueReadBuffer(data.queue, data.points, CL_BLOCKING, 0, data.pointsSize, data.pointsToReturn.data(), 0, nullptr, nullptr));

    data.pointLabelsToReturn.resize(data.numberOfPoints);
    ASSERT_CL_SUCCESS(clEnqueueReadBuffer(data.queue, data.pointLabels, CL_BLOCKING, 0, data.pointLabelsSize, data.pointLabelsToReturn.data(), 0, nullptr, nullptr));

    std::vector<Coordinate> centroidCoords = {};
    std::vector<Label> centroidLabels = {};
    centroidCoords.resize(2 * data.numberOfClusters);
    centroidLabels.resize(data.numberOfClusters);
    ASSERT_CL_SUCCESS(clEnqueueReadBuffer(data.queue, data.centroidCoords[data.currentBufferIndex], CL_BLOCKING, 0, data.centroidCoordsSize, centroidCoords.data(), 0, nullptr, nullptr));
    ASSERT_CL_SUCCESS(clEnqueueReadBuffer(data.queue, data.centroidLabels, CL_BLOCKING, 0, data.centroidLabelsSize, centroidLabels.data(), 0, nullptr, nullptr));
    data.centroidsToReturn.clear();
    for (size_t i = 0; i < data.numberOfClusters; i++) {
        Centroid centroid{centroidLabels[i], centroidCoords[i * 2], centroidCoords[i * 2 + 1]};
        data.centroidsToReturn.push_back(centroid);
    }

    return IterationData{
        data.pointsToReturn,
        data.pointLabelsToReturn,
        data.centroidsToReturn};
}

bool OclImplementation::update(void *rawData) {
    OclData &data = *static_cast<OclData *>(rawData);
    const auto inputIndex = data.currentBufferIndex;
    const auto outputIndex = !data.currentBufferIndex;
    data.currentBufferIndex = outputIndex;

    // Clear temporary allocations
    const cl_uint zeroUint = 0u;
    const Coordinate zeroFloat = 0.0;
    ASSERT_CL_SUCCESS(clEnqueueFillBuffer(data.queue, data.centroidSums, &zeroFloat, sizeof(zeroFloat), 0, data.centroidSumsSize, 0, nullptr, nullptr));
    ASSERT_CL_SUCCESS(clEnqueueFillBuffer(data.queue, data.centroidDivisors, &zeroUint, sizeof(zeroUint), 0, data.centroidDivisorsSize, 0, nullptr, nullptr));
    ASSERT_CL_SUCCESS(clEnqueueFillBuffer(data.queue, data.centroidChanged, &zeroUint, sizeof(zeroUint), 0, data.centroidChangedSize, 0, nullptr, nullptr));

    // findClosestClusters
    cl_uint argIndex = 0;
    ASSERT_CL_SUCCESS(clSetKernelArg(data.kernelFindClosestClusters, argIndex++, sizeof(data.points), &data.points));                                         // in
    ASSERT_CL_SUCCESS(clSetKernelArg(data.kernelFindClosestClusters, argIndex++, sizeof(data.centroidCoords[inputIndex]), &data.centroidCoords[inputIndex])); // in
    ASSERT_CL_SUCCESS(clSetKernelArg(data.kernelFindClosestClusters, argIndex++, sizeof(data.numberOfClusters), &data.numberOfClusters));                     // in
    ASSERT_CL_SUCCESS(clSetKernelArg(data.kernelFindClosestClusters, argIndex++, sizeof(data.pointLabels), &data.pointLabels));                               // out
    ASSERT_CL_SUCCESS(clSetKernelArg(data.kernelFindClosestClusters, argIndex++, sizeof(data.centroidSums), &data.centroidSums));                             // out
    ASSERT_CL_SUCCESS(clSetKernelArg(data.kernelFindClosestClusters, argIndex++, sizeof(data.centroidDivisors), &data.centroidDivisors));                     // out
    const size_t *gwo = nullptr;
    size_t gws = data.numberOfPoints;
    const size_t *lws = nullptr;
    ASSERT_CL_SUCCESS(clEnqueueNDRangeKernel(data.queue, data.kernelFindClosestClusters, 1, gwo, &gws, lws, 0, nullptr, nullptr));

    // DEBUG CODE
    /*
    auto a = std::make_unique<cl_uint[]>(data.numberOfClusters);
    ASSERT_CL_SUCCESS(clEnqueueReadBuffer(data.queue, data.centroidDivisors, CL_BLOCKING, 0, data.centroidDivisorsSize, a.get(), 0, nullptr, nullptr));
    auto b = std::make_unique<Coordinate[]>(2 * data.numberOfClusters);
    ASSERT_CL_SUCCESS(clEnqueueReadBuffer(data.queue, data.centroidSums, CL_BLOCKING, 0, data.centroidSumsSize, b.get(), 0, nullptr, nullptr));
    auto c = std::make_unique<cl_uint[]>(data.numberOfPoints);
    ASSERT_CL_SUCCESS(clEnqueueReadBuffer(data.queue, data.pointLabels, CL_BLOCKING, 0, data.pointLabelsSize, c.get(), 0, nullptr, nullptr));
    */

    // updateCentroids
    argIndex = 0;
    ASSERT_CL_SUCCESS(clSetKernelArg(data.kernelUpdateCentroids, argIndex++, sizeof(data.centroidSums), &data.centroidSums));                               // in
    ASSERT_CL_SUCCESS(clSetKernelArg(data.kernelUpdateCentroids, argIndex++, sizeof(data.centroidDivisors), &data.centroidDivisors));                       // in
    ASSERT_CL_SUCCESS(clSetKernelArg(data.kernelUpdateCentroids, argIndex++, sizeof(data.centroidCoords[inputIndex]), &data.centroidCoords[inputIndex]));   // in
    ASSERT_CL_SUCCESS(clSetKernelArg(data.kernelUpdateCentroids, argIndex++, sizeof(data.numberOfPoints), &data.numberOfPoints));                           // in
    ASSERT_CL_SUCCESS(clSetKernelArg(data.kernelUpdateCentroids, argIndex++, sizeof(data.centroidCoords[outputIndex]), &data.centroidCoords[outputIndex])); // out
    ASSERT_CL_SUCCESS(clSetKernelArg(data.kernelUpdateCentroids, argIndex++, sizeof(data.centroidChanged), &data.centroidChanged));                         // out
    gws = data.numberOfClusters;
    ASSERT_CL_SUCCESS(clEnqueueNDRangeKernel(data.queue, data.kernelUpdateCentroids, 1, gwo, &gws, lws, 0, nullptr, nullptr));

    // Check if centroids changed
    cl_uint changed{};
    ASSERT_CL_SUCCESS(clEnqueueReadBuffer(data.queue, data.centroidChanged, CL_BLOCKING, 0, sizeof(changed), &changed, 0, nullptr, nullptr));
    return changed != 0;
}
