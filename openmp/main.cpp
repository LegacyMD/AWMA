#include "definitions.h"
#include "random.h"
#include "parameters.h"

#include <fstream>
#include <iostream>
#include <omp.h>
#include <string>

Coordinate distance(const Point &point, const Centroid &centroid) {
    const auto x = point.x - centroid.x;
    const auto y = point.y - centroid.y;
    return x * x + y * y; // no sqrt, because it's only for comparisons
}

bool update(std::vector<Point> &points, std::vector<Centroid> &centroids, size_t numberOfPoints, size_t numberOfClusters) {
    struct NewCentroidPositionData {
        Coordinate x = 0;
        Coordinate y = 0;
        int divisor = 0;
    };
    std::vector<NewCentroidPositionData> newCentroidPositions(centroids.size());

#pragma omp parallel for default(none) shared(points) shared(centroids) firstprivate(numberOfPoints), firstprivate(numberOfClusters) shared(newCentroidPositions)
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

        points[pointIndex].clusterLabel = centroids[nearestCentroidIndex].clusterLabel;
#pragma omp atomic
        newCentroidPositions[nearestCentroidIndex].x += point.x;
#pragma omp atomic
        newCentroidPositions[nearestCentroidIndex].y += point.y;
#pragma omp atomic
        newCentroidPositions[nearestCentroidIndex].divisor++;
    }

    bool changed = false;
    // clang-format off
#pragma omp parallel for default(none) shared(centroids) shared(newCentroidPositions) reduction(|| : changed)
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

void dumpCsv(size_t iteration, const std::vector<Point> &points) {
    const std::string fileName = std::string{CSV_PATH} + "points_" + std::to_string(iteration) + ".csv";
    std::ofstream file{fileName};
    for (const Point &point : points) {
        file << point.clusterLabel << ','
             << point.x << ','
             << point.y << '\n';
    }
}

int main(int argc, const char **argv) {
    Parameters params = {};
    params.parseCommandLine(argc, argv);
    params.display();

    // Generate points
    std::vector<Point> points = {};
    points.reserve(params.numberOfPoints);
    for (size_t pointIndex = 0; pointIndex < params.numberOfPoints; pointIndex++) {
        points.push_back(Point{invalidLabel,
                               RandomHelper::random<Coordinate>(params.minX, params.maxX),
                               RandomHelper::random<Coordinate>(params.minY, params.maxY)});
    }

    // Generate centroids
    std::vector<Centroid> centroids = {};
    points.reserve(params.numberOfClusters);
    for (Label clusterIndex = 0; clusterIndex < params.numberOfClusters; clusterIndex++) {
        centroids.push_back(Centroid{clusterIndex,
                                     RandomHelper::random<Coordinate>(params.minX, params.maxX),
                                     RandomHelper::random<Coordinate>(params.minY, params.maxY)});
    }

    bool converged = false;
    for (size_t iteration = 0; iteration < params.maxIterations; iteration++) {
        converged = !update(points, centroids, params.numberOfPoints, params.numberOfClusters);
        if (converged) {
            std::cout << "Achieved convergence in iteration " << iteration << ".\n";
            break;
        }

        dumpCsv(iteration, points);
    }

    if (!converged) {
        std::cout << "Did not achieve convergence after " << params.maxIterations << " iterations.\n";
    }
}
