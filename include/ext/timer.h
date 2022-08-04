#pragma once

#include <chrono>
#include <cstdint>

namespace ext {
class Timer {
public:
    static Timer ms(uint64_t v) { return Timer().setMs(v); }
    static Timer sec(uint64_t v) { return Timer().setSec(v); }
    static Timer min(uint64_t v) { return Timer().setMin(v); }
    static Timer hour(uint64_t v) { return Timer().setHour(v); }

public:
    Timer(bool enabled = true);

    std::chrono::milliseconds elapsed() const;

    uint64_t elapsedMs() const;
    uint64_t elapsedUs() const;
    template<typename T>
    T elapsed() const { return static_cast<T>(elapsed().count()); }

    bool enabled() const { return mEnabled; }

    bool expired() const;
    bool expired();
    bool wasExpired() const { return mWasExpired; }

    Timer& enable();
    Timer& disable() { mEnabled = false; return *this; }

    void reset();

    Timer& setMs(uint64_t v) { mTimeout = std::chrono::milliseconds(v); return *this; }
    Timer& setSec(uint64_t v) { return setMs(v * 1000); }
    Timer& setMin(uint64_t v) { return setMs(v * 1000 * 60); }
    Timer& setHour(uint64_t v) { return setMs(v * 1000 * 60 * 60); }

private:
    bool mEnabled = true;
    std::chrono::milliseconds mStart;
    std::chrono::milliseconds mTimeout;
    bool mWasExpired = false;
};
}
