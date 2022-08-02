#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "memoryinfo.h"

class AppInfo {
public:
    AppInfo(const std::vector<uint8_t>& data);

    const std::string& appVersion() const { return mAppVersion.second; }
    const std::string& bootloaderVersion() const { return mBootloaderVersion.second; }

    std::vector<uint8_t> data() const;

private:
    uint64_t mSignature;
    uint32_t mVersion;
    uint32_t mLength;

    std::pair<uint8_t, std::string> mAppVersion;
    std::pair<uint8_t, std::string> mAppDescription;
    std::pair<uint8_t, std::string> mBootloaderVersion;
    std::pair<uint8_t, std::string> mBootloaderDescription;

    std::vector<MemoryInfo> mPersistentMemory;
};
