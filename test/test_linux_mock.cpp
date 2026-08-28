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
#include <input/LinuxRotaryInputAdapter.h>
#include <renderer/CharacterDisplayRenderer.h>
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

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

    // Commit edit (2nd ENTER)
    menu.process(ENTER);
    TEST_ASSERT(MenuItem::isEditing() == false, "Committed edit mode on 2nd ENTER");
}

void test_range_item_cancel_discards_value() {
    std::cout << "\n[TEST] test_range_item_cancel_discards_value\n";
    MockCharacterDisplayAdapter display(20, 4, false);
    CharacterDisplayRenderer renderer(&display, 20, 4);
    LcdMenu menu(renderer);

    int volume = 50;
    MenuScreen mainScreen({
        ITEM_RANGE("Volume", volume, 10, 0, 100),
    });

    renderer.begin();
    menu.setScreen(&mainScreen);

    // Enter edit mode
    menu.process(ENTER);
    TEST_ASSERT(MenuItem::isEditing() == true, "Entered edit mode");

    // Increment to 60
    menu.process(RIGHT);
    TEST_ASSERT(display.getRow(0).find("60") != std::string::npos, "Range incremented to 60");

    // Cancel edit mode with BACK (ESC)
    menu.process(BACK);
    TEST_ASSERT(MenuItem::isEditing() == false, "Exited edit mode on BACK (ESC)");
    TEST_ASSERT(display.getRow(0).find("50") != std::string::npos, "Range reverted back to original 50 on cancel");
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

void test_input_item_commit_and_cancel() {
    std::cout << "\n[TEST] test_input_item_commit_and_cancel\n";
    MockCharacterDisplayAdapter display(20, 4, false);
    CharacterDisplayRenderer renderer(&display, 20, 4);
    LcdMenu menu(renderer);

    inputCallbackBuffer[0] = '\0';
    ItemInput* itemInput = new ItemInput("Name", (char*)"Test", onInputTest);
    MenuScreen mainScreen({
        itemInput,
    });

    renderer.begin();
    menu.setScreen(&mainScreen);

    TEST_ASSERT(display.getRow(0).find("Test") != std::string::npos, "Initial input text is 'Test'");

    // 1. Test Commit with 2nd ENTER
    menu.process(ENTER);
    TEST_ASSERT(MenuItem::isEditing() == true, "ItemInput entered edit mode on 1st ENTER");

    menu.process('1');
    TEST_ASSERT(display.getRow(0).find("Test1") != std::string::npos, "Typed '1', text is 'Test1'");

    menu.process(ENTER);
    TEST_ASSERT(MenuItem::isEditing() == false, "ItemInput exited edit mode on 2nd ENTER");
    TEST_ASSERT(std::string(inputCallbackBuffer) == "Test1", "Callback received committed text 'Test1'");
    TEST_ASSERT(std::string(itemInput->getValue()) == "Test1", "ItemInput getValue() holds 'Test1'");

    // 2. Test Discard with BACK (ESC)
    menu.process(ENTER);
    TEST_ASSERT(MenuItem::isEditing() == true, "ItemInput re-entered edit mode on ENTER");

    menu.process('X');
    TEST_ASSERT(display.getRow(0).find("Test1X") != std::string::npos, "Typed 'X', text is 'Test1X'");

    menu.process(BACK);  // ESC / Cancel
    TEST_ASSERT(MenuItem::isEditing() == false, "ItemInput exited edit mode on BACK (ESC)");
    TEST_ASSERT(display.getRow(0).find("Test1") != std::string::npos, "Display text reverted back to 'Test1' on discard");
    TEST_ASSERT(std::string(itemInput->getValue()) == "Test1", "ItemInput getValue() reverted back to 'Test1'");
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

void test_rotary_counter_wrap_around() {
    std::cout << "\n[TEST] test_rotary_counter_wrap_around\n";
    MockCharacterDisplayAdapter display(20, 4, false);
    CharacterDisplayRenderer renderer(&display, 20, 4);
    LcdMenu menu(renderer);

    MenuScreen mainScreen({
        new MenuItem("Item 0"),
        new MenuItem("Item 1"),
        new MenuItem("Item 2"),
    });

    renderer.begin();
    menu.setScreen(&mainScreen);

    std::string tempCounter = "/tmp/test_counter.txt";
    std::ofstream out(tempCounter);
    out << "58\n";
    out.close();

    // 2 counts per step, ceiling = 60
    LinuxRotaryInputAdapter adapter(&menu, tempCounter, "/dev/null", 2, 60, false);
    adapter.begin();
    TEST_ASSERT(menu.getCursor() == 0, "Initial cursor at 0");

    // Move forward across wrap-around: 58 -> 0 (delta = +2 ticks after wrap-around)
    out.open(tempCounter, std::ofstream::trunc);
    out << "0\n";
    out.close();
    adapter.observe();
    TEST_ASSERT(menu.getCursor() == 1, "Cursor moved DOWN across ceiling wrap-around (58 -> 0)");

    // Move backward across wrap-around: 0 -> 58 (delta = -2 ticks after wrap-around)
    out.open(tempCounter, std::ofstream::trunc);
    out << "58\n";
    out.close();
    adapter.observe();
    TEST_ASSERT(menu.getCursor() == 0, "Cursor moved UP across ceiling wrap-around (0 -> 58)");

    unlink(tempCounter.c_str());
}

static uint8_t listCallbackIndex = 99;
void onListTest(const uint8_t index) {
    listCallbackIndex = index;
}

void test_list_item() {
    std::cout << "\n[TEST] test_list_item\n";
    MockCharacterDisplayAdapter display(20, 4, false);
    CharacterDisplayRenderer renderer(&display, 20, 4);
    LcdMenu menu(renderer);

    listCallbackIndex = 99;
    // Pass temporary rvalue vector
    MenuScreen mainScreen({
        ITEM_LIST("Mode", std::vector<const char*>{"ECO", "AUTO", "BOOST"}, onListTest, 0, "%s", 0, true),
    });

    renderer.begin();
    menu.setScreen(&mainScreen);

    TEST_ASSERT(display.getRow(0).find("ECO") != std::string::npos, "Initial list item is 'ECO'");

    // Enter edit mode
    menu.process(ENTER);
    TEST_ASSERT(MenuItem::isEditing() == true, "ItemList entered edit mode");

    // Advance to next option via RIGHT (rotary clockwise in edit mode)
    menu.process(RIGHT);
    TEST_ASSERT(display.getRow(0).find("AUTO") != std::string::npos, "ItemList switched to 'AUTO' on RIGHT");

    // Commit selection
    menu.process(ENTER);
    TEST_ASSERT(MenuItem::isEditing() == false, "ItemList exited edit mode on 2nd ENTER");
    TEST_ASSERT(listCallbackIndex == 1, "Callback received index 1 ('AUTO')");

    // Re-enter edit mode and advance via UP
    menu.process(ENTER);
    menu.process(UP);  // "BOOST" (index 2)
    TEST_ASSERT(display.getRow(0).find("BOOST") != std::string::npos, "ItemList switched to 'BOOST' on UP");

    // Go back via LEFT (rotary counter-clockwise in edit mode)
    menu.process(LEFT);
    TEST_ASSERT(display.getRow(0).find("AUTO") != std::string::npos, "ItemList switched back to 'AUTO' on LEFT");

    // Discard with BACK
    menu.process(BACK);  // Discard
    TEST_ASSERT(MenuItem::isEditing() == false, "ItemList exited edit mode on BACK (ESC)");
    TEST_ASSERT(display.getRow(0).find("AUTO") != std::string::npos, "ItemList reverted back to 'AUTO'");
}

static bool boolCallbackState = false;
void onBoolTest(const bool state) {
    boolCallbackState = state;
}

void test_bool_item() {
    std::cout << "\n[TEST] test_bool_item\n";
    MockCharacterDisplayAdapter display(20, 4, false);
    CharacterDisplayRenderer renderer(&display, 20, 4);
    LcdMenu menu(renderer);

    bool pump = false;
    boolCallbackState = false;
    MenuScreen mainScreen({
        ITEM_BOOL("Pump", pump, "ON", "OFF", onBoolTest),
    });

    renderer.begin();
    menu.setScreen(&mainScreen);

    TEST_ASSERT(display.getRow(0).find("OFF") != std::string::npos, "Initial bool item is 'OFF'");

    // Enter edit mode
    menu.process(ENTER);
    TEST_ASSERT(MenuItem::isEditing() == true, "ItemBool entered edit mode on 1st ENTER");

    // Toggle via RIGHT (rotary knob in edit mode)
    menu.process(RIGHT);
    TEST_ASSERT(display.getRow(0).find("ON") != std::string::npos, "ItemBool switched to 'ON' on RIGHT");

    // Toggle via LEFT
    menu.process(LEFT);
    TEST_ASSERT(display.getRow(0).find("OFF") != std::string::npos, "ItemBool switched to 'OFF' on LEFT");

    // Toggle back to ON and commit on 2nd ENTER
    menu.process(RIGHT);
    menu.process(ENTER);
    TEST_ASSERT(MenuItem::isEditing() == false, "ItemBool exited edit mode on 2nd ENTER");
    TEST_ASSERT(boolCallbackState == true, "Callback received true");
}

void test_item_value_auto_scroll() {
    std::cout << "\n[TEST] test_item_value_auto_scroll\n";
    MockCharacterDisplayAdapter display(20, 4, false);
    CharacterDisplayRenderer renderer(&display, 20, 4);
    LcdMenu menu(renderer);

    char longStationName[64] = "Station-Alpha-Central-Hub-01";
    MenuScreen mainScreen({
        ITEM_VALUE("Tag", longStationName, "%s", 10),
    });

    renderer.begin();
    menu.setScreen(&mainScreen);

    // Initial draw: row 0 starts with 'Station'
    std::string initialRow = display.getRow(0);
    TEST_ASSERT(initialRow.find("Station") != std::string::npos, "Initial draw displays start of long value");

    // Advance ticks to trigger marquee scroll past initial hold
    for (int i = 0; i < 20; i++) {
        usleep(20000);  // 20ms > 10ms scrollSpeedMs
        menu.poll(10);
    }

    std::string scrolledRow = display.getRow(0);
    TEST_ASSERT(scrolledRow != initialRow, "Row updated during marquee auto-scroll");
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Running LcdMenu Linux Unit Tests\n";
    std::cout << "========================================\n";

    test_menu_initialization_and_rendering();
    test_cursor_navigation_down_up();
    test_toggle_item();
    test_range_item();
    test_range_item_cancel_discards_value();
    test_list_item();
    test_bool_item();
    test_item_value_auto_scroll();
    test_submenu_and_back();
    test_input_item_commit_and_cancel();
    test_mock_file_dump();
    test_rotary_counter_wrap_around();

    std::cout << "\n========================================\n";
    std::cout << "ALL UNIT TESTS PASSED SUCCESSFULLY!\n";
    std::cout << "========================================\n";
    return 0;
}
