#pragma once
#include <map>
#include <memory>
#include <vector>

#include "appinfo.h"
#include "deviceinfo.h"
#include "hid.h"

class Flasher {
public:
    static constexpr auto Tag = "Flasher";

public:
    static constexpr uint8_t MODE_REGULAR = 0;
    static constexpr uint8_t MODE_BOOT = 1;

private:
    struct AddressResult {
        uint32_t address;
        int32_t result;
    };

public:
    Flasher() = default;

    std::shared_ptr<DeviceInfo> deviceInfo();
    std::shared_ptr<AppInfo> appInfo();

    bool erase();
    bool flash(const FlashFile& file, const DeviceInfo& info);
    bool setAppInfo(const FlashFile& file, const DeviceInfo& deviceInfo, const AppInfo& appInfo);
    bool verify(const FlashFile& file, const DeviceInfo& info);

    bool switchMode(uint8_t mode);

    std::vector<uint8_t> readData(uint32_t address, uint32_t size);

private:
    bool flashMemory(const FlashFile& file, const DeviceInfo& info, MemoryInfo::Type memType);
    bool verifyMemory(const FlashFile& file, const DeviceInfo& info, MemoryInfo::Type memType);

    std::vector<uint8_t> send(const std::shared_ptr<HID::Device>& dev, uint8_t cmd, std::vector<uint8_t> data = {});
    AddressResult sendResult(const std::shared_ptr<HID::Device>& dev, uint8_t cmd, std::vector<uint8_t> data = {});

    std::vector<uint8_t> readSegmented(const std::shared_ptr<HID::Device>& dev, uint8_t cmd, std::vector<uint8_t> data = {});

    bool writeSegmented(const std::shared_ptr<HID::Device>& dev, uint8_t cmd, uint32_t address, const std::vector<uint8_t>& data);

    bool waitMode(uint8_t mode);

private:
    static constexpr uint16_t GB_VID = 0x0782U;
    static constexpr uint16_t GB_PID = 0x001BU;
    static constexpr uint16_t GB_BOOT_PID = 0xFFE3U;

private:
    static constexpr uint8_t REPORT_NORMAL = 0x00U;

    static constexpr uint8_t CMD_DEVICEINFO = 0x02U;
    static constexpr uint8_t CMD_CONFIG_UNLOCK = 0x03U;
    static constexpr uint8_t CMD_ERASE = 0x04U;
    static constexpr uint8_t CMD_WRITE = 0x05U;
    static constexpr uint8_t CMD_WRITE_COMPLETE = 0x06U;
    static constexpr uint8_t CMD_RESET = 0x08U;
    static constexpr uint8_t CMD_SIGN = 0x09U;
    static constexpr uint8_t CMD_WRITE_CIPHERED = 0x0EU;
    static constexpr uint8_t CMD_GET_APPINFO = 0x0FU;
    static constexpr uint8_t CMD_SET_APPINFO = 0x10U;
    static constexpr uint8_t CMD_VERIFY = 0x11U;
    static constexpr uint8_t CMD_VERIFY_CIPHERED = 0x12U;
    static constexpr uint8_t CMD_BOOTLOADER = 0xB3U;

    static constexpr uint8_t REPORT_BOOT = 0x03U;
};
