#include <chrono>
#include <print>

template<typename TimeUnit = std::chrono::nanoseconds, typename Func>
TimeUnit average(Func f, int iterations) {
    using namespace std::chrono;

    TimeUnit totalTime{ 0 };
    for (int i = 0; i < iterations; i++) {
        auto start = steady_clock::now();
        f();
        auto time = duration_cast<TimeUnit>((steady_clock::now() - start));
        std::println("Time to render {}: {}", i, time);
        totalTime += time;
    }
    return totalTime / iterations;
}