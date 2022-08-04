#include "appinfo.h"
#include "ext/bufferstream.h"
#include "utils/hex.h"

AppInfo::AppInfo(const std::vector<uint8_t>& data)
{
    ext::BufferStream stream(data);
    mSignature = stream.readUInt64();
    mVersion = stream.readUInt32();
    mLength = stream.readUInt32();

    auto appVerLen = stream.readUInt8();
    auto appDescLen = stream.readUInt8();
    auto bootVerLen = stream.readUInt8();
    auto bootDescLen = stream.readUInt8();
    mAppVersion = {appVerLen, stream.readString(appVerLen)};
    mAppDescription = {appDescLen, stream.readString(appDescLen)};
    mBootloaderVersion = {bootVerLen, stream.readString(bootVerLen)};
    mBootloaderDescription = {bootDescLen, stream.readString(bootDescLen)};

    auto numPersistBlocks = stream.readUInt32();
    for (uint32_t i = 0; i < numPersistBlocks; ++i) {
        auto address = stream.readUInt32();
        auto len = stream.readUInt32();
        mPersistentMemory.emplace_back(MemoryInfo::PERSISTENT, address, len);
    }
}

std::vector<uint8_t> AppInfo::data() const
{
    ext::BufferStream stream;
    stream.appendQword(mSignature);
    stream.appendDword(mVersion);
    stream.appendDword(mLength);
    stream.append(mAppVersion.first);
    stream.append(mAppDescription.first);
    stream.append(mBootloaderVersion.first);
    stream.append(mBootloaderDescription.first);

    stream.append(mAppVersion.second, false);
    stream.fill(mAppVersion.first - mAppVersion.second.size(), 0x00U);
    stream.append(mAppDescription.second, false);
    stream.fill(mAppDescription.first - mAppDescription.second.size(), 0x00U);
    stream.append(mBootloaderVersion.second, false);
    stream.fill(mBootloaderVersion.first - mBootloaderVersion.second.size(), 0x00U);
    stream.append(mBootloaderDescription.second, false);
    stream.fill(mBootloaderDescription.first - mBootloaderDescription.second.size(), 0x00U);

    stream.appendDword(mPersistentMemory.size());
    for (const auto& p: mPersistentMemory) {
        stream.appendDword(p.address);
        stream.appendDword(p.length);
    }

    stream.pad(mLength, 0x00U);

    return stream.data();
}
