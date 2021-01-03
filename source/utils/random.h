#include <random>
#include <tuple>
#include <type_traits>
#include <vector>

namespace RandomHelper {
extern thread_local std::mt19937 gen;

inline void init(unsigned int seed) {
    gen = std::mt19937{seed};
}

template <class T, typename std::enable_if_t<std::is_integral<T>::value, T> * = nullptr>
auto random(T min, T max) {
    return std::uniform_int_distribution<T>{min, max}(gen);
}

template <class T, typename std::enable_if_t<!std::is_integral<T>::value, T> * = nullptr>
auto random(T min, T max) {
    return std::uniform_real_distribution<T>{min, max}(gen);
}

} // namespace RandomHelper
