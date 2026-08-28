CXX ?= g++
CXXFLAGS ?= -std=c++11 -Wall -Wextra -O2 -Isrc
AR ?= ar
ARFLAGS ?= rcs

BUILD_DIR = build
BIN_DIR = bin

LIB_SRCS = \
	src/LcdMenu.cpp \
	src/MenuItem.cpp \
	src/MenuScreen.cpp \
	src/renderer/MenuRenderer.cpp \
	src/renderer/CharacterDisplayRenderer.cpp \
	src/renderer/GraphicalDisplayRenderer.cpp \
	src/display/NHD0420D3Z_UARTAdapter.cpp \
	src/display/MockCharacterDisplayAdapter.cpp \
	src/input/LinuxRotaryInputAdapter.cpp

LIB_OBJS = $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(LIB_SRCS))
LIB_TARGET = $(BUILD_DIR)/liblcdmenu.a

ALL_HEADERS = \
	src/LcdMenu.h \
	src/MenuItem.h \
	src/MenuScreen.h \
	src/ItemBack.h \
	src/ItemBool.h \
	src/ItemCommand.h \
	src/ItemInput.h \
	src/ItemInputCharset.h \
	src/ItemLabel.h \
	src/ItemList.h \
	src/ItemRange.h \
	src/ItemSubMenu.h \
	src/ItemToggle.h \
	src/ItemValue.h \
	src/ItemWidget.h \
	src/BaseItemManyWidgets.h \
	src/display/CharacterDisplayInterface.h \
	src/display/DisplayInterface.h \
	src/display/GraphicalDisplayInterface.h \
	src/display/MockCharacterDisplayAdapter.h \
	src/display/NHD0420D3Z_UARTAdapter.h \
	src/input/InputInterface.h \
	src/input/LinuxRotaryInputAdapter.h \
	src/input/PosixTerminalInputAdapter.h \
	src/renderer/CharacterDisplayRenderer.h \
	src/renderer/GraphicalDisplayRenderer.h \
	src/renderer/GraphicalMenuItem.h \
	src/renderer/MenuRenderer.h \
	src/utils/lcd_menu_utils.h \
	src/widget/BaseWidget.h \
	src/widget/BaseWidgetValue.h \
	src/widget/WidgetBool.h \
	src/widget/WidgetList.h \
	src/widget/WidgetRange.h

TEST_TARGET = $(BIN_DIR)/test_linux_mock
BBB_DEMO_TARGET = $(BIN_DIR)/bbb_rotary_nhd_demo
TUI_DEMO_TARGET = $(BIN_DIR)/tui_demo
SIMPLE_TARGET = $(BIN_DIR)/simple_menu

.PHONY: all lib test bbb_demo tui_demo simple clean run_test

all: lib test bbb_demo tui_demo simple

# Static Library Target
lib: $(LIB_TARGET)

$(LIB_TARGET): $(LIB_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(AR) $(ARFLAGS) $@ $^

# Pattern rule for compiling C++ sources (using $< ensures only the source file is compiled)
$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Explicit header dependencies for library object files
$(BUILD_DIR)/LcdMenu.o: src/LcdMenu.h src/MenuScreen.h src/renderer/MenuRenderer.h src/MenuItem.h
$(BUILD_DIR)/MenuItem.o: src/MenuItem.h src/renderer/MenuRenderer.h src/utils/lcd_menu_utils.h
$(BUILD_DIR)/MenuScreen.o: src/MenuScreen.h src/MenuItem.h
$(BUILD_DIR)/renderer/MenuRenderer.o: src/renderer/MenuRenderer.h src/display/DisplayInterface.h
$(BUILD_DIR)/renderer/CharacterDisplayRenderer.o: src/renderer/CharacterDisplayRenderer.h src/renderer/MenuRenderer.h src/display/CharacterDisplayInterface.h
$(BUILD_DIR)/renderer/GraphicalDisplayRenderer.o: src/renderer/GraphicalDisplayRenderer.h src/renderer/MenuRenderer.h src/display/GraphicalDisplayInterface.h
$(BUILD_DIR)/display/NHD0420D3Z_UARTAdapter.o: src/display/NHD0420D3Z_UARTAdapter.h src/display/CharacterDisplayInterface.h
$(BUILD_DIR)/display/MockCharacterDisplayAdapter.o: src/display/MockCharacterDisplayAdapter.h src/display/CharacterDisplayInterface.h
$(BUILD_DIR)/input/LinuxRotaryInputAdapter.o: src/input/LinuxRotaryInputAdapter.h src/input/InputInterface.h

# Executable Targets (depend on source, all headers, and static library)
test: $(TEST_TARGET) run_test

$(TEST_TARGET): test/test_linux_mock.cpp $(ALL_HEADERS) $(LIB_TARGET)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -L$(BUILD_DIR) -llcdmenu -o $@

run_test: $(TEST_TARGET)
	@echo "Running test suite..."
	@$(TEST_TARGET)

bbb_demo: $(BBB_DEMO_TARGET)

$(BBB_DEMO_TARGET): examples/bbb_rotary_nhd_demo.cpp $(ALL_HEADERS) $(LIB_TARGET)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -L$(BUILD_DIR) -llcdmenu -o $@

tui_demo: $(TUI_DEMO_TARGET)

$(TUI_DEMO_TARGET): examples/linux_tui_demo.cpp $(ALL_HEADERS) $(LIB_TARGET)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -L$(BUILD_DIR) -llcdmenu -o $@

simple: $(SIMPLE_TARGET)

$(SIMPLE_TARGET): examples/simple_menu.cpp $(ALL_HEADERS) $(LIB_TARGET)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -L$(BUILD_DIR) -llcdmenu -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) /tmp/test_lcd_dump.txt simple_menu_output.txt tui_menu_output.txt
