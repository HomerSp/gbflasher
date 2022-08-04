#include <iomanip>

#include "logger.h"
#include "utils/date.h"

std::atomic<bool> Logger::sRunning = {false};
std::unique_ptr<std::thread> Logger::sThread;
std::shared_timed_mutex Logger::sMutex;
std::deque<std::pair<std::ostream*, std::string>> Logger::sStream; // NOLINT

std::mutex Logger::sWaiterMutex;
std::condition_variable Logger::sWaiter;

bool LoggerSettings::sStdOut = true;
bool LoggerSettings::sTime = true;
bool LoggerSettings::sVerbose = false;
std::shared_ptr<std::ofstream> LoggerSettings::sOutput;
bool LoggerSettings::sThreading = true;

LoggerStream::LoggerStream(const LoggerStream& o)
    : mSkip(o.mSkip)
    , mOut(o.mOut)
    , mType(o.mType)
    , mTag(o.mTag)
    , mFunc(o.mFunc)
    , mTime(o.mTime)
{
    mStream << o.mStream.rdbuf();
}

LoggerStream::LoggerStream(std::ostream* s, char type, std::string tag, std::string func)
    : mOut(s)
    , mType(type)
    , mTag(std::move(tag))
#ifdef DEBUG
    , mFunc(std::move(func))
#endif
    , mTime(time(nullptr))
{
}

LoggerStream::~LoggerStream()
{
    if (shouldSkip()) {
        return;
    }

    std::stringstream stream;
    if (LoggerSettings::sTime) {
        stream << "[" << utils::Date::formatAnsi(mTime) << "] ";
    }

    stream << mType;
    if (!mTag.empty()) {
        stream << "/" << mTag;
    }

    if (!mFunc.empty()) {
        if (!mTag.empty()) {
            stream << "::";
        }

        stream << mFunc;
    }

    auto str = mStream.str();
    if (!str.empty()) {
        stream << ": " << str;
    }

    stream << std::endl;

    auto* out = (!!LoggerSettings::sOutput) ? LoggerSettings::sOutput.get() : mOut;
    if (LoggerSettings::sThreading) {
        std::unique_lock<std::shared_timed_mutex> lock(Logger::sMutex);
        Logger::sStream.emplace_back(out, stream.str());
    } else {
        (*out) << stream.str() << std::flush;
    }

    std::unique_lock<std::mutex> lock(Logger::sWaiterMutex);
    Logger::sWaiter.notify_all();
}

void Logger::stop()
{
    if (!LoggerSettings::sThreading || !sThread) {
        return;
    }

    {
        std::unique_lock<std::mutex> cvLock(sWaiterMutex);
        sRunning = false;
    }

    sWaiter.notify_all();
    sThread->join();
}

void Logger::start()
{
    if (!LoggerSettings::sThreading) {
        return;
    }

    if (!sThread) {
        {
            std::unique_lock<std::mutex> cvLock(sWaiterMutex);
            sRunning = true;
        }

        sThread = std::make_unique<std::thread>(&Logger::process);
    }
}

void Logger::process()
{
    while (true) {
        {
            std::unique_lock<std::mutex> cvLock(sWaiterMutex);
            sWaiter.wait(cvLock, [&] {
                return !sRunning || !sStream.empty();
            });

            // Spurious wakeup can cause this
            if (sStream.empty()) {
                if (!sRunning) {
                    break;
                }

                continue;
            }
        }

        std::pair<std::ostream*, std::string> front;
        {
            std::unique_lock<std::shared_timed_mutex> lock(sMutex);
            front = sStream.front();
            sStream.pop_front();
        }

        (*front.first) << front.second << std::flush;
    }
}
