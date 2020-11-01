#pragma once

#include "definitions.h"

#include <string>
#include <vector>

// Returns true if some centroid changed
using Implementation = bool (*)(const std::vector<Point> &points, std::vector<Label> &pointLabels, std::vector<Centroid> &centroids, size_t numberOfPoints, size_t numberOfClusters);

Coordinate distance(const Point &point, const Centroid &centroid);

Implementation getImplementation(size_t index);

std::string getImplementationString(size_t index);
