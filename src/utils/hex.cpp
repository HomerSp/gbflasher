#include <iostream>

#include "utils/hex.h"

std::vector<uint8_t> utils::Hex::fromString(const std::string& str, char padByte, bool canFail)
{
    std::vector<uint8_t> ret;
    uint8_t last = 0;
    bool found = false;
    for (char c: str) {
        bool error;
        uint8_t r = toNibble(c, error);
        if (error) {
            if (canFail) {
                return {};
            }

            continue;
        }

        if (found) {
            ret.push_back(static_cast<uint8_t>(last << 4U) | r);
            last = 0;
            found = false;
        } else {
            last = r;
            found = true;
        }
    }

    // Pad it to fill a full octet
    if (found) {
        ret.push_back(static_cast<uint8_t>(last << 4U) | (static_cast<uint8_t>(padByte) & 0x0FU));
    }

    return ret;
}

std::vector<uint8_t> utils::Hex::fromString(uint16_t p)
{
    std::vector<uint8_t> ret;
    ret.push_back(static_cast<uint8_t>((p >> 8U)) & 0xFFU);
    ret.push_back(p & 0xFFU);
    return ret;
}

std::string utils::Hex::toString(const std::vector<uint8_t>& data)
{
    std::string ret;
    for (uint8_t c: data) {
        for (uint8_t i = 0; i < 2; ++i) {
            uint8_t c1 = static_cast<uint8_t>(c >> static_cast<uint8_t>((1 - i) * 4U)) & 0x0FU;
            if (c1 >= 0x0 && c1 <= 0x9) {
                ret += static_cast<uint8_t>('0' + c1);
            } else {
                ret += static_cast<uint8_t>('A' + (c1 - 0xa));
            }
        }
    }

    return ret;
}

std::string utils::Hex::toString(uint8_t data)
{
    return utils::Hex::toString(std::vector<uint8_t>({data}));
}

std::string utils::Hex::toString(uint16_t data)
{
    return utils::Hex::toString(std::vector<uint8_t>({static_cast<uint8_t>(static_cast<uint8_t>(data >> 8U) & 0xFFU), static_cast<uint8_t>(data & 0xFFU)}));
}

std::string utils::Hex::toString(const std::string& data)
{
    return toString(std::vector<uint8_t>(data.begin(), data.end()));
}

std::string utils::Hex::toASCII(const std::vector<uint8_t>& data)
{
    return toString(toString(data));
}

uint8_t utils::Hex::toNibble(char c, bool& error)
{
    error = false;

    uint8_t r = 0U;
    if (c >= 'A' && c <= 'F') {
        r = (0x0AU + (c - 'A')) & 0x0FU;
    } else if (c >= 'a' && c <= 'f') {
        r = (0x0AU + (c - 'a')) & 0x0FU;
    } else if (c >= '0' && c <= '9') {
        r = (0x00U + (c - '0')) & 0x0FU;
    } else {
        error = true;
    }

    return r;
}