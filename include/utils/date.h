#pragma once
#include <iomanip>

namespace utils {
class Date {
public:
    static std::string format(const std::tm* t, const char* f) {
        std::stringstream s;
        s << std::put_time(t, f);
        return s.str();
    }

    static std::string format(time_t val, const char* f) {
        return format(std::localtime(&val), f);
    }

    static std::string formatAnsi(time_t val)
    {
        return format(std::localtime(&val), "%Y-%m-%d %H:%M:%S");
    }

    static std::string formatHttp(time_t val)
    {
        return format(std::gmtime(&val), "%a, %d %b %Y %H:%M:%S %Z");
    }


    static std::string formatC(time_t val)
    {
        return format(std::localtime(&val), "%Y-%m-%dT%H:%M:%SZ");
    }
};
}