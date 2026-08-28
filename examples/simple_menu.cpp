#include <ItemBack.h>
#include <ItemCommand.h>
#include <ItemSubMenu.h>
#include <ItemToggle.h>
#include <LcdMenu.h>
#include <display/MockCharacterDisplayAdapter.h>
#include <renderer/CharacterDisplayRenderer.h>
#include <iostream>

void onHello() {
    std::cout << "\n>>> Hello from LcdMenu! <<<\n";
}

int main() {
    // 1. Initialize 4x20 mock display with ANSI TUI enabled
    MockCharacterDisplayAdapter display(20, 4, true, "simple_menu_output.txt");
    CharacterDisplayRenderer renderer(&display, 20, 4);
    LcdMenu menu(renderer);

    // 2. Build Submenu
    MenuScreen subScreen({
        ITEM_BACK("< Back"),
        ITEM_COMMAND("Say Hello", onHello),
    });

    // 3. Build Main Menu
    MenuScreen mainScreen({
        ITEM_SUBMENU("Sub Menu", subScreen),
        ITEM_TOGGLE("Setting", "ON", "OFF"),
        ITEM_COMMAND("Action", onHello),
    });

    // 4. Initialize and show menu
    renderer.begin();
    menu.setScreen(&mainScreen);

    std::cout << "\nInitial menu rendered. Simulating user navigation:\n";

    // Simulate navigating down
    std::cout << "\n[Action] Moving Down:\n";
    menu.process(DOWN);

    // Simulate pressing enter on toggle
    std::cout << "\n[Action] Toggling Setting:\n";
    menu.process(ENTER);

    // Simulate navigating up and entering submenu
    std::cout << "\n[Action] Moving Up:\n";
    menu.process(UP);
    std::cout << "\n[Action] Entering Submenu:\n";
    menu.process(ENTER);

    // Simulate going back
    std::cout << "\n[Action] Going Back:\n";
    menu.process(ENTER);

    std::cout << "\nFinished simple_menu demo successfully.\n";
    return 0;
}
