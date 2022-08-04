#include <chrono>
#include <map>
#include <thread>

#include "ext/bufferstream.h"
#include "flasher.h"
#include "flashfile.h"
#include "hid.h"
#include "timer.h"
#include "utils/hex.h"

std::shared_ptr<DeviceInfo> Flasher::deviceInfo()
{
    auto bootDev = HID::find(GB_VID, GB_BOOT_PID);
    if (!!bootDev && bootDev->open()) {
        auto data = send(bootDev, CMD_DEVICEINFO);
        if (!data.empty()) {
            return std::make_shared<DeviceInfo>(data);
        }
    }

    return nullptr;
}

std::shared_ptr<AppInfo> Flasher::appInfo()
{
    auto bootDev = HID::find(GB_VID, GB_BOOT_PID);
    if (!!bootDev && bootDev->open()) {
        auto data = readSegmented(bootDev, CMD_GET_APPINFO);
        if (!data.empty()) {
            return std::make_shared<AppInfo>(data);
        }
    }

    return nullptr;
}

bool Flasher::erase()
{
    auto bootDev = HID::find(GB_VID, GB_BOOT_PID);
    if (!!bootDev && bootDev->open()) {
        auto data = send(bootDev, CMD_ERASE);
        if (!data.empty()) {
            return !send(bootDev, CMD_DEVICEINFO).empty();
        }
    }

    return false;
}

bool Flasher::flash(const FlashFile& file, const DeviceInfo& info)
{
    // Only supports APPLICATION
    return flashMemory(file, info, MemoryInfo::APPLICATION);
    /*return MemoryInfo::ALL([&](auto t) {
        if (file.has(t) && !flashMemory(file, info, t)) {
            return false;
        }

        return true;
    });*/
}

bool Flasher::flashMemory(const FlashFile& file, const DeviceInfo& info, MemoryInfo::Type memType)
{
    auto bootDev = HID::find(GB_VID, GB_BOOT_PID);
    if (!!bootDev && bootDev->open()) {
        uint32_t address = 0xFFFFFFFFU;
        for (const auto& p: file.cmds(memType)) {
            const auto& f = p.second;
            if (address == 0xFFFFFFFFU) {
                address = f.addr;
            }

            if (address != f.addr) {
                auto ret = send(bootDev, CMD_WRITE_COMPLETE);
                if (ret.empty()) {
                    printf("Program complete failed!\n");
                    return false;
                }

                address = f.addr;
            }

            auto res = sendResult(bootDev, f.encrypted ? CMD_WRITE_CIPHERED : CMD_WRITE, f.encoded());
            if (res.result < 0) {
                printf("Failed writing address %08X, res %d\n", f.addr, res.result);
                return false;
            }

            address += f.length();
        }

        if (address != 0xFFFFFFFFU) {
            auto ret = send(bootDev, CMD_WRITE_COMPLETE);
            if (ret.empty()) {
                printf("Program complete failed!\n");
                return false;
            }
        }

        return true;
    }

    return false;
}

bool Flasher::setAppInfo(const FlashFile& file, const DeviceInfo& deviceInfo, const AppInfo& appInfo)
{
    auto bootDev = HID::find(GB_VID, GB_BOOT_PID);
    if (!!bootDev && bootDev->open()) {
        auto start = deviceInfo.address(MemoryInfo::APPINFO);
        if (writeSegmented(bootDev, CMD_SET_APPINFO, start, appInfo.data())) {
            auto r = send(bootDev, CMD_SIGN);
            return (!r.empty());
        }
    }

    return false;
}

bool Flasher::verify(const FlashFile& file, const DeviceInfo& info)
{
    return MemoryInfo::ALL([&](auto t) {
        if (file.has(t) && !verifyMemory(file, info, t)) {
            return false;
        }

        return true;
    });
}

bool Flasher::verifyMemory(const FlashFile& file, const DeviceInfo& info, MemoryInfo::Type memType)
{
    auto bootDev = HID::find(GB_VID, GB_BOOT_PID);
    if (!!bootDev && bootDev->open()) {
        for (const auto& p: file.cmds(memType)) {
            const auto& f = p.second;
            auto res = sendResult(bootDev, f.encrypted ? CMD_VERIFY_CIPHERED : CMD_VERIFY, f.encoded());
            if (res.result < 0) {
                printf("Failed verifying address %08X, res %d\n", f.addr, res.result);
                return false;
            }
        }

        return true;
    }

    return false;
}

