#include "appinfo.h"
#include "ext/bufferstream.h"
#include "utils/hex.h"

AppInfo::AppInfo(const std::vector<uint8_t>& data)
{
    ext::BufferStream stream(data);
    mSignature = stream.readQword();
    mVersion = stream.readDword();
    mLength = stream.readDword();

    auto appVerLen = stream.readByte();
    auto appDescLen = stream.readByte();
    auto bootVerLen = stream.readByte();
    auto bootDescLen = stream.readByte();
    mAppVersion = {appVerLen, stream.readString(appVerLen)};
    mAppDescription = {appDescLen, stream.readString(appDescLen)};
    mBootloaderVersion = {bootVerLen, stream.readString(bootVerLen)};
    mBootloaderDescription = {bootDescLen, stream.readString(bootDescLen)};

    auto numPersistBlocks = stream.readDword();
    for (uint32_t i = 0; i < numPersistBlocks; ++i) {
        auto start = stream.readDword();
        auto len = stream.readDword();
        mPersistentMemory.emplace_back(MemoryInfo::PERSISTENT, start, len);
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
        stream.appendDword(p.start);
        stream.appendDword(p.length);
    }

    stream.pad(mLength, 0x00U);

    return stream.data();
}
