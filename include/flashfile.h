#pragma once
#include <fstream>
#include <map>
#include <memory>
#include <string>

#include "appinfo.h"
#include "deviceinfo.h"

class FlashFile {
public:
    struct Command {
        uint32_t addr;
        uint8_t cmd;
        std::vector<uint8_t> data;
        bool encrypted;
        uint8_t padding;

        std::vector<uint8_t> encoded() const;
    };

public:
    FlashFile(const std::string& path, const DeviceInfo& deviceInfo)
        : FlashFile(std::ifstream(path), deviceInfo)
    {}

    FlashFile(std::ifstream stream, const DeviceInfo& deviceInfo);

    bool has(MemoryInfo::Type type) const { return mCommands.find(type) != mCommands.end(); }
    const std::map<uint32_t, Command>& cmds(MemoryInfo::Type type) const { return mCommands.at(type); }

    std::shared_ptr<AppInfo> appInfo() const { return mAppInfo; }

    operator bool() const { return mValid; }

private:
    bool mValid = false;
    std::map<MemoryInfo::Type, std::map<uint32_t, Command>> mCommands;
    std::shared_ptr<AppInfo> mAppInfo;
};
