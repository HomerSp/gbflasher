#include <cctype>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "flasher.h"
#include "flashfile.h"
#include "logger.h"

int showUsage()
{
    std::cerr << "Usage:\n"
        << "\tgbflasher [options] info\n"
        << "\tgbflasher [options] flash <firmware file>\n"
        << "\tgbflasher [options] reset\n"
        << "\tgbflasher [options] erase\n"
        << "[options]:\n"
        << "-v|--verbose - Verbose logging\n"
        << "-n|--no-reset - Don't reset after flashing\n"
        ;
    return -1;
}

int main(int argc, char** argv)
{
    Logger::start();
    std::atexit([]() {
        Logger::stop();
    });

    bool noReset = false;
    std::string cmd;
    std::vector<std::string> args;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a.empty()) {
            continue;
        }

        if (a[0] == '-') {
            if (a == "-v" || a == "--verbose") {
                Logger::setVerbose(true);
            } else if (a == "-n" || a == "--no-reset") {
                noReset = true;
            } else {
                return showUsage();
            }
        } else {
            if (cmd.empty()) {
                cmd = a;
            } else {
                args.push_back(a);
            }
        }
    }

    if (cmd.empty()) {
        return showUsage();
    }

    Flasher flasher;
    if (!flasher.switchMode(Flasher::MODE_BOOT)) {
        Logger::error("main") << "Could not switch to boot mode";
        return -1;
    }

    Logger::info("main") << "Boot mode detected";
    auto deviceInfo = flasher.deviceInfo();
    if (!deviceInfo) {
        Logger::error("main") << "Failed retrieving device info";
        return -1;
    }

    if (cmd == "info") {
        auto info = deviceInfo->memInfo();
        Logger::info("main") << "Device memory:";
        for (const auto& p: info) {
            Logger::info("main") ("Type 0x%02X, address 0x%08X, length 0x%08X", p.type, p.address, p.length);
        }

        auto appInfo = flasher.appInfo();
        if (!!appInfo) {
            Logger::info("main") << "Device app info:" << "App version" << appInfo->appVersion() << ", Bootloader version" << appInfo->bootloaderVersion();
        }
    } else if (cmd == "flash") {
        if (args.empty()) {
            return showUsage();
        }

        FlashFile flashFile(args.at(0), *deviceInfo);
        if (!flashFile) {
            Logger::error("main") << "Failed parsing firmware file";
            return -1;
        }

        auto flashAppInfo = flashFile.appInfo();
        if (!flashAppInfo) {
            Logger::error("main") << "Missing app info in flash file";
            return -1;
        }

        auto appInfo = flasher.appInfo();
        if (!!appInfo) {
            Logger::info("main") << "Device app info:" << "App version" << appInfo->appVersion() << ", Bootloader version" << appInfo->bootloaderVersion();
        }

        Logger::info("main") << "Firmware app info:" << "App version" << flashAppInfo->appVersion() << ", Bootloader version" << flashAppInfo->bootloaderVersion();

        flasher.erase();

        if (!flasher.flash(flashFile, *deviceInfo)) {
            Logger::error("main") << "Failed flashing";
            return -1;
        }

        if (!flasher.verify(flashFile, *deviceInfo)) {
            Logger::error("main") << "Failed verifying";
            return -1;
        }

        if (!flasher.setAppInfo(flashFile, *deviceInfo, *flashAppInfo)) {
            Logger::error("main") << "Failed flashing app info";
            return -1;
        }

        auto newAppInfo = flasher.appInfo();
        if (!!newAppInfo) {
            Logger::info("main") << "Flashed app info:" << "App version" << newAppInfo->appVersion() << ", Bootloader version" << newAppInfo->bootloaderVersion();
        }

        if (!noReset) {
            flasher.switchMode(Flasher::MODE_REGULAR);
        }
    } else if (cmd == "reset") {
        flasher.switchMode(Flasher::MODE_REGULAR);
    } else if (cmd == "erase") {
        flasher.erase();
    } else {
        return showUsage();
    }

    return 0;
}
