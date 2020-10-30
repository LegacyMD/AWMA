#pragma once

#include "implementation/implementation.h"

inline bool updateOmpReduction(PointBundleVector &pointBundles, std::vector<Centroid> &centroids, size_t numberOfBundles, size_t numberOfClusters) {
#pragma omp parallel for default(none) shared(pointBundles) shared(centroids) firstprivate(numberOfBundles), firstprivate(numberOfClusters)
    for (int bundleIndex = 0; bundleIndex < numberOfBundles; bundleIndex++) {
        PointBundle &bundle = pointBundles[bundleIndex];
        for (size_t pointIndex = 0; pointIndex < pointBundles.getPointsCountInCurrentBundle(bundleIndex); pointIndex++) {
            Point &point = bundle.points[pointIndex];

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

            point.clusterLabel = centroids[nearestCentroidIndex].clusterLabel;
        }
    }

    bool changed = false;
    for (int centroidIndex = 0u; centroidIndex < numberOfClusters; centroidIndex++) {
        Centroid &centroid = centroids[centroidIndex];
        Coordinate x = 0;
        Coordinate y = 0;
        int divisor = 0;

        // clang-format off
#pragma omp parallel for default(none) shared(centroid) shared(numberOfBundles) shared(pointBundles) reduction(+ : x) reduction(+ : y) reduction(+ : divisor)
        // clang-format on
        for (int bundleIndex = 0; bundleIndex < numberOfBundles; bundleIndex++) {
            PointBundle &bundle = pointBundles[bundleIndex];
            for (size_t pointIndex = 0; pointIndex < pointBundles.getPointsCountInCurrentBundle(bundleIndex); pointIndex++) {
                const Point &point = bundle.points[pointIndex];
                if (point.clusterLabel == centroid.clusterLabel) {
                    x += point.x;
                    y += point.y;
                    divisor++;
                }
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
