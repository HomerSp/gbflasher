/*    Copyright (c) 2021-2022, Mathias Tillman <mathias@aqba.se>
 *    All rights reserved
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace utils {
class Hex {
public:
    static std::vector<uint8_t> fromString(const std::string& str, char padByte = 0xFF, bool canFail = false);
    static std::vector<uint8_t> fromString(uint16_t p);

    static std::string toString(const std::vector<uint8_t>& data);
    static std::string toString(uint8_t data);
    static std::string toString(uint16_t data);

    static std::string toString(const std::string& str);

    static std::string toASCII(const std::vector<uint8_t>& data);

    static uint8_t toNibble(char c) { bool error; return toNibble(c, error); }
    static uint8_t toNibble(char c, bool& error) ;
};
}
