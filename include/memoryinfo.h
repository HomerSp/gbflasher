#pragma once
#include <cstdint>
#include <functional>

struct MemoryInfo {
    enum Type : uint8_t {
        NONE,
        APPLICATION,
        E2PROM,
        CONFIG,
        USERID,
        DEVICEID,
        MCUBOOT,
        BOOTLOADER,
        ALTCONFIG,
        APPINFO,
        PERSISTENT,
        END = 0xFFU,
    };

    static bool ALL(std::function<bool(Type)> cb);

    Type type;
    uint32_t address;
    uint32_t length;

    MemoryInfo(Type t, uint32_t a, uint32_t l) : type(t), address(a), length(l) {}
};

MemoryInfo::Type& operator++(MemoryInfo::Type& t);
MemoryInfo::Type operator++(MemoryInfo::Type& t, int);
