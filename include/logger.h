#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <thread>

struct Logger;
class LoggerStream;

struct LoggerSettings {
private:
    friend struct Logger;
    friend class LoggerStream;

    static bool sStdOut;
    static bool sTime;
    static bool sVerbose;
    static std::shared_ptr<std::ofstream> sOutput;
    static bool sThreading;
};

#ifndef DEBUG
class DummyStream {
public:
    ~DummyStream() {}

    template <class T>
    DummyStream& operator<<(const T& x)
    {
        return *this;
    }

    template<typename ... T>
    DummyStream& operator()( const std::string& format, T ... args)
    {
        return *this;
    }
};
#endif

class LoggerStream {
public:
    LoggerStream(const LoggerStream& o);
    ~LoggerStream();

    template <class T>
    LoggerStream& operator<<(const T& x)
    {
        if (shouldSkip()) {
            return *this;
        }

        if (mStream.tellp() > 0) {
            mStream << " ";
        }

        mStream << x;
        return *this;
    }

    template<typename ... T>
    LoggerStream& operator()( const std::string& format, T ... args)
    {
        if (shouldSkip()) {
            return *this;
        }

        size_t size = ::snprintf(nullptr, 0, format.c_str(), args ...) + 1;
        auto buf = std::make_unique<char[]>(size);
        snprintf(buf.get(), size, format.c_str(), args ...);
        return (*this) << std::string(buf.get(), buf.get() + size - 1);
    }

protected:
    LoggerStream(std::ostream* s, char type, std::string tag, std::string func);

    inline bool shouldSkip() { return (mSkip = mSkip || skipStdout() || skipVerbose()); }

private:
    friend class Logger;

    inline bool skipStdout() const {
        return (!LoggerSettings::sOutput && mOut == &std::cout && !LoggerSettings::sStdOut);
    }

    inline bool skipVerbose() {
        return (mType == 'V' && !LoggerSettings::sVerbose);
    }

    bool mSkip = false;
    std::ostream* mOut;
    char mType;
    std::string mTag;
    std::string mFunc;
    std::ostringstream mStream;
    time_t mTime;
};

struct Logger {
public:
    static void stop();
    static void start();

private:
    static void process();

public:
    static inline void setFile(const std::string& f) {
        if (!f.empty()) {
            LoggerSettings::sOutput = std::make_shared<std::ofstream>(f, std::ofstream::out | std::ofstream::app);
        }
    }
    static inline void setStdout(bool b) { LoggerSettings::sStdOut = b; }
    static inline void setTime(bool b) { LoggerSettings::sTime = b; }
    static inline void setVerbose(bool b) { LoggerSettings::sVerbose = b; }
    static inline void setThreading(bool b) { LoggerSettings::sThreading = b; }

    static inline bool isStdout() { return LoggerSettings::sStdOut; }
    static inline bool isVerbose() { return LoggerSettings::sVerbose; }

protected:
    friend class LoggerStream;

    static std::atomic<bool> sRunning;
    static std::unique_ptr<std::thread> sThread;
    static std::shared_timed_mutex sMutex;
    static std::deque<std::pair<std::ostream*, std::string>> sStream;
    static std::mutex sWaiterMutex;
    static std::condition_variable sWaiter;

public:
    template<class T>
    static LoggerStream critical(std::string func = "") { return critical(T::Tag, std::move(func)); }
    static LoggerStream critical(std::string tag, std::string func = "") { return LoggerStream(&std::cerr, 'C', std::move(tag), std::move(func)); }

    template<class T>
    static LoggerStream error(std::string func = "") { return error(T::Tag, std::move(func)); }
    static LoggerStream error(std::string tag, std::string func = "") { return LoggerStream(&std::cerr, 'E', std::move(tag), std::move(func)); }

    template<class T>
    static LoggerStream warning(std::string func = "") { return warning(T::Tag, std::move(func)); }
    static LoggerStream warning(std::string tag, std::string func = "") { return LoggerStream(&std::cerr, 'W', std::move(tag), std::move(func)); }

    template<class T>
    static LoggerStream info(std::string func = "") { return info(T::Tag, std::move(func)); }
    static LoggerStream info(std::string tag, std::string func = "") { return LoggerStream(&std::cout, 'I', std::move(tag), std::move(func)); }

#ifdef DEBUG
    template<class T>
    static LoggerStream debug(std::string func = "") { return debug(T::Tag, std::move(func)); }
    static LoggerStream debug(std::string tag, std::string func = "") { return LoggerStream(&std::cout, 'D', std::move(tag), std::move(func)); }

    template<class T>
    static LoggerStream verbose(std::string func = "") { return verbose(T::Tag, std::move(func)); }
    static LoggerStream verbose(std::string tag, std::string func = "") { return LoggerStream(&std::cout, 'V', std::move(tag), std::move(func)); }
#else
    template<class T>
    static DummyStream debug(std::string func = "") { return DummyStream(); }
    static DummyStream debug(std::string tag, std::string func = "") { return DummyStream(); }

    template<class T>
    static DummyStream verbose(std::string func = "") { return DummyStream(); }
    static DummyStream verbose(std::string tag, std::string func = "") { return DummyStream(); }
#endif
};
