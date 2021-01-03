#pragma once

#include <cstdint>
#include <limits>

using Coordinate = float;
using Label = uint32_t;
constexpr static Label invalidLabel = std::numeric_limits<Label>::max();

struct Point {
    Coordinate x;
    Coordinate y;
};

struct Centroid {
    const Label clusterLabel;
    Coordinate x;
    Coordinate y;
};
