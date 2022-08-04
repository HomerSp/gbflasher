#pragma once
#include <cstdint>
#include <hidapi.h>
#include <memory>
#include <string>
#include <vector>

class HID {
public:
    static constexpr auto Tag = "HID";
public:
    class Device {
    public:
        static constexpr auto Tag = "HID::Device";
    public:
        Device(std::string path);
        ~Device();

        bool open();
        void close();

        std::vector<uint8_t> read();
        bool write(const std::vector<uint8_t>& data) { return write(0x00U, data); }
        bool write(uint8_t report, const std::vector<uint8_t>& data);

    private:
        std::string mPath;
        hid_device* mDevice = nullptr;
    };

public:
    static std::shared_ptr<HID::Device> find(uint16_t vid, uint16_t pid, int interfaceNum = -1);
};
