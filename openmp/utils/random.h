#include <random>
#include <tuple>
#include <vector>

namespace RandomHelper {
extern thread_local std::mt19937 gen;

inline void init(unsigned int seed) {
    gen = std::mt19937{seed};
}

template <typename T>
auto random(T min, T max) {
    return std::uniform_int_distribution<T>{min, max}(gen);
}

} // namespace RandomHelper
