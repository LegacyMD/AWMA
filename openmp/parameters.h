#include "definitions.h"

struct Parameters {
    Coordinate minX = 0;
    Coordinate maxX = 50;
    Coordinate minY = 0;
    Coordinate maxY = 50;

    size_t numberOfPoints = 3000;
    size_t numberOfClusters = 20;
    size_t maxIterations = 30;

    void parseCommandLine(int argc, const char** argv);
    void display();
};
