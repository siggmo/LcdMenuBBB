#include "LinuxRotaryInputAdapter.h"
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>

LinuxRotaryInputAdapter::LinuxRotaryInputAdapter(
    LcdMenu* menu,
    const std::string& counterPath,
    const std::string& evdevPath,
    int countsPerStep,
    bool reverseDirection,
    int longPressMs)
    : InputInterface(menu),
      counterPath(counterPath),
      evdevPath(evdevPath),
      counterFd(-1),
      evdevFd(-1),
      lastCount(0),
      countsPerStep(countsPerStep > 0 ? countsPerStep : 1),
      reverseDirection(reverseDirection),
      accumulatedCounts(0),
      initialized(false),
      longPressMs(longPressMs > 0 ? longPressMs : 500),
      buttonPressed(false),
      longPressTriggered(false) {}

LinuxRotaryInputAdapter::~LinuxRotaryInputAdapter() {
    closeDescriptors();
}

void LinuxRotaryInputAdapter::closeDescriptors() {
    if (counterFd >= 0) {
        close(counterFd);
        counterFd = -1;
    }
    if (evdevFd >= 0) {
        close(evdevFd);
        evdevFd = -1;
    }
}

bool LinuxRotaryInputAdapter::autoDetectCounterPath() {
    // 1. Check modern Linux Counter Subsystem (/sys/bus/counter/devices/counterX/count0/count)
    for (int i = 0; i < 8; i++) {
        std::string namePath = "/sys/bus/counter/devices/counter" + std::to_string(i) + "/name";
        std::ifstream nameFile(namePath);
        if (nameFile.is_open()) {
            std::string name;
            std::getline(nameFile, name);
            if (name.find("eqep") != std::string::npos || name.find("48302180") != std::string::npos) {
                std::string countCandidate = "/sys/bus/counter/devices/counter" + std::to_string(i) + "/count0/count";
                struct stat st;
                if (stat(countCandidate.c_str(), &st) == 0) {
                    counterPath = countCandidate;
                    return true;
                }
            }
        }

        // Generic check for counterX/count0/count
        std::string fallbackCount = "/sys/bus/counter/devices/counter" + std::to_string(i) + "/count0/count";
        struct stat st;
        if (stat(fallbackCount.c_str(), &st) == 0) {
            counterPath = fallbackCount;
            return true;
        }
    }

    // 2. Check legacy eQEP paths
    const char* legacyPaths[] = {
        "/sys/devices/platform/ocp/48302000.epwmss/48302180.eqep/position",
        "/sys/devices/platform/ocp/48302000.epwmss/48302180.eqep/count0/count",
        "/sys/devices/platform/ocp/48304000.epwmss/48304180.eqep/position"
    };

    for (const char* path : legacyPaths) {
        struct stat st;
        if (stat(path, &st) == 0) {
            counterPath = path;
            return true;
        }
    }

    return false;
}

bool LinuxRotaryInputAdapter::autoDetectEvdevPath() {
    // Check /dev/input/by-path/ for gpio-keys
    DIR* dir = opendir("/dev/input/by-path");
    if (dir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strstr(entry->d_name, "gpio_keys") || strstr(entry->d_name, "gpio-keys")) {
                evdevPath = std::string("/dev/input/by-path/") + entry->d_name;
                closedir(dir);
                return true;
            }
        }
        closedir(dir);
    }

    // Scan /dev/input/event* by device name in /sys/class/input/eventX/device/name
    for (int i = 0; i < 16; i++) {
        std::string namePath = "/sys/class/input/event" + std::to_string(i) + "/device/name";
        std::ifstream nameFile(namePath);
        if (nameFile.is_open()) {
            std::string name;
            std::getline(nameFile, name);
            if (name.find("gpio_keys") != std::string::npos || name.find("gpio-keys") != std::string::npos) {
                evdevPath = "/dev/input/event" + std::to_string(i);
                return true;
            }
        }
    }

    // Default fallback
    struct stat st;
    if (stat("/dev/input/event0", &st) == 0) {
        evdevPath = "/dev/input/event0";
        return true;
    }

    return false;
}

