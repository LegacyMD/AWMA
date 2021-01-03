#include "definitions.h"

struct Parameters {
    size_t maxIterations = 30;
    bool writeCsv = true;
    bool verbose = true;
    int implementationIndex = 1;
    std::string inputFileName{};
    unsigned int randomSeed = 0;
    bool hasRandomSeed = false;

    void parseCommandLine(int argc, const char **argv);
    void display();
};
