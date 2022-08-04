#include "ext/timer.h"

using namespace ext;

Timer::Timer(bool enabled)
    : mEnabled(enabled)
    , mStart(0)
    , mTimeout(0)
{
    if (mEnabled) {
        reset();
    }
}

std::chrono::milliseconds Timer::elapsed() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) - mStart;
}

uint64_t Timer::elapsedMs() const
{
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) - mStart;
    return ms.count();
}

uint64_t Timer::elapsedUs() const
{
    std::chrono::microseconds us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()) - mStart;
    return us.count();
}

bool Timer::expired() const
{
    if (mEnabled) {
        return (elapsed() >= mTimeout);
    }

    return false;
}

bool Timer::expired()
{
    if (mEnabled) {
        if (elapsed() >= mTimeout) {
            mWasExpired = true;
            return true;
        }
    }

    return false;
}

Timer& Timer::enable()
{
    if (!mEnabled) {
        mEnabled = true;
        reset();
    }

    return *this;
}

void Timer::reset()
{
    mStart = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
}
