#pragma once

#include "implementation/implementation.h"

struct OclImplementation : Implementation {
    void *upload(const std::vector<Point> &points, std::vector<Label> &pointLabels,
                 std::vector<Centroid> &centroids, size_t numberOfPoints, size_t numberOfClusters) override {
        return {};
    }

    bool update(void *data) override {
        return {};
    }

    void cleanup(void *data) override {
    }
};
