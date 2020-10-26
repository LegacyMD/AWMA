#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

template <typename T>
static bool parseParameter(const std::string &commandLineToken, T &parameter, const std::string &key) {
    const auto valueStart = commandLineToken.find('=');
    if (valueStart == std::string::npos) {
        return false;
    }

    if (commandLineToken.substr(0, valueStart) != key) {
        return false;
    }

    std::istringstream value{commandLineToken.substr(valueStart + 1)};
    value >> parameter;
    return true;
}

template <typename T>
auto random(std::mt19937 &gen, T min, T max) {
    return std::uniform_int_distribution<T>{min, max}(gen);
}

int main(int argc, char **argv) {
    // Parameters
    size_t minX = 0;
    size_t maxX = 50;
    size_t minY = 0;
    size_t maxY = 50;
    size_t numberOfPoints = 10000;
    size_t numberOfClusters = 20;
    unsigned int randomSeed = 0;
    bool hasRandomSeed = false;

    // Parse parameters
    for (int argIndex = 0; argIndex < argc; argIndex++) {
        const std::string commandLineToken = argv[argIndex];
        parseParameter(commandLineToken, minX, "minX");
        parseParameter(commandLineToken, maxX, "maxX");
        parseParameter(commandLineToken, minY, "minY");
        parseParameter(commandLineToken, maxY, "maxY");
        parseParameter(commandLineToken, numberOfPoints, "numberOfPoints");
        parseParameter(commandLineToken, numberOfClusters, "numberOfClusters");
        hasRandomSeed |= parseParameter(commandLineToken, randomSeed, "randomSeed");
    }

    // Display parameters
    std::cout << "Parameters:\n"
              << "\tminX = " << minX << '\n'
              << "\tmaxX = " << maxX << '\n'
              << "\tminY = " << minY << '\n'
              << "\tmaxY = " << maxY << '\n'
              << "\tnumberOfPoints = " << numberOfPoints << '\n'
              << "\tnumberOfClusters = " << numberOfClusters << '\n'
              << "\trandomSeed = " << (hasRandomSeed ? std::to_string(randomSeed) : "none (seed itself is randomized)") << '\n'
              << std::endl;

    // Open file
    std::ofstream file{std::string{DATA_DIRECTORY} + "generated.txt"};
    if (!file) {
        std::cerr << "Error creating file" << std::endl;
        return 1;
    }

    // Write cluster count
    file << numberOfClusters << '\n';

    // Write points
    std::mt19937 gen{};
    for (size_t pointIndex = 0; pointIndex < numberOfPoints; pointIndex++) {
        file << random<size_t>(gen, minX, maxX) << " "
             << random<size_t>(gen, minY, maxY) << '\n';
    }
}
