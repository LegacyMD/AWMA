#pragma once

#include <cstdint>
#include <limits>
#include <vector>

using Coordinate = long double;
using Label = uint32_t;
constexpr static Label invalidLabel = std::numeric_limits<Label>::max();

struct Point {
    Label clusterLabel;
    Coordinate x;
    Coordinate y;
};

struct Centroid {
    const Label clusterLabel;
    Coordinate x;
    Coordinate y;
};

constexpr static auto cacheLineSize = 128u;
struct alignas(cacheLineSize) PointBundle {
    constexpr static size_t pointsCount = cacheLineSize / sizeof(Point);
    Point points[pointsCount];
};
static_assert(sizeof(PointBundle) == cacheLineSize, "");

#undef DEFAULT_CTORS_AND_OPERATORS

class PointBundleVector : public std::vector<PointBundle> {
public:
    using BaseClass = std::vector<PointBundle>;
    using BaseClass::BaseClass;

    void setPointsCount(size_t arg) { pointsCount = arg; }
    size_t getPointsCount() const { return pointsCount; }

    size_t getPointsCountInCurrentBundle(size_t bundleIndex) {
        return getPointsCountInCurrentBundle(bundleIndex, size(), pointsCount);
    }

    static size_t getPointsCountInCurrentBundle(size_t bundleIndex, size_t bundleCount, size_t pointsCount) {
        if (bundleIndex == bundleCount - 1) {
            return pointsCount % PointBundle::pointsCount;
        }
        return PointBundle::pointsCount;
    }

private:
    size_t pointsCount;
};
