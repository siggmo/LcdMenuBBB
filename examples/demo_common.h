#pragma once

#include <ItemBack.h>
#include <ItemBool.h>
#include <ItemCommand.h>
#include <ItemList.h>
#include <ItemRange.h>
#include <ItemSubMenu.h>
#include <ItemToggle.h>
#include <ItemValue.h>
#include <LcdMenu.h>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace DemoMenu {

static float systemVoltage = 5.02f;
static int lcdBrightness = 8;
static int fanSpeed = 65;
static bool pumpActive = true;
static int uptimeSec = 0;
static char unitTag[64] = "BBB-PowerUnit-Station-Alpha-01";

inline void onBrightnessChange(const int val) {
    std::cout << "[Event] LCD Brightness set to: " << val << "/8\n";
}

inline void onAlarmToggle(bool state) {
    std::cout << "[Event] Alarm Sound toggled to: " << (state ? "ON" : "OFF") << "\n";
}

inline void onRelay1Toggle(bool state) {
    std::cout << "[Event] Relay 1 toggled to: " << (state ? "CLOSED" : "OPEN") << "\n";
}

inline void onModeChange(const uint8_t index) {
    const char* modes[] = {"AUTO", "MANUAL", "ECO", "BOOST"};
    if (index < 4) {
        std::cout << "[Event] Mode changed to: " << modes[index] << "\n";
    }
}

inline void onFanSpeedChange(const int val) {
    std::cout << "[Event] Fan Speed set to: " << val << "%\n";
}

inline void onStatusLedToggle(bool state) {
    std::cout << "[Event] Status LED toggled to: " << (state ? "ACTIVE" : "IDLE") << "\n";
}

inline void onPumpChange(const bool state) {
    std::cout << "[Event] Main Pump toggled to: " << (state ? "ON" : "OFF") << "\n";
}

inline void onProfileChange(const uint8_t index) {
    const char* profiles[] = {"DEFAULT", "PWR-SAVE", "HI-PERF"};
    if (index < 3) {
        std::cout << "[Event] Profile selected: " << profiles[index] << "\n";
    }
}

inline void onReboot() {
    std::cout << "[Action] Reboot command triggered!\n";
}

// Submenu 1: Display Settings
inline MenuScreen& getDisplaySettingsScreen() {
    static MenuScreen screen({
        ITEM_BACK("< Back"),
        ITEM_RANGE("Brightness", lcdBrightness, 1, 1, 8, "%d", 0, false, onBrightnessChange),
        ITEM_TOGGLE("Alarm Sound", "ON", "OFF", onAlarmToggle),
    });
    return screen;
}

// Submenu 2: System Sensors
inline MenuScreen& getSensorsScreen() {
    static MenuScreen screen({
        ITEM_BACK("< Back"),
        ITEM_VALUE("Voltage", systemVoltage, "%.2f V"),
        ITEM_TOGGLE("Relay 1", "CLOSED", "OPEN", onRelay1Toggle),
    });
    return screen;
}

// Main Menu Screen (9 items)
inline MenuScreen& getMainScreen() {
    static MenuScreen screen({
        ITEM_SUBMENU("Display Setup", getDisplaySettingsScreen()),
        ITEM_SUBMENU("Sensors/Relays", getSensorsScreen()),
        ITEM_LIST("Op Mode", std::vector<const char*>{"AUTO", "MANUAL", "ECO", "BOOST"}, onModeChange, 0, "%s", 0, true),
        ITEM_RANGE("Fan Speed", fanSpeed, 5, 0, 100, "%d", 0, false, onFanSpeedChange),
        ITEM_TOGGLE("Status LED", "ACTIVE", "IDLE", onStatusLedToggle),
        ITEM_BOOL("Main Pump", pumpActive, "ON", "OFF", onPumpChange),
        ITEM_LIST("Profile", std::vector<const char*>{"DEFAULT", "PWR-SAVE", "HI-PERF"}, onProfileChange, 0, "%s", 0, true),
        ITEM_VALUE("Unit Tag", unitTag, "%s"),
        ITEM_COMMAND("Reboot System", onReboot),
    });
    return screen;
}

// Helper to simulate periodic background telemetry updates
inline void updateTelemetry(std::chrono::steady_clock::time_point startTime) {
    auto now = std::chrono::steady_clock::now();
    systemVoltage = 5.00f + (static_cast<float>(rand() % 10) / 100.0f);
    uptimeSec = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
}

}  // namespace DemoMenu
