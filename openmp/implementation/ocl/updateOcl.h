#pragma once

#include "implementation/implementation.h"

#include <CL/cl.h>
#include <cstdint>

class OclImplementation : public Implementation {
private:
    struct OclData {
        // OpenCL Context
        cl_platform_id platform;
        cl_device_id device;
        cl_context context;
        cl_command_queue queue;

        // Kernels
        cl_program program;
        cl_kernel kernelFindClosestClusters;
        cl_kernel kernelUpdateCentroids;

        // Allocations: core data
        cl_mem points;
        cl_mem pointLabels;
        cl_mem centroidCoords[2];
        cl_mem centroidLabels;

        // Allocation: temporary data
        cl_mem centroidSums;
        cl_mem centroidDivisors;
        cl_mem centroidChanged;

        // Buffer sizes
        size_t pointsSize;
        size_t pointLabelsSize;
        size_t centroidCoordsSize;
        size_t centroidLabelsSize;
        size_t centroidSumsSize;
        size_t centroidDivisorsSize;
        size_t centroidChangedSize;

        // Other numeric data
        cl_uint numberOfPoints;
        cl_uint numberOfClusters;
        int currentBufferIndex;

        // Iteration data (for returning to dump csv each frame)
        std::vector<Point> pointsToReturn;
        std::vector<Label> pointLabelsToReturn;
        std::vector<Centroid> centroidsToReturn;
    };

    void uploadOclContext(OclData *data);
    void uploadOclProgram(OclData *data);
    void uploadOclBuffers(OclData *data, const std::vector<Point> &points, std::vector<Label> &pointLabels,
                          std::vector<Centroid> &centroids, size_t numberOfPoints, size_t numberOfClusters);

public:
    void *upload(const std::vector<Point> &points, std::vector<Label> &pointLabels,
                 std::vector<Centroid> &centroids, size_t numberOfPoints, size_t numberOfClusters) override;

    void cleanup(void *data) override;

    IterationData getIterationData(void *rawData) override;

    bool update(void *rawData) override;
};