bool LinuxRotaryInputAdapter::openCounter() {
    if (counterPath.empty()) {
        autoDetectCounterPath();
    }

    if (counterPath.empty()) {
        fprintf(stderr, "LinuxRotaryInputAdapter: Could not locate eQEP counter path in sysfs.\n");
        return false;
    }

    counterFd = open(counterPath.c_str(), O_RDONLY);
    if (counterFd < 0) {
        fprintf(stderr, "LinuxRotaryInputAdapter: Failed to open counter %s: %s\n", counterPath.c_str(), strerror(errno));
        return false;
    }

    // Read initial count baseline
    char buf[64];
    ssize_t bytesRead = pread(counterFd, buf, sizeof(buf) - 1, 0);
    if (bytesRead > 0) {
        buf[bytesRead] = '\0';
        lastCount = std::strtoll(buf, nullptr, 10);
    }

    return true;
}

bool LinuxRotaryInputAdapter::openEvdev() {
    if (evdevPath.empty()) {
        autoDetectEvdevPath();
    }

    if (evdevPath.empty()) {
        fprintf(stderr, "LinuxRotaryInputAdapter: Could not locate gpio-keys event device.\n");
        return false;
    }

    evdevFd = open(evdevPath.c_str(), O_RDONLY | O_NONBLOCK);
    if (evdevFd < 0) {
        fprintf(stderr, "LinuxRotaryInputAdapter: Failed to open evdev %s: %s\n", evdevPath.c_str(), strerror(errno));
        return false;
    }

    return true;
}

bool LinuxRotaryInputAdapter::begin() {
    closeDescriptors();
    bool counterOk = openCounter();
    bool evdevOk = openEvdev();
    initialized = counterOk || evdevOk;
    return initialized;
}

void LinuxRotaryInputAdapter::processEncoder() {
    if (counterFd < 0 || menu == nullptr) return;

    char buf[64];
    ssize_t bytesRead = pread(counterFd, buf, sizeof(buf) - 1, 0);
    if (bytesRead <= 0) return;

    buf[bytesRead] = '\0';
    int64_t currentCount = std::strtoll(buf, nullptr, 10);
    int64_t delta = currentCount - lastCount;
    lastCount = currentCount;

    if (delta == 0) return;

    accumulatedCounts += delta;

    while (accumulatedCounts >= countsPerStep) {
        accumulatedCounts -= countsPerStep;
        if (reverseDirection) {
            menu->process(MenuItem::isEditing() ? LEFT : UP);
        } else {
            menu->process(MenuItem::isEditing() ? RIGHT : DOWN);
        }
    }

    while (accumulatedCounts <= -countsPerStep) {
        accumulatedCounts += countsPerStep;
        if (reverseDirection) {
            menu->process(MenuItem::isEditing() ? RIGHT : DOWN);
        } else {
            menu->process(MenuItem::isEditing() ? LEFT : UP);
        }
    }
}

void LinuxRotaryInputAdapter::processEvents() {
    if (evdevFd < 0 || menu == nullptr) return;

    struct input_event ev;
    while (read(evdevFd, &ev, sizeof(ev)) == sizeof(ev)) {
        if (ev.type == EV_KEY) {
            if (ev.code == KEY_ENTER) {  // rotary_btn
                auto now = std::chrono::steady_clock::now();
                if (ev.value == 1) {  // Key Down
                    buttonPressed = true;
                    longPressTriggered = false;
                    pressStartTime = now;
                } else if (ev.value == 0) {  // Key Up
                    if (buttonPressed) {
                        buttonPressed = false;
                        if (!longPressTriggered) {
                            menu->process(ENTER);  // Short press -> ENTER
                        }
                    }
                }
            }
            // Note: KEY_PROG1 (boot_btn) is intentionally ignored as requested.
        }
    }
}

void LinuxRotaryInputAdapter::observe() {
    if (!initialized) {
        begin();
    }
    processEncoder();
    processEvents();

    if (buttonPressed && !longPressTriggered && menu != nullptr) {
        auto now = std::chrono::steady_clock::now();
        int elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - pressStartTime).count();
        if (elapsedMs >= longPressMs) {
            longPressTriggered = true;
            menu->process(BACK);  // Long press -> BACK (ESC / Cancel)
        }
    }
}
