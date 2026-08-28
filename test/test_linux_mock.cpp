#include <ItemBack.h>
#include <ItemCommand.h>
#include <ItemInput.h>
#include <ItemList.h>
#include <ItemRange.h>
#include <ItemSubMenu.h>
#include <ItemToggle.h>
#include <ItemValue.h>
#include <LcdMenu.h>
#include <display/MockCharacterDisplayAdapter.h>
#include <renderer/CharacterDisplayRenderer.h>
#include <cassert>
#include <iostream>
#include <string>

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "FAIL: " << msg << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
            assert(cond); \
        } else { \
            std::cout << "  PASS: " << msg << "\n"; \
        } \
    } while (0)

void test_menu_initialization_and_rendering() {
    std::cout << "\n[TEST] test_menu_initialization_and_rendering\n";
    MockCharacterDisplayAdapter display(20, 4, false);
    CharacterDisplayRenderer renderer(&display, 20, 4);
    LcdMenu menu(renderer);

    MenuScreen mainScreen({
        new MenuItem("Option 1"),
        new MenuItem("Option 2"),
        new MenuItem("Option 3"),
        new MenuItem("Option 4"),
    });

    renderer.begin();
    menu.setScreen(&mainScreen);

    std::string row0 = display.getRow(0);
    std::string row1 = display.getRow(1);

    TEST_ASSERT(row0.find("Option 1") != std::string::npos, "Row 0 contains 'Option 1'");
    TEST_ASSERT(row1.find("Option 2") != std::string::npos, "Row 1 contains 'Option 2'");
    TEST_ASSERT(menu.getCursor() == 0, "Cursor is at index 0");
}

void test_cursor_navigation_down_up() {
    std::cout << "\n[TEST] test_cursor_navigation_down_up\n";
    MockCharacterDisplayAdapter display(20, 4, false);
    CharacterDisplayRenderer renderer(&display, 20, 4);
    LcdMenu menu(renderer);

    MenuScreen mainScreen({
        new MenuItem("Item A"),
        new MenuItem("Item B"),
        new MenuItem("Item C"),
    });

    renderer.begin();
    menu.setScreen(&mainScreen);

    TEST_ASSERT(menu.getCursor() == 0, "Initial cursor is 0");

    menu.process(DOWN);
    TEST_ASSERT(menu.getCursor() == 1, "Cursor moved DOWN to 1");

    menu.process(DOWN);
    TEST_ASSERT(menu.getCursor() == 2, "Cursor moved DOWN to 2");

    menu.process(UP);
    TEST_ASSERT(menu.getCursor() == 1, "Cursor moved UP to 1");
}

static bool toggleCallbackState = false;
void onToggleTest(bool on) {
    toggleCallbackState = on;
}

void test_toggle_item() {
    std::cout << "\n[TEST] test_toggle_item\n";
    MockCharacterDisplayAdapter display(20, 4, false);
    CharacterDisplayRenderer renderer(&display, 20, 4);
    LcdMenu menu(renderer);

    toggleCallbackState = false;
    MenuScreen mainScreen({
        ITEM_TOGGLE("Motor", "ON", "OFF", onToggleTest),
    });

    renderer.begin();
    menu.setScreen(&mainScreen);

    TEST_ASSERT(display.getRow(0).find("OFF") != std::string::npos, "Initial toggle is OFF");

    menu.process(ENTER);
    TEST_ASSERT(display.getRow(0).find("ON") != std::string::npos, "Toggle switched to ON");
    TEST_ASSERT(toggleCallbackState == true, "Callback received state == true");

    menu.process(ENTER);
    TEST_ASSERT(display.getRow(0).find("OFF") != std::string::npos, "Toggle switched back to OFF");
    TEST_ASSERT(toggleCallbackState == false, "Callback received state == false");
}

void test_range_item() {
    std::cout << "\n[TEST] test_range_item\n";
    MockCharacterDisplayAdapter display(20, 4, false);
    CharacterDisplayRenderer renderer(&display, 20, 4);
    LcdMenu menu(renderer);

    int volume = 50;
    MenuScreen mainScreen({
        ITEM_RANGE("Volume", volume, 10, 0, 100),
    });

    renderer.begin();
    menu.setScreen(&mainScreen);

    TEST_ASSERT(display.getRow(0).find("50") != std::string::npos, "Initial range is 50");

    // Enter edit mode
    menu.process(ENTER);
    TEST_ASSERT(MenuItem::isEditing() == true, "Entered edit mode");

    // Increment
    menu.process(RIGHT);
    TEST_ASSERT(display.getRow(0).find("60") != std::string::npos, "Range incremented to 60");

    // Decrement
    menu.process(LEFT);
    TEST_ASSERT(display.getRow(0).find("50") != std::string::npos, "Range decremented back to 50");

    // Commit edit
    menu.process(ENTER);
    TEST_ASSERT(MenuItem::isEditing() == false, "Committed edit mode");
}

