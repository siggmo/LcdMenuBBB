#include "demo_common.h"
#include <display/MockCharacterDisplayAdapter.h>
#include <input/PosixTerminalInputAdapter.h>
#include <renderer/CharacterDisplayRenderer.h>
#include <iostream>
#include <unistd.h>

int main() {
    // 1. Initialize Mock 4x20 Character Display with ANSI terminal UI rendering
    MockCharacterDisplayAdapter display(20, 4, true, "tui_menu_output.txt");
    CharacterDisplayRenderer renderer(&display, 20, 4);
    LcdMenu menu(renderer);

    // 2. Setup Terminal Keyboard Input
    PosixTerminalInputAdapter terminalInput(&menu);

    // 3. Begin Renderer and Display Main Menu from demo_common
    renderer.begin();
    menu.setScreen(&DemoMenu::getMainScreen());

    std::cout << "Interactive TUI Demo Started.\n"
              << "Controls: [Up/Down Arrows] Navigate | [Enter] Select/Edit | [Esc] Back/Discard | [q] Quit\n";

    auto lastSensorUpdate = std::chrono::steady_clock::now();
    auto startTime = std::chrono::steady_clock::now();

    // Main event loop
    while (!terminalInput.shouldQuit()) {
        terminalInput.observe();
        menu.poll();

        // Periodic simulated telemetry update
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastSensorUpdate).count() >= 1) {
            lastSensorUpdate = now;
            DemoMenu::updateTelemetry(startTime);
        }

        display.renderIfDirty();
        usleep(10000);  // 10ms tick for responsive UI
    }

    std::cout << "\nExited TUI Demo cleanly.\n";
    return 0;
}
