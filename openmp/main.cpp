#include "definitions.h"

#include "implementation/implementation.h"
#include "utils/parameters.h"
#include "utils/random.h"
#include "utils/timer.h"

#include <fstream>
#include <iostream>
#include <omp.h>
#include <sstream>
#include <string>
#include <thread>

void dumpCsv(size_t iteration, const std::vector<Point> &points) {
    const std::string fileName = std::string{CSV_PATH} + "points_" + std::to_string(iteration) + ".csv";
    std::ofstream file{fileName};
    for (const Point &point : points) {
        file << point.clusterLabel << ','
             << point.x << ','
             << point.y << '\n';
    }
}

bool init(std::vector<Point> &points, std::vector<Centroid> &centroids, const Parameters &params) {
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
        points.push_back(Point{invalidLabel, x, y});

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
    Implementation update = getImplementation(params.implementationIndex);
    params.display();
    Timer timer{};

    // Generate points and centroids
    std::vector<Point> points = {};
    std::vector<Centroid> centroids = {};
    timer.start();
    if (!init(points, centroids, params)) {
        return 1;
    }
    timer.end();
    const auto totalInitTimeUs = timer.getUs().count();

    // Display what has been loaded
    std::cout << "Loaded data:\n"
              << '\t' << points.size() << " points\n"
              << '\t' << centroids.size() << " clusters\n"
              << std::endl;

    // Print general system info
    std::cout << "Execution environment:\n";
    std::cout << "\tstd::thread::hardware_concurrency = " << std::thread::hardware_concurrency() << '\n';
    std::cout << "\tomp_get_num_procs = " << omp_get_num_procs() << '\n';
    std::cout << "\tomp_get_max_threads = " << omp_get_max_threads() << '\n';
    std::cout << std::endl;

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
        converged = !update(points, centroids, points.size(), centroids.size());
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
