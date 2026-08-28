#include <ItemBack.h>
#include <ItemBool.h>
#include <ItemCommand.h>
#include <ItemInput.h>
#include <ItemList.h>
#include <ItemRange.h>
#include <ItemSubMenu.h>
#include <ItemToggle.h>
#include <ItemValue.h>
#include <LcdMenu.h>
#include <display/NHD0420D3Z_UARTAdapter.h>
#include <input/LinuxRotaryInputAdapter.h>
#include <renderer/CharacterDisplayRenderer.h>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <vector>

static volatile bool running = true;
static void signalHandler(int /*signum*/) {
    running = false;
}

static float systemVoltage = 5.02f;
static int lcdBrightness = 8;
static int fanSpeed = 65;
static bool pumpActive = true;
static int uptimeSec = 0;
static char unitTag[32] = "BBB-Node-1";

void onBrightnessChange(const int val) {
    std::cout << "[Event] LCD Brightness set to: " << val << "/8\n";
}

void onAlarmToggle(bool state) {
    std::cout << "[Event] Alarm Sound toggled to: " << (state ? "ON" : "OFF") << "\n";
}

void onRelay1Toggle(bool state) {
    std::cout << "[Event] Relay 1 toggled to: " << (state ? "CLOSED" : "OPEN") << "\n";
}

void onModeChange(const uint8_t index) {
    const char* modes[] = {"AUTO", "MANUAL", "ECO", "BOOST"};
    if (index < 4) {
        std::cout << "[Event] Mode changed to: " << modes[index] << "\n";
    }
}

void onFanSpeedChange(const int val) {
    std::cout << "[Event] Fan Speed set to: " << val << "%\n";
}

void onStatusLedToggle(bool state) {
    std::cout << "[Event] Status LED toggled to: " << (state ? "ACTIVE" : "IDLE") << "\n";
}

void onPumpChange(const bool state) {
    std::cout << "[Event] Main Pump toggled to: " << (state ? "ON" : "OFF") << "\n";
}

void onProfileChange(const uint8_t index) {
    const char* profiles[] = {"DEFAULT", "PWR-SAVE", "HI-PERF"};
    if (index < 3) {
        std::cout << "[Event] Profile selected: " << profiles[index] << "\n";
    }
}

void onReboot() {
    std::cout << "[Action] Reboot command triggered!\n";
}

int main(int argc, char* argv[]) {
    // Graceful signal handling
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Default serial port /dev/ttyS3 (or override via argv[1])
    std::string uartPort = (argc > 1) ? argv[1] : "/dev/ttyS3";
    std::cout << "Starting BeagleBone Black NHD-0420D3Z + Rotary Demo...\n";
    std::cout << "UART Port: " << uartPort << " @ 9600 baud\n";

    // 1. Initialize NHD-0420D3Z-NSW-BBW-V3 UART Display Adapter
    NHD0420D3Z_UARTAdapter lcd(uartPort, B9600, 20, 4);
    CharacterDisplayRenderer renderer(&lcd, 20, 4);
    LcdMenu menu(renderer);

    // 2. Initialize BeagleBone Black Rotary Encoder (eqep1) & Push Button (rotary_btn)
    LinuxRotaryInputAdapter rotaryInput(&menu, "", "", 2, 60, false);

    // 3. Define Submenu: Display Settings
    MenuScreen displaySettingsScreen({
        ITEM_BACK("< Back"),
        ITEM_RANGE("Brightness", lcdBrightness, 1, 1, 8, "%d", 0, false, onBrightnessChange),
        ITEM_TOGGLE("Alarm Sound", "ON", "OFF", onAlarmToggle),
    });

    // 4. Define Submenu: System Sensors
    MenuScreen sensorsScreen({
        ITEM_BACK("< Back"),
        ITEM_VALUE("Voltage", systemVoltage, "%.2f V"),
        ITEM_TOGGLE("Relay 1", "CLOSED", "OPEN", onRelay1Toggle),
    });

    // 5. Define Main Screen with 9 items (demonstrates scrolling with ^ and v indicators)
    MenuScreen mainScreen({
        ITEM_SUBMENU("Display Setup", displaySettingsScreen),
        ITEM_SUBMENU("Sensors/Relays", sensorsScreen),
        ITEM_LIST("Op Mode", std::vector<const char*>{"AUTO", "MANUAL", "ECO", "BOOST"}, onModeChange, 0, "%s", 0, true),
        ITEM_RANGE("Fan Speed", fanSpeed, 5, 0, 100, "%d", 0, false, onFanSpeedChange),
        ITEM_TOGGLE("Status LED", "ACTIVE", "IDLE", onStatusLedToggle),
        ITEM_BOOL("Main Pump", pumpActive, "ON", "OFF", onPumpChange),
        ITEM_LIST("Profile", std::vector<const char*>{"DEFAULT", "PWR-SAVE", "HI-PERF"}, onProfileChange, 0, "%s", 0, true),
        ITEM_VALUE("Unit Tag", unitTag, "%s"),
        ITEM_COMMAND("Reboot System", onReboot),
    });

    // 6. Begin Hardware and Display Main Menu
    renderer.begin();
    rotaryInput.begin();
    menu.setScreen(&mainScreen);

    std::cout << "Menu initialized on " << uartPort << ".\n"
              << "eQEP Counter: " << rotaryInput.getCounterPath() << "\n"
              << "GPIO-Keys: " << rotaryInput.getEvdevPath() << "\n"
              << "Controls: [Rotate] Scroll/Adjust | [Short Press] Select/Confirm | [Long Press] Back/Discard\n"
              << "Running event loop (press Ctrl+C to terminate)...\n";

    auto lastSensorUpdate = std::chrono::steady_clock::now();
    auto startTime = std::chrono::steady_clock::now();

    // 7. Event Loop
    while (running) {
        // Poll hardware rotary encoder ticks and rotary button presses
        rotaryInput.observe();

        // Update display timeout and bound value polling
        menu.poll();

        // Simulate periodic telemetry update
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastSensorUpdate).count() >= 1) {
            lastSensorUpdate = now;
            systemVoltage = 5.00f + (static_cast<float>(rand() % 10) / 100.0f);
            uptimeSec = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
        }

        usleep(10000);  // 10ms poll interval
    }

    std::cout << "\nShutting down display...\n";
    lcd.clear();
    std::cout << "Clean exit.\n";
    return 0;
}
