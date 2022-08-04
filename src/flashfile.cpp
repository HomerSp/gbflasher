#include "ext/bufferstream.h"
#include "flashfile.h"
#include "logger.h"
#include "utils/hex.h"

std::vector<uint8_t> FlashFile::Command::encoded() const
{
    ext::BufferStream stream;
    stream.appendDword(address);
    stream.appendDword(data.size());
    stream.append(data);
    return stream.data();
}

FlashFile::FlashFile(std::ifstream stream, const DeviceInfo& deviceInfo)
{
    uint32_t baseAddr = 0;
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line[0] != ':') {
            return;
        }

        auto data = utils::Hex::fromString(line.substr(1));
        if (!verifyChecksum(data)) {
            Logger::error<FlashFile>("FlashFile") << "Found invalid checksum at line" << line;
            return;
        }

        ext::BufferStream lineStream(data);
        if (lineStream.eof()) {
            return;
        }

        FlashFile::Command cmd({0, 0, {}, false, 0});
        auto len = lineStream.readUInt8();
        cmd.address = lineStream.bitRead(16);   // Little endian
        cmd.cmd = lineStream.readUInt8();
        cmd.data = lineStream.readBytes(len);
        auto chk = lineStream.readUInt8();

        switch (cmd.cmd) {
            case 0x00U: {
                break;
            }
            case 0x04U: {
                ext::BufferStream addrData(cmd.data);
                baseAddr = addrData.bitRead(16);
                baseAddr <<= 16;
                continue;
            }
            default: {
                if ((cmd.cmd & 0xC0U) == 0xC0U) {
                    cmd.encrypted = true;
                    cmd.padding = (cmd.cmd & 0x0FU);
                    cmd.cmd &= 0xF0U;
                    break;
                }

                return;
            }
        }

        cmd.address |= baseAddr;
        cmd.address &= 0x1FFFFFFF;

        auto memType = deviceInfo.memoryType(cmd.address, cmd.data.size());
        Logger::verbose<FlashFile>("FlashFile")("cmd %02X, addr %08X, len %08X, type %d", cmd.cmd, cmd.address, cmd.data.size(), memType);
        if (memType == MemoryInfo::NONE) {
            return;
        }

        mCommands[memType].insert({cmd.address, cmd});
    }

    if (mCommands.find(MemoryInfo::APPINFO) != mCommands.end()) {
        std::vector<uint8_t> appdata;
        for (const auto& p: mCommands.at(MemoryInfo::APPINFO)) {
            std::copy(p.second.data.begin(), p.second.data.end(), std::back_inserter(appdata));
        }

        if (!appdata.empty()) {
            mAppInfo = std::make_shared<AppInfo>(appdata);
        }

        // App info is set using a separate command
        mCommands.erase(MemoryInfo::APPINFO);
    }

    mValid = !!mAppInfo && !mCommands.empty();
}

bool FlashFile::verifyChecksum(const std::vector<uint8_t>& data) const
{
    if (data.size() < 2) {
        return false;
    }

    auto found = data.at(data.size() - 1);
    uint8_t calculated = 0;
    for (uint32_t i = 0; i < data.size() - 1; ++i) {
        calculated += data.at(i);
    }

    calculated = (~calculated + 1) & 0xFFU;
    if (calculated != found) {
        Logger::error<FlashFile>("verifyChecksum") ("failed (calculated %02X, vs expected %02X)", calculated, found);
        return false;
    }

    return true;
}
