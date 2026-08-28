# LcdMenu - BeagleBone Black (Linux C++)

LcdMenu is a modular C++ library for creating menu systems on character LCDs and embedded user interfaces. This fork is ported to pure, standard modern Linux C++ (C++11/C++17) tailored for the **BeagleBone Black** (Linux 6.x / Debian) and generic POSIX systems.

---

## Features & Hardware Support

- **Pure Linux C++ Core**: No Arduino or PlatformIO dependencies; uses standard C++11 (`std::chrono`, standard STL containers, POSIX APIs).
- **Newhaven UART LCD Support**: Native driver for `NHD-0420D3Z-NSW-BBW-V3` 4x20 character displays over serial UART (`/dev/ttyS3` default at 9600 baud, 8N1).
- **BeagleBone Black Rotary & Button Input**:
  - Quadrature counter ticks read directly from Linux kernel `ti-eqep` driver via Counter Subsystem sysfs (`/sys/bus/counter/devices/counter*/count0/count`).
  - Button presses read from `gpio-keys` (`rotary_btn` / `KEY_ENTER`) via Linux `evdev`.
  - Context-aware rotation: navigation mode uses Up/Down; value editing mode uses Increment/Decrement.
  - **Short Press (< 500ms)**: Emits `ENTER` to select/toggle or confirm & save in edit mode.
  - **Long Press (>= 500ms)**: Emits `BACK` (ESC) to cancel & discard edits or return to parent menu.
- **Mock Terminal Display & File Output**: Virtual 4x20 character grid with zero-lag ANSI boxed Terminal User Interface (TUI) and continuous text file dumping for PC simulation and unit tests.
- **Interactive Keyboard Input**: Non-blocking POSIX raw terminal keyboard reader for desktop testing (`Enter` = Confirm, `Esc` = Cancel & Discard).

---

## Quick Start

### Minimal Example (`examples/simple_menu.cpp`)

```cpp
#include <ItemBack.h>
#include <ItemCommand.h>
#include <ItemSubMenu.h>
#include <ItemToggle.h>
#include <LcdMenu.h>
#include <display/MockCharacterDisplayAdapter.h>
#include <renderer/CharacterDisplayRenderer.h>
#include <iostream>

void onAction() {
    std::cout << "Action executed!\n";
}

int main() {
    // 1. Initialize display adapter and renderer (4 rows x 20 cols)
    MockCharacterDisplayAdapter display(20, 4, true);
    CharacterDisplayRenderer renderer(&display, 20, 4);
    LcdMenu menu(renderer);

    // 2. Define Submenu
    MenuScreen subScreen({
        ITEM_BACK("< Back"),
        ITEM_COMMAND("Run Action", onAction),
    });

    // 3. Define Main Screen
    MenuScreen mainScreen({
        ITEM_SUBMENU("Sub Menu", subScreen),
        ITEM_TOGGLE("Setting", "ON", "OFF"),
        ITEM_COMMAND("Trigger", onAction),
    });

    // 4. Start menu
    renderer.begin();
    menu.setScreen(&mainScreen);

    // 5. Navigate
    menu.process(DOWN);   // Move cursor down
    menu.process(ENTER);  // Toggle setting
    return 0;
}
```

---

## BeagleBone Black Hardware Setup

### Device Tree Configuration
The input adapter is designed to work out-of-the-box with standard BeagleBone Black device tree overlays:

```dts
// Rotary Encoder (ti-eqep)
&epwmss1 {
    status = "okay";
};

&eqep1 {
    status = "okay";
    pinctrl-names = "default";
    pinctrl-0 = <&powerunit_eqep1_pins>;
};

// Push Button (gpio-keys)
&{/} {
    gpio_keys {
        compatible = "gpio-keys";
        pinctrl-names = "default";
        pinctrl-0 = <&powerunit_rotary_btn_pins>;

        rotary_btn {
            label = "INC_TASTE";
            linux,code = <KEY_ENTER>;
            gpios = <&gpio1 14 GPIO_ACTIVE_LOW>;
            debounce-interval = <7>;
        };
    };
};
```

### Hardware Integration Example (`examples/bbb_rotary_nhd_demo.cpp`)

```cpp
#include <LcdMenu.h>
#include <display/NHD0420D3Z_UARTAdapter.h>
#include <input/LinuxRotaryInputAdapter.h>
#include <renderer/CharacterDisplayRenderer.h>

int main() {
    // 1. Initialize NHD-0420D3Z UART Display on /dev/ttyS3 (9600 baud)
    NHD0420D3Z_UARTAdapter lcd("/dev/ttyS3", B9600, 20, 4);
    CharacterDisplayRenderer renderer(&lcd, 20, 4);
    LcdMenu menu(renderer);

    // 2. Initialize BeagleBone Black Rotary & Button Adapter
    LinuxRotaryInputAdapter rotaryInput(&menu);

    // 3. Define Menus
    MenuScreen mainScreen({
        ITEM_BASIC("System Status"),
        ITEM_TOGGLE("Relay 1", "ON", "OFF"),
    });

    renderer.begin();
    rotaryInput.begin();
    menu.setScreen(&mainScreen);

    // 4. Main Event Loop
    while (true) {
        rotaryInput.observe();
        menu.poll();
        usleep(10000); // 10ms poll tick
    }
    return 0;
}
```

---

## Building & Testing

Native compilation on the BeagleBone Black (via SSH) or any Linux PC using standard `g++` and `make`:

```bash
# Build static library, unit tests, and all demo binaries
make all

# Run automated unit test suite
make test

# Run interactive terminal simulation (PC arrow keys & Enter)
./bin/tui_demo

# Run BeagleBone Black hardware demo
./bin/bbb_rotary_nhd_demo /dev/ttyS3
```

### Makefile Targets

| Target | Description |
|---|---|
| `make lib` | Builds static library `build/liblcdmenu.a` |
| `make test` | Compiles and runs `bin/test_linux_mock` |
| `make bbb_demo` | Compiles BeagleBone Black hardware demo `bin/bbb_rotary_nhd_demo` |
| `make tui_demo` | Compiles interactive terminal simulation `bin/tui_demo` |
| `make simple` | Compiles starter demo `bin/simple_menu` |
| `make clean` | Removes build directories and temporary files |
