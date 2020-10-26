#include "implementation/implementation.h"
#include "utils/parameters.h"

#include <iostream>
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

void Parameters::parseCommandLine(int argc, const char **argv) {
    for (int argIndex = 0; argIndex < argc; argIndex++) {
        const std::string commandLineToken = argv[argIndex];
        parseParameter(commandLineToken, maxIterations, "maxIterations");
        parseParameter(commandLineToken, writeCsv, "writeCsv");
        parseParameter(commandLineToken, verbose, "verbose");
        parseParameter(commandLineToken, implementationIndex, "implementation");
        parseParameter(commandLineToken, inputFileName, "inputFileName");
        hasRandomSeed |= parseParameter(commandLineToken, randomSeed, "randomSeed");
    }
}

void Parameters::display() {
    std::cout << "Parameters:\n"
              << "\tmaxIterations = " << maxIterations << '\n'
              << "\timplementation = " << implementationIndex << " (" << getImplementationString(implementationIndex) << ")\n"
              << "\tinputFileName = " << inputFileName << '\n'
              << "\trandomSeed = " << (hasRandomSeed ? std::to_string(randomSeed) : "none (seed itself is randomized)") << '\n'
              << std::endl;
}
