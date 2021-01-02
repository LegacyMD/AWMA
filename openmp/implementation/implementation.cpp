#include "implementation/implementation.h"
#include "implementation/omp/updateOmpAtomics.h"
#include "implementation/omp/updateOmpReduction.h"

Coordinate distance(const Point &point, const Centroid &centroid) {
    const auto x = point.x - centroid.x;
    const auto y = point.y - centroid.y;
    return x * x + y * y; // no sqrt, because it's only for comparisons
}

std::unique_ptr<Implementation> getImplementation(size_t index) {
    switch (index) {
    case 1:
        return std::unique_ptr<Implementation>(new OmpAtomicsImplementation());
    case 2:
        return std::unique_ptr<Implementation>(new OmpReductionImplementation());
    default:
        return nullptr;
    }
}

std::string getImplementationString(size_t index) {
    switch (index) {
    case 1:
        return "ompAtomics";
    case 2:
        return "ompReduction";
    default:
        return "unknown";
    }
}
