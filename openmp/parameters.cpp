#include "parameters.h"

#include <iostream>
#include <sstream>
#include <string>

template <typename T>
static void parseParameter(const std::string &commandLineToken, T &parameter, const std::string &key) {
    const auto valueStart = commandLineToken.find('=');
    if (valueStart == std::string::npos) {
        return;
    }

    std::istringstream value{commandLineToken.substr(valueStart + 1)};
    value >> parameter;
}

void Parameters::parseCommandLine(int argc, const char **argv) {
    for (int argIndex = 0; argIndex < argc; argIndex++) {
        const std::string commandLineToken = argv[argIndex];
        parseParameter(commandLineToken, minX, "minX");
        parseParameter(commandLineToken, maxX, "maxX");
        parseParameter(commandLineToken, minY, "minY");
        parseParameter(commandLineToken, maxY, "maxY");
        parseParameter(commandLineToken, numberOfPoints, "numberOfPoints");
        parseParameter(commandLineToken, numberOfClusters, "numberOfClusters");
        parseParameter(commandLineToken, maxIterations, "maxIterations");
    }
}

void Parameters::display() {
    std::cout << "Parameters:\n"
              << "\tminX = " << minX << '\n'
              << "\tmaxX = " << maxX << '\n'
              << "\tminY = " << minY << '\n'
              << "\tmaxY = " << maxY << '\n'
              << "\tnumberOfPoints = " << numberOfPoints << '\n'
              << "\tnumberOfClusters = " << numberOfClusters << '\n'
              << "\tmaxIterations = " << maxIterations << '\n';
}
