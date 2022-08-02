#include "memoryinfo.h"

bool MemoryInfo::ALL(std::function<bool(MemoryInfo::Type)> cb)
{
    for (auto t = MemoryInfo::NONE; t <= MemoryInfo::PERSISTENT; ++t) {
        if (!cb(t)) {
            return false;
        }
    }

    return true;
}

MemoryInfo::Type& operator++(MemoryInfo::Type& t)
{
    switch (t) {
        case MemoryInfo::NONE:
            return t = MemoryInfo::APPLICATION;
        case MemoryInfo::APPLICATION:
            return t = MemoryInfo::E2PROM;
        case MemoryInfo::E2PROM:
            return t = MemoryInfo::CONFIG;
        case MemoryInfo::CONFIG:
            return t = MemoryInfo::USERID;
        case MemoryInfo::USERID:
            return t = MemoryInfo::DEVICEID;
        case MemoryInfo::DEVICEID:
            return t = MemoryInfo::MCUBOOT;
        case MemoryInfo::MCUBOOT:
            return t = MemoryInfo::BOOTLOADER;
        case MemoryInfo::BOOTLOADER:
            return t = MemoryInfo::ALTCONFIG;
        case MemoryInfo::ALTCONFIG:
            return t = MemoryInfo::APPINFO;
        case MemoryInfo::APPINFO:
            return t = MemoryInfo::PERSISTENT;
        default:
            return t = MemoryInfo::END;
    }
}

MemoryInfo::Type operator++(MemoryInfo::Type& t, int)
{
    MemoryInfo::Type tmp(t);
    ++t;
    return tmp;
}
