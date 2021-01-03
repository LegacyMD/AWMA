#pragma once

#include <chrono>

class Timer {
public:
    using Clock = std::chrono::high_resolution_clock;

    void start() { startTime = Clock::now(); }
    void end() { endTime = Clock::now(); }

    template <typename T>
    auto get() { return std::chrono::duration_cast<T>(endTime - startTime); }
    auto getS() { return get<std::chrono::seconds>(); }
    auto getMs() { return get<std::chrono::milliseconds>(); }
    auto getUs() { return get<std::chrono::microseconds>(); }

private:
    Clock::time_point startTime{};
    Clock::time_point endTime{};
};
