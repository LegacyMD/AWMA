#pragma once

#include "implementation/implementation.h"

inline bool updateOmpReduction(const std::vector<Point> &points, std::vector<Label> &pointLabels, std::vector<Centroid> &centroids, size_t numberOfPoints, size_t numberOfClusters) {
    constexpr static size_t cachelineSize = 64u;
    constexpr static size_t labelsPerCacheline = cachelineSize / sizeof(Label);
    static_assert(cachelineSize % sizeof(Label) == 0, "Cacheline size is not divisible by sizeof(Label)");

    const int cachelineCount = points.size() / labelsPerCacheline;

#pragma omp parallel for default(none) shared(points) shared(pointLabels) shared(centroids) firstprivate(numberOfPoints), firstprivate(numberOfClusters)
    for (int cachelineIndex = 0; cachelineIndex < cachelineCount; cachelineIndex++) {
        int pointIndex = cachelineIndex * labelsPerCacheline;

        for (int pointInsideCachelineIndex = 0; pointInsideCachelineIndex < labelsPerCacheline; pointInsideCachelineIndex++) {
            Coordinate nearestCentroidDistance = std::numeric_limits<Coordinate>::max();
            size_t nearestCentroidIndex = 0;
            for (size_t centroidIndex = 0u; centroidIndex < numberOfClusters; centroidIndex++) {
                const Centroid &centroid = centroids[centroidIndex];
                const Coordinate centroidDistance = distance(points[pointIndex], centroid);
                if (centroidDistance < nearestCentroidDistance) {
                    nearestCentroidDistance = centroidDistance;
                    nearestCentroidIndex = centroid.clusterLabel;
                }
            }

            pointLabels[pointIndex] = centroids[nearestCentroidIndex].clusterLabel;
            pointIndex++;
        }
    }

    bool changed = false;
    for (int centroidIndex = 0u; centroidIndex < numberOfClusters; centroidIndex++) {
        Centroid &centroid = centroids[centroidIndex];
        Coordinate x = 0;
        Coordinate y = 0;
        int divisor = 0;

        const auto centroidClusterLabel = centroid.clusterLabel;
        // clang-format off
#pragma omp parallel for default(none) shared(points) shared(pointLabels) firstprivate(centroidClusterLabel) firstprivate(numberOfPoints) reduction(+ : x) reduction(+ : y) reduction(+ : divisor)
        // clang-format on
        for (int pointIndex = 0; pointIndex < numberOfPoints; pointIndex++) {
            if (pointLabels[pointIndex] == centroidClusterLabel) {
                x += points[pointIndex].x;
                y += points[pointIndex].y;
                divisor++;
            }
        }

        if (divisor != 0) {
            x /= divisor;
            y /= divisor;

            if ((x != centroid.x) || (y != centroid.y)) {
                centroid.x = x;
                centroid.y = y;
                changed = true;
            }
        }
    }

    return changed;
}
