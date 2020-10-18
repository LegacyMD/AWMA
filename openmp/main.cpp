#include "definitions.h"
#include "parameters.h"
#include "random.h"
#include "timer.h"

#include <fstream>
#include <iostream>
#include <omp.h>
#include <string>
#include <thread>

Coordinate distance(const Point &point, const Centroid &centroid) {
    const auto x = point.x - centroid.x;
    const auto y = point.y - centroid.y;
    return x * x + y * y; // no sqrt, because it's only for comparisons
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

void init(std::vector<Point> &points, std::vector<Centroid> &centroids, const Parameters &params) {
    points.reserve(params.numberOfPoints);
    for (size_t pointIndex = 0; pointIndex < params.numberOfPoints; pointIndex++) {
        points.push_back(Point{invalidLabel,
                               RandomHelper::random<Coordinate>(params.minX, params.maxX),
                               RandomHelper::random<Coordinate>(params.minY, params.maxY)});
    }

    points.reserve(params.numberOfClusters);
    for (Label clusterIndex = 0; clusterIndex < params.numberOfClusters; clusterIndex++) {
        centroids.push_back(Centroid{clusterIndex,
                                     RandomHelper::random<Coordinate>(params.minX, params.maxX),
                                     RandomHelper::random<Coordinate>(params.minY, params.maxY)});
    }
}

using Algorithm = bool (*)(std::vector<Point> &points, std::vector<Centroid> &centroids, size_t numberOfPoints, size_t numberOfClusters);

bool update1(std::vector<Point> &points, std::vector<Centroid> &centroids, size_t numberOfPoints, size_t numberOfClusters) {
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

bool update2(std::vector<Point> &points, std::vector<Centroid> &centroids, size_t numberOfPoints, size_t numberOfClusters) {
#pragma omp parallel for default(none) shared(points) shared(centroids) firstprivate(numberOfPoints), firstprivate(numberOfClusters)
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
    }

    bool changed = false;
    for (int centroidIndex = 0u; centroidIndex < numberOfClusters; centroidIndex++) {
        Centroid &centroid = centroids[centroidIndex];
        Coordinate x = 0;
        Coordinate y = 0;
        int divisor = 0;

        // clang-format off
#pragma omp parallel for default(none) shared(centroid) shared(points) reduction(+ : x) reduction(+ : y) reduction(+ : divisor)
        // clang-format on
        for (int pointIndex = 0; pointIndex < numberOfPoints; pointIndex++) {
            const Point &point = points[pointIndex];
            if (point.clusterLabel == centroid.clusterLabel) {
                x += point.x;
                y += point.y;
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

int main(int argc, const char **argv) {
    // Setup parameters
    Parameters params = {};
    params.parseCommandLine(argc, argv);
    if (params.hasRandomSeed) {
        RandomHelper::init(params.randomSeed);
    }
    Algorithm updateAlgorithm = {};
    switch (params.algorithm) {
    case 1:
        updateAlgorithm = update1;
        break;
    case 2:
        updateAlgorithm = update2;
        break;
    default:
        std::cout << "ERROR Unknown algorithm selected\n";
        return 1;
    }
    params.display();
    Timer timer{};

    // Print general system info
    std::cout << "Execution environment:\n";
    std::cout << "\tstd::thread::hardware_concurrency = " << std::thread::hardware_concurrency() << '\n';
    std::cout << "\tomp_get_num_procs = " << omp_get_num_procs() << '\n';
    std::cout << "\tomp_get_max_threads = " << omp_get_max_threads() << '\n';
    std::cout << std::endl;

    // Generate points and centroids
    std::vector<Point> points = {};
    std::vector<Centroid> centroids = {};
    timer.start();
    init(points, centroids, params);
    timer.end();
    const auto totalInitTimeUs = timer.getUs().count();

    // Run clustering
    std::cout << "Running clustering:\n";
    bool converged = false;
    size_t iteration = 0;
    unsigned long long totalCsvTimeUs = 0;
    unsigned long long totalClusteringTimeUs = 0;
    for (; iteration < params.maxIterations && !converged; iteration++) {
        timer.start();
        if (params.writeCsv) {
            dumpCsv(iteration, points);
        }
        timer.end();
        totalCsvTimeUs += timer.getUs().count();

        timer.start();
        converged = !updateAlgorithm(points, centroids, params.numberOfPoints, params.numberOfClusters);
        timer.end();
        totalClusteringTimeUs += timer.getUs().count();

        std::cout << "\tIteration " << iteration << ": " << timer.getUs().count() << "us" << std::endl;
    }

    // Print total timings
    std::cout << "Summary:\n";
    std::cout << "\tInit time: " << totalInitTimeUs << "us = " << totalInitTimeUs / 1000 << "ms\n";
    std::cout << "\tClustering time: " << totalClusteringTimeUs << "us = " << totalClusteringTimeUs / 1000 << "ms\n";
    std::cout << "\tCsv writing time: " << totalCsvTimeUs << "us = " << totalCsvTimeUs / 1000 << "ms\n\n";

    // Print info about convergence
    if (converged) {
        std::cout << "Achieved convergence after " << iteration << " iterations.\n";
    } else {
        std::cout << "Did not achieve convergence after " << iteration << " iterations.\n";
    }
}
