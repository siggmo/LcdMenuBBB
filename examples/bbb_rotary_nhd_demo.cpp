#include "demo_common.h"
#include <display/NHD0420D3Z_UARTAdapter.h>
#include <input/LinuxRotaryInputAdapter.h>
#include <renderer/CharacterDisplayRenderer.h>
#include <csignal>
#include <iostream>
#include <unistd.h>

static volatile bool running = true;
static void signalHandler(int /*signum*/) {
    running = false;
}

int main(int argc, char* argv[]) {
    // Graceful signal handling
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Default serial port /dev/ttyS3 (or override via argv[1])
    std::string uartPort = (argc > 1) ? argv[1] : "/dev/ttyS3";
    std::cout << "Starting BeagleBone Black NHD-0420D3Z + Rotary Demo...\n";
    std::cout << "UART Port: " << uartPort << " @ 9600 baud\n";

    // 1. Initialize NHD-0420D3Z-NSW-BBW-V3 UART Display Adapter & Character Renderer
    NHD0420D3Z_UARTAdapter lcd(uartPort, B9600, 20, 4);
    CharacterDisplayRenderer renderer(&lcd, 20, 4);
    LcdMenu menu(renderer);

    // 2. Initialize BeagleBone Black Rotary Encoder (eqep1) & Push Button (rotary_btn)
    LinuxRotaryInputAdapter rotaryInput(&menu, "", "", 2, 60, false);

    // 3. Begin Hardware and Display Main Menu from demo_common
    renderer.begin();
    rotaryInput.begin();
    menu.setScreen(&DemoMenu::getMainScreen());

    std::cout << "Menu initialized on " << uartPort << ".\n"
              << "eQEP Counter: " << rotaryInput.getCounterPath() << "\n"
              << "GPIO-Keys: " << rotaryInput.getEvdevPath() << "\n"
              << "Controls: [Rotate] Scroll/Adjust | [Short Press] Select/Confirm | [Long Press] Back/Discard\n"
              << "Running event loop (press Ctrl+C to terminate)...\n";

    auto lastSensorUpdate = std::chrono::steady_clock::now();
    auto startTime = std::chrono::steady_clock::now();

    // 4. Event Loop
    while (running) {
        // Poll hardware rotary encoder ticks and rotary button presses
        rotaryInput.observe();

        // Update display timeout and bound value polling
        menu.poll();

        // Periodic simulated telemetry update
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastSensorUpdate).count() >= 1) {
            lastSensorUpdate = now;
            DemoMenu::updateTelemetry(startTime);
        }

        usleep(10000);  // 10ms poll interval
    }

    std::cout << "\nShutting down display...\n";
    lcd.clear();
    std::cout << "Clean exit.\n";
    return 0;
}
