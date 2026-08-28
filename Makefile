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

$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Executable Targets
test: $(TEST_TARGET) run_test

$(TEST_TARGET): test/test_linux_mock.cpp $(LIB_TARGET)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -L$(BUILD_DIR) -llcdmenu -o $@

run_test: $(TEST_TARGET)
	@echo "Running test suite..."
	@$(TEST_TARGET)

bbb_demo: $(BBB_DEMO_TARGET)

$(BBB_DEMO_TARGET): examples/bbb_rotary_nhd_demo.cpp $(LIB_TARGET)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -L$(BUILD_DIR) -llcdmenu -o $@

tui_demo: $(TUI_DEMO_TARGET)

$(TUI_DEMO_TARGET): examples/linux_tui_demo.cpp $(LIB_TARGET)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -L$(BUILD_DIR) -llcdmenu -o $@

simple: $(SIMPLE_TARGET)

$(SIMPLE_TARGET): examples/simple_menu.cpp $(LIB_TARGET)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -L$(BUILD_DIR) -llcdmenu -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) /tmp/test_lcd_dump.txt simple_menu_output.txt tui_menu_output.txt
