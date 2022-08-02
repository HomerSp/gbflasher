#include "ext/bufferstream.h"
#include "flashfile.h"
#include "utils/hex.h"

std::vector<uint8_t> FlashFile::Command::encoded() const
{
    ext::BufferStream stream;
    stream.appendDword(addr);
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

        ext::BufferStream lineStream(utils::Hex::fromString(line.substr(1)));
        if (lineStream.eof()) {
            return;
        }

        FlashFile::Command cmd({0, 0, {}, false, 0});
        auto len = lineStream.readByte();
        cmd.addr = lineStream.bitRead(16);
        cmd.cmd = lineStream.readByte();
        cmd.data = lineStream.readBytes(len);
        auto chk = lineStream.readByte();

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

        cmd.addr |= baseAddr;
        cmd.addr &= 0x1FFFFFFF;

        auto memType = deviceInfo.memoryType(cmd.addr, cmd.data.size());
        //printf("FlashFile cmd %02X, addr %08X, len %08X, type %d\n", cmd.cmd, cmd.addr, cmd.data.size(), memType);
        if (memType == MemoryInfo::NONE) {
            return;
        }

        mCommands[memType].insert({cmd.addr, cmd});
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
