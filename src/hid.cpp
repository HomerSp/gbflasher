#include <cstdio>

#include "hid.h"
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
    printf("Device::open %s\n", mPath.c_str());
    if (mDevice != nullptr) {
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
        printf("hid_read_timeout %d\n", read);
        return {};
    }

    ret.resize(read);
    //printf("HID::Device::read %s\n", utils::Hex::toString(ret).c_str());
    return ret;
}

bool HID::Device::write(uint8_t report, const std::vector<uint8_t>& data)
{
    if (mDevice == nullptr) {
        printf("Invalid device\n");
        return false;
    }

    if (data.size() > 64) {
        printf("Data size (%d) too big - %s\n", data.size(), utils::Hex::toString(data).c_str());
        return false;
    }

    std::vector<uint8_t> d(65, 0x00U);
    d[0] = report;
    if (!data.empty()) {
        std::copy(data.begin(), data.end(), d.begin() + 1);
    }

    //printf("HID::Device::write %s\n", utils::Hex::toString(d).c_str());
    return hid_write(mDevice, d.data(), d.size()) == d.size();
}

std::shared_ptr<HID::Device> HID::find(uint16_t vid, uint16_t pid, int interfaceNum)
{
    auto* devs = hid_enumerate(vid, pid);
    auto* cur_dev = devs;

    std::string path;
    while (cur_dev != nullptr) {
        printf("HID::find %04X %04X %d %s\n", cur_dev->vendor_id, cur_dev->product_id, cur_dev->interface_number, cur_dev->path);
        if (cur_dev->vendor_id == vid && cur_dev->product_id == pid && (interfaceNum == -1 || cur_dev->interface_number == interfaceNum)) {
            path = cur_dev->path;
            break;
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