void test_submenu_and_back() {
    std::cout << "\n[TEST] test_submenu_and_back\n";
    MockCharacterDisplayAdapter display(20, 4, false);
    CharacterDisplayRenderer renderer(&display, 20, 4);
    LcdMenu menu(renderer);

    MenuScreen subScreen({
        ITEM_BACK("< Back"),
        new MenuItem("Sub Item 1"),
    });

    MenuScreen mainScreen({
        ITEM_SUBMENU("Settings", subScreen),
        new MenuItem("Main Item 2"),
    });

    renderer.begin();
    menu.setScreen(&mainScreen);

    TEST_ASSERT(display.getRow(0).find("Settings") != std::string::npos, "Main menu shown");

    // Enter Submenu
    menu.process(ENTER);
    TEST_ASSERT(display.getRow(0).find("< Back") != std::string::npos, "Entered submenu, showing '< Back'");
    TEST_ASSERT(menu.getScreen() == &subScreen, "Current screen is subScreen");

    // Select < Back
    menu.process(ENTER);
    TEST_ASSERT(display.getRow(0).find("Settings") != std::string::npos, "Returned to main menu");
    TEST_ASSERT(menu.getScreen() == &mainScreen, "Current screen is mainScreen");
}

static char inputCallbackBuffer[32] = "";
void onInputTest(char* val) {
    if (val) {
        strncpy(inputCallbackBuffer, val, sizeof(inputCallbackBuffer) - 1);
    }
}

void test_input_item() {
    std::cout << "\n[TEST] test_input_item\n";
    MockCharacterDisplayAdapter display(20, 4, false);
    CharacterDisplayRenderer renderer(&display, 20, 4);
    LcdMenu menu(renderer);

    char name[16] = "Test";
    inputCallbackBuffer[0] = '\0';
    MenuScreen mainScreen({
        ITEM_INPUT("Name", name, onInputTest),
    });

    renderer.begin();
    menu.setScreen(&mainScreen);

    TEST_ASSERT(display.getRow(0).find("Test") != std::string::npos, "Initial input text is 'Test'");

    // Enter edit mode
    menu.process(ENTER);
    TEST_ASSERT(MenuItem::isEditing() == true, "ItemInput in edit mode");

    // Type character '1'
    menu.process('1');
    TEST_ASSERT(display.getRow(0).find("Test1") != std::string::npos, "Typed '1', text is 'Test1'");

    // Exit edit mode (BACK key commits/exits edit mode for ItemInput)
    menu.process(BACK);
    TEST_ASSERT(MenuItem::isEditing() == false, "ItemInput exited edit mode");
    TEST_ASSERT(std::string(inputCallbackBuffer) == "Test1", "Callback received committed text 'Test1'");
}

void test_mock_file_dump() {
    std::cout << "\n[TEST] test_mock_file_dump\n";
    const std::string dumpPath = "/tmp/test_lcd_dump.txt";
    MockCharacterDisplayAdapter display(20, 4, false, dumpPath);
    CharacterDisplayRenderer renderer(&display, 20, 4);
    LcdMenu menu(renderer);

    MenuScreen mainScreen({
        new MenuItem("File Dump Test"),
    });

    renderer.begin();
    menu.setScreen(&mainScreen);

    std::string box = display.getFormattedBox();
    TEST_ASSERT(box.find("File Dump Test") != std::string::npos, "Formatted box contains 'File Dump Test'");
    TEST_ASSERT(box.find("+--------------------+") != std::string::npos, "Formatted box has border");
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Running LcdMenu Linux Unit Tests\n";
    std::cout << "========================================\n";

    test_menu_initialization_and_rendering();
    test_cursor_navigation_down_up();
    test_toggle_item();
    test_range_item();
    test_submenu_and_back();
    test_input_item();
    test_mock_file_dump();

    std::cout << "\n========================================\n";
    std::cout << "ALL UNIT TESTS PASSED SUCCESSFULLY!\n";
    std::cout << "========================================\n";
    return 0;
}
