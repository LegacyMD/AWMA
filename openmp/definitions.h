#pragma once

#include <cstdint>
#include <limits>

using Coordinate = int32_t;
using Label = uint32_t;
constexpr static Label invalidLabel = std::numeric_limits<Label>::max();

struct Point {
    Label clusterLabel;
    const Coordinate x;
    const Coordinate y;
};

struct Centroid {
    const Label clusterLabel;
    Coordinate x;
    Coordinate y;
};
