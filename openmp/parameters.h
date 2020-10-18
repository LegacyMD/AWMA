#include "definitions.h"

struct Parameters {
    Coordinate minX = 0;
    Coordinate maxX = 50;
    Coordinate minY = 0;
    Coordinate maxY = 50;

    size_t numberOfPoints = 10000;
    size_t numberOfClusters = 20;
    size_t maxIterations = 30;
    int algorithm = 1;
    bool writeCsv = true;
    unsigned int randomSeed = 0;
    bool hasRandomSeed = false;

    void parseCommandLine(int argc, const char** argv);
    void display();
};
