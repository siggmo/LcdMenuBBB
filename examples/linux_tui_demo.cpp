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
#include <display/MockCharacterDisplayAdapter.h>
#include <input/PosixTerminalInputAdapter.h>
#include <renderer/CharacterDisplayRenderer.h>
#include <chrono>
#include <iostream>
#include <unistd.h>
#include <vector>

static float temperature = 24.5f;
static int brightness = 80;
static int fanSpeed = 50;
static bool pumpState = true;
static int uptimeSec = 0;
static char deviceName[32] = "BeagleBone";

void onSave() {
    std::cout << "\a";  // Beep
}

int main() {
    // 1. Initialize Mock 4x20 Character Display with ANSI terminal UI rendering
    MockCharacterDisplayAdapter display(20, 4, true, "tui_menu_output.txt");
    CharacterDisplayRenderer renderer(&display, 20, 4);
    LcdMenu menu(renderer);

    // 2. Setup Terminal Keyboard Input
    PosixTerminalInputAdapter terminalInput(&menu);

    // 3. Define Submenu for Settings
    MenuScreen settingsScreen({
        ITEM_BACK("< Back"),
        ITEM_RANGE("Brightness", brightness, 5, 0, 100),
        ITEM_TOGGLE("LED Active", "YES", "NO"),
        ITEM_INPUT("Device Name", deviceName, [](char* val) {
            std::cout << "Name updated: " << val << "\n";
        }),
    });

    // 4. Define Submenu for Diagnostics
    MenuScreen diagScreen({
        ITEM_BACK("< Back"),
        ITEM_VALUE("Temp", temperature, "%.1f C"),
        ITEM_COMMAND("Save Config", onSave),
    });

    // 5. Define Main Screen with 9 items (demonstrates scrolling with ^ and v indicators)
    MenuScreen mainScreen({
        ITEM_SUBMENU("Settings", settingsScreen),
        ITEM_SUBMENU("Diagnostics", diagScreen),
        ITEM_LIST("Profile", std::vector<const char*>{"ECO", "AUTO", "BOOST", "TURBO"}, nullptr, 0, "%s", 0, true),
        ITEM_RANGE("Fan Speed", fanSpeed, 5, 0, 100),
        ITEM_TOGGLE("Power", "ON", "OFF"),
        ITEM_BOOL("Aux Pump", pumpState),
        ITEM_INPUT("Unit Tag", deviceName, [](char* val) {
            std::cout << "Tag updated: " << val << "\n";
        }),
        ITEM_VALUE("Uptime", uptimeSec, "%d s"),
        ITEM_COMMAND("Save All", onSave),
    });

    renderer.begin();
    menu.setScreen(&mainScreen);

    std::cout << "Interactive TUI Demo Started.\n"
              << "Controls: [Up/Down Arrows] Navigate | [Enter] Select/Edit | [Esc] Back | [q] Quit\n";

    // Main event loop
    while (!terminalInput.shouldQuit()) {
        terminalInput.observe();
        menu.poll();
        display.renderIfDirty();
        usleep(10000);  // 10ms tick for responsive UI
    }

    std::cout << "\nExited TUI Demo cleanly.\n";
    return 0;
}
