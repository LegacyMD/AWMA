#include "definitions.h"

#include "implementation/implementation.h"
#include "utils/parameters.h"
#include "utils/random.h"
#include "utils/timer.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

void dumpCsv(size_t iteration, Implementation &implementation, void *data) {
    auto iterationData = implementation.getIterationData(data);

    const std::string fileName = std::string{CSV_PATH} + "points_" + std::to_string(iteration) + ".csv";
    std::ofstream file{fileName};
    for (auto i = 0u; i < iterationData.points.size(); i++) {
        file << iterationData.pointLabels[i] << ','
             << iterationData.points[i].x << ','
             << iterationData.points[i].y << '\n';
    }

    const std::string fileNameCentroids = std::string{CSV_PATH} + "centroids_" + std::to_string(iteration) + ".csv";
    std::ofstream fileCentroids{fileNameCentroids};
    for (const Centroid &centroid : iterationData.centroids) {
        fileCentroids << centroid.clusterLabel << ','
                      << centroid.x << ','
                      << centroid.y << '\n';
    }
}

bool init(std::vector<Point> &points, std::vector<Label> &pointLabels, std::vector<Centroid> &centroids, const Parameters &params) {
    // Open file
    std::ifstream file(std::string(DATA_DIRECTORY) + params.inputFileName);
    if (!file) {
        std::cerr << "Error analyzing file: could not open\n";
        return false;
    }

    // Load centroids count
    size_t centroidsCount;
    file >> centroidsCount;
    if (!file) {
        std::cerr << "Error analyzing file: could not load centroids count\n";
        return false;
    }
    file.ignore(1, '\n');

    // Load all points
    Coordinate minX{}, maxX{}, minY{}, maxY{};
    while (true) {
        // Load line
        std::string line{};
        std::getline(file, line);
        if (!file) {
            if (file.eof()) {
                break;
            }

            std::cerr << "Error analyzing file: could not load line\n";
            return false;
        }

        // Parse line
        std::istringstream lineStream{line};
        Coordinate x{}, y{};
        lineStream >> x >> y;
        if (!lineStream) {
            std::cerr << "Error analyzing file: could not parse line\n";
            return false;
        }

        // Add point
        points.push_back(Point{x, y});
        pointLabels.push_back(invalidLabel);

        // Update maxes and mins
        minX = std::min(minX, x);
        maxX = std::max(maxX, x);
        minY = std::min(minY, y);
        maxY = std::max(maxY, y);
    }

    // Randomly generate centroids
    centroids.reserve(centroidsCount);
    for (Label clusterIndex = 0; clusterIndex < centroidsCount; clusterIndex++) {
        centroids.push_back(Centroid{clusterIndex,
                                     RandomHelper::random<Coordinate>(minX, maxX),
                                     RandomHelper::random<Coordinate>(minY, maxY)});
    }

    return true;
}

int main(int argc, const char **argv) {
    // Setup parameters
    Parameters params = {};
    params.parseCommandLine(argc, argv);
    if (params.hasRandomSeed) {
        RandomHelper::init(params.randomSeed);
    }
    auto implementation = getImplementation(params.implementationIndex);
    if (params.verbose) {
        params.display();
    }
    Timer timer{};

    // Generate points and centroids
    std::vector<Point> points = {};
    std::vector<Label> pointLabels = {};
    std::vector<Centroid> centroids = {};
    timer.start();
    if (!init(points, pointLabels, centroids, params)) {
        return 1;
    }
    timer.end();
    const auto totalInitTimeUs = timer.getUs().count();

    // Upload data
    timer.start();
    auto data = implementation->upload(points, pointLabels, centroids, points.size(), centroids.size());
    timer.end();
    const auto totalUploadTimeUs = timer.getUs().count();

    // Display what has been loaded
    if (params.verbose) {
        std::cout << "Loaded data:\n"
                  << '\t' << points.size() << " points\n"
                  << '\t' << centroids.size() << " clusters\n"
                  << std::endl;
    }

    // Run clustering
    if (params.verbose) {
        std::cout << "Running clustering:\n";
    }
    bool converged = false;
    size_t iteration = 0;
    unsigned long long totalCsvTimeUs = 0;
    unsigned long long totalClusteringTimeUs = 0;
    for (; iteration < params.maxIterations && !converged; iteration++) {
        timer.start();
        if (params.writeCsv) {
            dumpCsv(iteration, *implementation, data);
        }
        timer.end();
        totalCsvTimeUs += timer.getUs().count();

        timer.start();
        converged = !implementation->update(data);
        timer.end();
        totalClusteringTimeUs += timer.getUs().count();

        if (params.verbose) {
            std::cout << "\tIteration " << iteration << ": " << timer.getUs().count() << "us" << std::endl;
        }
    }
    if (params.verbose) {
        std::cout << std::endl;
    }

    // Cleanup
    implementation->cleanup(data);

    // Print total timings
    if (params.verbose) {
        std::cout << "Summary:\n";
        std::cout << "\tInit time: " << totalInitTimeUs << "us = " << totalInitTimeUs / 1000 << "ms\n";
        std::cout << "\tUpload time: " << totalUploadTimeUs << "us = " << totalUploadTimeUs / 1000 << "ms\n";
        std::cout << "\tClustering time: " << totalClusteringTimeUs << "us = " << totalClusteringTimeUs / 1000 << "ms\n";
        std::cout << "\tCsv writing time: " << totalCsvTimeUs << "us = " << totalCsvTimeUs / 1000 << "ms\n\n";
    } else {
        std::cout << totalClusteringTimeUs << '\n';
    }

    // Print info about convergence
    if (converged) {
        if (params.verbose) {
            std::cout << "Achieved convergence after " << iteration << " iterations.\n";
        }
        return 0;
    } else {
        if (params.verbose) {
            std::cout << "Did not achieve convergence after " << iteration << " iterations.\n";
        }
        return 1;
    }
}
