#pragma once

#include "InputInterface.h"
#include <string>
#include <stdint.h>
#include <chrono>

/**
 * @class LinuxRotaryInputAdapter
 * @brief Input adapter for BeagleBone Black using Linux kernel drivers for rotary encoder and button.
 *
 * Reads quadrature counter ticks from the kernel ti-eqep driver (/sys/bus/counter/devices/counterX/count0/count)
 * and push button events from gpio-keys (/dev/input/eventX).
 *
 * Context-aware rotation:
 * - When browsing menu: Clockwise = DOWN, Counter-Clockwise = UP.
 * - When editing item value: Clockwise = RIGHT (increment), Counter-Clockwise = LEFT (decrement).
 *
 * Rotary push button (KEY_ENTER):
 * - Short press (< longPressMs, default 500ms): emits ENTER.
 * - Long press (>= longPressMs): emits BACK (cancel/discard or exit submenu).
 * Boot button (KEY_PROG1) is explicitly ignored.
 */
class LinuxRotaryInputAdapter : public InputInterface {
  private:
    std::string counterPath;
    std::string evdevPath;
    int counterFd;
    int evdevFd;
    int64_t lastCount;
    int countsPerStep;
    bool reverseDirection;
    int accumulatedCounts;
    bool initialized;

    int longPressMs;
    bool buttonPressed;
    bool longPressTriggered;
    std::chrono::steady_clock::time_point pressStartTime;

    bool autoDetectCounterPath();
    bool autoDetectEvdevPath();
    bool openCounter();
    bool openEvdev();
    void closeDescriptors();

    void processEncoder();
    void processEvents();

  public:
    /**
     * @brief Constructor for LinuxRotaryInputAdapter.
     * @param menu Pointer to LcdMenu instance.
     * @param counterPath Path to counter count file (empty to auto-detect eQEP).
     * @param evdevPath Path to gpio-keys event device (empty to auto-detect gpio-keys).
     * @param countsPerStep Number of quadrature counter ticks per physical detent/step (divider, default 2).
     * @param reverseDirection Reverse rotation direction mapping if true.
     * @param longPressMs Milliseconds required to trigger a long press (BACK). Default 500ms.
     */
    LinuxRotaryInputAdapter(
        LcdMenu* menu,
        const std::string& counterPath = "",
        const std::string& evdevPath = "",
        int countsPerStep = 2,
        bool reverseDirection = false,
        int longPressMs = 500);

    virtual ~LinuxRotaryInputAdapter();

    bool begin();
    void observe() override;

    void setCountsPerStep(int steps) { countsPerStep = steps > 0 ? steps : 1; }
    void setDivider(int divider) { setCountsPerStep(divider); }
    int getCountsPerStep() const { return countsPerStep; }
    int getDivider() const { return countsPerStep; }
    void setReverseDirection(bool reverse) { reverseDirection = reverse; }
    void setLongPressMs(int ms) { longPressMs = ms > 0 ? ms : 500; }
    int getLongPressMs() const { return longPressMs; }

    bool isCounterOpen() const { return counterFd >= 0; }
    bool isEvdevOpen() const { return evdevFd >= 0; }
    const std::string& getCounterPath() const { return counterPath; }
    const std::string& getEvdevPath() const { return evdevPath; }
};
