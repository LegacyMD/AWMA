#pragma once

#include "implementation/implementation.h"

inline bool updateOmpAtomics(PointBundleVector &pointBundles, std::vector<Centroid> &centroids, size_t numberOfBundles, size_t numberOfClusters) {
    struct NewCentroidPositionData {
        Coordinate x = 0;
        Coordinate y = 0;
        int divisor = 0;
    };
    std::vector<NewCentroidPositionData> newCentroidPositions(centroids.size());

#pragma omp parallel for default(none) shared(pointBundles) shared(centroids) firstprivate(numberOfBundles), firstprivate(numberOfClusters) shared(newCentroidPositions)
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
#pragma omp atomic
            newCentroidPositions[nearestCentroidIndex].x += point.x;
#pragma omp atomic
            newCentroidPositions[nearestCentroidIndex].y += point.y;
#pragma omp atomic
            newCentroidPositions[nearestCentroidIndex].divisor++;
        }
    }

    bool changed = false;
    // clang-format off
#pragma omp parallel for default(none) shared(centroids) shared(newCentroidPositions) shared(numberOfClusters) reduction(|| : changed)
    // clang-format on
    for (int centroidIndex = 0u; centroidIndex < numberOfClusters; centroidIndex++) {
        NewCentroidPositionData &newPositionData = newCentroidPositions[centroidIndex];
        if (newPositionData.divisor != 0) {
            Centroid &centroid = centroids[centroidIndex];
            newPositionData.x /= newPositionData.divisor;
            newPositionData.y /= newPositionData.divisor;
            if ((newPositionData.x != centroid.x) || (newPositionData.y != centroid.y)) {
                centroid.x = newPositionData.x;
                centroid.y = newPositionData.y;
                changed = true;
            }
        }
    }

    return changed;
}
