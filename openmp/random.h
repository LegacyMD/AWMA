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

template <typename T>
auto random() {
    return random<T>(min, max);
}

template <typename T>
auto randomTuple() {
    return std::make_tuple(random<T>(), random<T>());
}

template <typename T>
auto randomVector(size_t count) {
    std::vector<T> result;
    result.reserve(count);
    for (size_t i = 0; i < count; i++) {
        result.push_back(random<T>());
    }
    return result
}

template <typename T>
auto randomVectorOfTuples(size_t count) {
    std::vector<std::tuple<T, T>> result;
    result.reserve(count);
    for (size_t i = 0; i < count; i++) {
        result.push_back(randomTuple<T>());
    }
    return result;
}

} // namespace RandomHelper
