#include <ItemBack.h>
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

static volatile bool running = true;
static void signalHandler(int /*signum*/) {
    running = false;
}

static float systemVoltage = 5.02f;
static int lcdBrightness = 8;

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
    LinuxRotaryInputAdapter rotaryInput(&menu, "", "", 1, false);

    // 3. Define Submenu: Display Settings
    MenuScreen displaySettingsScreen({
        ITEM_BACK("< Back"),
        ITEM_RANGE("Brightness", lcdBrightness, 1, 1, 8),
        ITEM_TOGGLE("Alarm Sound", "ON", "OFF"),
    });

    // 4. Define Submenu: System Sensors
    MenuScreen sensorsScreen({
        ITEM_BACK("< Back"),
        ITEM_VALUE("Voltage", systemVoltage, "%.2f V"),
        ITEM_TOGGLE("Relay 1", "CLOSED", "OPEN"),
    });

    // 5. Define Main Screen
    MenuScreen mainScreen({
        ITEM_SUBMENU("Display Setup", displaySettingsScreen),
        ITEM_SUBMENU("Sensors/Relays", sensorsScreen),
        ITEM_TOGGLE("Status LED", "ACTIVE", "IDLE"),
        ITEM_COMMAND("Clear Screen", []() {
            std::cout << "Command executed: Clear\n";
        }),
    });

    // 6. Begin Hardware and Display Main Menu
    renderer.begin();
    rotaryInput.begin();
    menu.setScreen(&mainScreen);

    std::cout << "Menu initialized on " << uartPort << ".\n"
              << "eQEP Counter: " << rotaryInput.getCounterPath() << "\n"
              << "GPIO-Keys: " << rotaryInput.getEvdevPath() << "\n"
              << "Running event loop (press Ctrl+C to terminate)...\n";

    auto lastSensorUpdate = std::chrono::steady_clock::now();

    // 7. Event Loop
    while (running) {
        // Poll hardware rotary encoder ticks and rotary button presses
        rotaryInput.observe();

        // Update display timeout and bound value polling
        menu.poll();

        // Simulate sensor update every 2 seconds
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastSensorUpdate).count() >= 2) {
            lastSensorUpdate = now;
            systemVoltage = 5.00f + (static_cast<float>(rand() % 10) / 100.0f);
        }

        usleep(10000);  // 10ms poll interval
    }

    std::cout << "\nShutting down display...\n";
    lcd.clear();
    std::cout << "Clean exit.\n";
    return 0;
}
