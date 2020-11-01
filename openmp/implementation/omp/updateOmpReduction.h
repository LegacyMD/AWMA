#pragma once

#include "implementation/implementation.h"

inline bool updateOmpReduction(const std::vector<Point> &points, std::vector<Label> &pointLabels, std::vector<Centroid> &centroids, size_t numberOfPoints, size_t numberOfClusters) {
#pragma omp parallel for default(none) shared(points) shared(pointLabels) shared(centroids) firstprivate(numberOfPoints), firstprivate(numberOfClusters)
    for (int pointIndex = 0; pointIndex < numberOfPoints; pointIndex++) {
        const Point &point = points[pointIndex];

        Coordinate nearestCentroidDistance = std::numeric_limits<Coordinate>::max();
        size_t nearestCentroidIndex = 0;
        for (size_t centroidIndex = 0u; centroidIndex < numberOfClusters; centroidIndex++) {
            const Centroid &centroid = centroids[centroidIndex];
            const Coordinate centroidDistance = distance(point, centroid);
            if (centroidDistance < nearestCentroidDistance) {
                nearestCentroidDistance = centroidDistance;
                nearestCentroidIndex = centroid.clusterLabel;
            }
        }

        pointLabels[pointIndex] = centroids[nearestCentroidIndex].clusterLabel;
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
