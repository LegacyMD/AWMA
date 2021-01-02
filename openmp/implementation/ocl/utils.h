#pragma once

#include <fstream>
#include <sstream>
#include <vector>

struct OclUtils {
    template <typename T>
    static size_t getTotalVectorSizeInBytes(const std::vector<T> &vector) {
        return sizeof(T) * vector.size();
    }

    static std::string readFile(const std::string &fileName) {
        std::ifstream file{fileName};
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
};
