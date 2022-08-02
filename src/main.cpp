#include <cctype>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "flasher.h"
#include "flashfile.h"

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "Usage: gbflasher <firmware file>\n";
        return -1;
    }

    Flasher flasher;
    if (!flasher.switchMode(Flasher::MODE_BOOT)) {
        std::cerr << "Could not switch to boot mode\n";
        return -1;
    }

    std::cout << "Boot device found!\n";

    auto deviceInfo = flasher.deviceInfo();
    if (!deviceInfo) {
        std::cerr << "Failed retrieving device info\n";
        return -1;
    }

    FlashFile flashFile(argv[1], *deviceInfo);
    if (!flashFile) {
        std::cerr << "Failed parsing firmware file\n";
        return -1;
    }

    auto appInfo = flasher.appInfo();
    if (!!appInfo) {
        std::cout << "Device App version " << appInfo->appVersion() << "\n"
            << "Bootloader version " << appInfo->bootloaderVersion() << "\n";
    }

    auto flashAppInfo = flashFile.appInfo();
    if (!flashAppInfo) {
        std::cerr << "Missing app info in flash file\n";
        return -1;
    }

    std::cout << "Firmware App version " << flashAppInfo->appVersion() << "\n"
        << "Bootloader version " << flashAppInfo->bootloaderVersion() << "\n";

    flasher.erase();

    if (!flasher.flash(flashFile, *deviceInfo)) {
        std::cerr << "Failed flashing\n";
        return -1;
    }

    if (!flasher.verify(flashFile, *deviceInfo)) {
        std::cerr << "Failed verifying\n";
        return -1;
    }

    if (!flasher.setAppInfo(flashFile, *deviceInfo, *flashAppInfo)) {
        std::cerr << "Failed flashing app info\n";
        return -1;
    }

    auto newAppInfo = flasher.appInfo();
    if (!!newAppInfo) {
        std::cout << "Flashed App version " << newAppInfo->appVersion() << "\n"
            << "Bootloader version " << newAppInfo->bootloaderVersion() << "\n";
    }

    //flasher.switchMode(Flasher::MODE_REGULAR);
    return 0;
}
