#include "random.h"

namespace RandomHelper {
thread_local std::mt19937 gen{std::random_device{}()};
}
