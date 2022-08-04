#include <cstdio>

#include "hid.h"
#include "logger.h"
#include "utils/hex.h"

HID::Device::Device(std::string path)
    : mPath(std::move(path))
{}

HID::Device::~Device()
{
    close();
}

bool HID::Device::open()
{
    Logger::verbose<HID::Device>("open") << mPath;
    if (mDevice != nullptr) {
        Logger::warning<HID::Device>("open") << "Already opened";
        return false;
    }

    mDevice = hid_open_path(mPath.c_str());
    return mDevice != nullptr;
}

void HID::Device::close()
{
    if (mDevice != nullptr) {
        hid_close(mDevice);
        mDevice = nullptr;
    }
}

std::vector<uint8_t> HID::Device::read()
{
    if (mDevice == nullptr) {
        return {};
    }

    std::vector<uint8_t> ret(65, 0x0U);
    auto read = hid_read_timeout(mDevice, &ret.at(0), ret.size(), 10000);
    if (read <= 0) {
        Logger::error<HID::Device>("read") << "error" << read;
        return {};
    }

    ret.resize(read);
    Logger::verbose<HID::Device>("read") << "result" << utils::Hex::toString(ret);
    return ret;
}

bool HID::Device::write(uint8_t report, const std::vector<uint8_t>& data)
{
    if (mDevice == nullptr) {
        Logger::error<HID::Device>() << "No device opened";
        return false;
    }

    if (data.size() > 64) {
        Logger::error<HID::Device>() << "Data size" << data.size() << "exceeds maximum packet size";
        return false;
    }

    std::vector<uint8_t> d(65, 0x00U);
    d[0] = report;
    if (!data.empty()) {
        std::copy(data.begin(), data.end(), d.begin() + 1);
    }

    Logger::verbose<HID>("write") << utils::Hex::toString(d);
    return hid_write(mDevice, d.data(), d.size()) == d.size();
}

std::shared_ptr<HID::Device> HID::find(uint16_t vid, uint16_t pid, int interfaceNum)
{
    auto* devs = hid_enumerate(vid, pid);
    auto* cur_dev = devs;

    std::string path;
    while (cur_dev != nullptr) {
        if (cur_dev->vendor_id == vid && cur_dev->product_id == pid && (interfaceNum == -1 || cur_dev->interface_number == interfaceNum)) {
            Logger::verbose<HID>("find") ("Found %04X:%04X, interface %d path %s", cur_dev->vendor_id, cur_dev->product_id, cur_dev->interface_number, cur_dev->path);
            if (!path.empty()) {
                Logger::warning<HID::Device>("find") << "Multiple devices match, using first";
                break;
            }

            path = cur_dev->path;
        }

        cur_dev = cur_dev->next;
    }

    if (devs != nullptr) {
        hid_free_enumeration(devs);
    }

    if (!path.empty()) {
        return std::make_shared<Device>(path);
    }

    return nullptr;
}