bool Flasher::switchMode(uint8_t mode)
{
    if (mode == MODE_BOOT) {
        // Check if we're already in boot mode
        auto bootDev = HID::find(GB_VID, GB_BOOT_PID);
        if (!!bootDev) {
            return true;
        }

        auto dev = HID::find(GB_VID, GB_PID, 1);
        if (!!dev && dev->open()) {
            // Don't check the result here as it's expected to fail
            dev->write(REPORT_BOOT, {CMD_BOOTLOADER});
            return waitMode(mode);
        }
    } else if (mode == MODE_REGULAR) {
        auto dev = HID::find(GB_VID, GB_PID, 1);
        if (!!dev) {
            return true;
        }

        auto bootDev = HID::find(GB_VID, GB_BOOT_PID);
        if (!!bootDev && bootDev->open()) {
            bootDev->write({CMD_RESET});
            return waitMode(mode);
        }
    }

    return false;
}

std::vector<uint8_t> Flasher::readData(uint32_t address, uint32_t size)
{
    auto bootDev = HID::find(GB_VID, GB_BOOT_PID);
    if (!!bootDev && bootDev->open()) {
        std::vector<uint8_t> ret;
        for (uint32_t offset = 0; offset < size; ++offset) {
            for (uint8_t i = 0xFFU; i >= 0x00U; --i) {
                ext::BufferStream stream;
                stream.appendDword(address + offset);
                stream.appendDword(0x1U);
                stream.append(i);

                auto res = sendResult(bootDev, CMD_VERIFY, stream.data());
                if (res.result >= 0) {
                    ret.emplace_back(i);
                    break;
                }
            }
        }

        return ret;
    }

    return {};
}

std::vector<uint8_t> Flasher::send(const std::shared_ptr<HID::Device>& dev, uint8_t cmd, std::vector<uint8_t> data)
{
    std::vector<uint8_t> write;
    write.emplace_back(cmd);
    std::copy(data.begin(), data.end(), std::back_inserter(write));
    if (!dev->write(write)) {
        printf("Flasher::send failed writing data\n");
        return {};
    }

    ext::BufferStream stream(dev->read());
    if (stream.eof()) {
        printf("Flasher::send failed reading data\n");
        return {};
    }

    if (stream.readByte() != cmd) {
        return {};
    }

    return stream.readBytes();
}

Flasher::AddressResult Flasher::sendResult(const std::shared_ptr<HID::Device>& dev, uint8_t cmd, std::vector<uint8_t> data)
{
    ext::BufferStream stream(send(dev, cmd, data));
    if (stream.eof()) {
        return {0U, -1};
    }

    return {stream.readDword(), static_cast<int32_t>(stream.readDword())};
}

std::vector<uint8_t> Flasher::readSegmented(const std::shared_ptr<HID::Device>& dev, uint8_t cmd, std::vector<uint8_t> data)
{
    uint16_t readSize = 0, totalSize = 0xFFFFU;
    std::map<uint8_t, std::vector<uint8_t>> blockData;
    while (totalSize == 0xFFFFU || readSize < totalSize) {
        ext::BufferStream stream(send(dev, cmd, data));
        if (stream.eof()) {
            break;
        }

        auto addr = stream.readDword();
        if (addr == 0x00U || addr == 0xFFFFFFFFU) {
            return {};
        }

        if (readSize == 0) {
            totalSize = stream.readWord();
        } else {
            stream.skip(2);
        }

        auto blkSize = stream.readByte();
        auto blkIndex = stream.readByte();

        blockData[blkIndex] = stream.readBytes(blkSize);
        readSize += blkSize;
    }

    // Merge the segmented data into one block
    std::vector<uint8_t> ret;
    for (auto it = blockData.begin(); it != blockData.end(); ++it) {
        std::copy(it->second.begin(), it->second.end(), std::back_inserter(ret));
    }

    return ret;
}

bool Flasher::writeSegmented(const std::shared_ptr<HID::Device>& dev, uint8_t cmd, uint32_t address, const std::vector<uint8_t>& data)
{
    // 54 = 64 (block size) - 10 (header size)
    for (uint8_t i = 0; i < std::ceil(data.size() / 54.0f); ++i) {
        uint32_t start = i * 54;
        uint8_t size = std::min<uint8_t>(54, data.size() - start);
        printf("Flasher::writeSegmented start %d, size %d\n", start, size);

        ext::BufferStream stream;
        stream.appendDword(address);
        stream.appendWord(data.size());
        stream.append(std::max<uint8_t>(size, 54));
        stream.append(i);
        stream.append(data, start, size);
        if (size < 54) {
            stream.fill(54 - size, 0x00U);
        }

        auto r = send(dev, cmd, stream.data());
        if (r.empty()) {
            return false;
        }
    }

    return true;
}

bool Flasher::waitMode(uint8_t mode)
{
    auto timeout = Timer::sec(10);
    do {
        // Wait for the boot device to become available
        auto dev = HID::find(GB_VID, (mode == MODE_BOOT) ? GB_BOOT_PID : GB_PID);
        if (!!dev && dev->open()) {
            return true;
        }

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(10ms);
    } while (!timeout.expired());

    return false;
}
