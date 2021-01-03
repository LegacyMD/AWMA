#pragma once

#include "definitions.h"

#include <memory>
#include <string>
#include <vector>

struct Implementation {
    // Uploads all data to the device
    virtual void *upload(const std::vector<Point> &points, std::vector<Label> &pointLabels,
                         std::vector<Centroid> &centroids, size_t numberOfPoints, size_t numberOfClusters) = 0;

    // Returns true if some centroid changed
    virtual bool update(void *data) = 0;

    struct IterationData {
        const std::vector<Point> &points;
        const std::vector<Label> &pointLabels;
        const std::vector<Centroid> &centroids;
    };
    virtual IterationData getIterationData(void *data) = 0;

    // De-allocated all memory allocated by UploadFunction
    virtual void cleanup(void *data) = 0;
};

struct CpuImplementation : Implementation {
    struct CpuData {
        CpuData(const std::vector<Point> &points, std::vector<Label> &pointLabels,
                std::vector<Centroid> &centroids, size_t numberOfPoints, size_t numberOfClusters)
            : points(points),
              pointLabels(pointLabels),
              centroids(centroids),
              numberOfPoints(numberOfPoints),
              numberOfClusters(numberOfClusters) {}

        const std::vector<Point> &points;
        std::vector<Label> &pointLabels;
        std::vector<Centroid> &centroids;
        size_t numberOfPoints;
        size_t numberOfClusters;
    };

    void *upload(const std::vector<Point> &points, std::vector<Label> &pointLabels,
                 std::vector<Centroid> &centroids, size_t numberOfPoints, size_t numberOfClusters) override {
        return new CpuData(points, pointLabels, centroids, numberOfPoints, numberOfClusters);
    }

    virtual bool cpuUpdate(CpuData &data) = 0;
    bool update(void *data) {
        return cpuUpdate(*static_cast<CpuData *>(data));
    }

    IterationData getIterationData(void *rawData) override {
        CpuData *data = static_cast<CpuData *>(rawData);
        return IterationData{
            data->points,
            data->pointLabels,
            data->centroids};
    }

    void cleanup(void *data) override {
        delete data;
    }
};

// Helper functions
std::unique_ptr<Implementation> getImplementation(size_t index);
std::string getImplementationString(size_t index);

Coordinate distance(const Point &point, const Centroid &centroid);
