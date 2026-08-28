#pragma once

#include <cstdio>
#include <cstring>
#include "BaseItemZeroWidget.h"
#include "renderer/MenuRenderer.h"
#include <utils/lcd_menu_utils.h>

/**
 * @class ItemValue
 * @brief A menu item that displays a value and automatically scrolls long values.
 *
 * This class extends the BaseItemZeroWidget class and provides a menu item
 * that displays a value. The value is provided as a reference during construction
 * and is displayed using the provided format string.
 * If the formatted value string exceeds the available display space on the row,
 * it automatically scrolls horizontally like a train station LED marquee display.
 */
template <typename T>
class ItemValue : public BaseItemZeroWidget {
  private:
    T& value;
    const char* format;
    bool autoScroll;
    uint16_t scrollSpeedMs;
    char overflowChar;
    uint16_t scrollOffset = 0;
    unsigned long lastScrollTime = 0;
    uint8_t holdCounter = 4;
    char lastRenderedValue[ITEM_DRAW_BUFFER_SIZE] = {0};

  public:
    ItemValue(
        const char* text,
        T& value,
        const char* format = "%s",
        bool autoScroll = false,
        uint16_t scrollSpeedMs = 250,
        char overflowChar = '>')
        : BaseItemZeroWidget(text),
          value(value),
          format(format),
          autoScroll(autoScroll),
          scrollSpeedMs(scrollSpeedMs),
          overflowChar(overflowChar) {
        this->polling = true;
    }

  protected:
    void handleCommit(LcdMenu* /*menu*/) override {}

    void draw(MenuRenderer* renderer) override {
        char buffer[ITEM_DRAW_BUFFER_SIZE];
        snprintf(buffer, ITEM_DRAW_BUFFER_SIZE, format, value);

        // Detect if value content changed -> reset ticker to start
        if (strncmp(buffer, lastRenderedValue, ITEM_DRAW_BUFFER_SIZE) != 0) {
            strncpy(lastRenderedValue, buffer, ITEM_DRAW_BUFFER_SIZE - 1);
            lastRenderedValue[ITEM_DRAW_BUFFER_SIZE - 1] = '\0';
            scrollOffset = 0;
            holdCounter = 4;
            lastScrollTime = millis();
        }

        uint8_t effectiveCols = renderer ? renderer->getEffectiveCols() : 20;
        uint8_t prefixLen = (text != nullptr ? strlen(text) : 0) + 2;  // cursor + text + ':'
        uint8_t availWidth = (effectiveCols > prefixLen) ? (effectiveCols - prefixLen) : 0;
        size_t valLen = strlen(buffer);
        bool focused = renderer ? renderer->isFocused() : false;

        // When not scrolling (unfocused, autoScroll disabled, or value fits completely):
        if (!autoScroll || !focused || availWidth == 0 || valLen <= availWidth) {
            if (!focused) {
                scrollOffset = 0;
                holdCounter = 4;
            }
            // If value overflows available space and is not in focus, show overflow indicator
            if (valLen > availWidth && availWidth > 1) {
                char previewSlice[ITEM_DRAW_BUFFER_SIZE];
                for (uint8_t i = 0; i < availWidth - 1; i++) {
                    previewSlice[i] = buffer[i];
                }
                previewSlice[availWidth - 1] = overflowChar;
                previewSlice[availWidth] = '\0';
                renderer->drawItem(text, previewSlice);
            } else {
                renderer->drawItem(text, buffer);
            }
            return;
        }

        // Focused + autoScroll enabled + value exceeds available space -> marquee ticker
        unsigned long now = millis();
        if (now - lastScrollTime >= scrollSpeedMs) {
            lastScrollTime = now;
            if (holdCounter > 0) {
                holdCounter--;
            } else {
                uint16_t cycleLen = valLen + 3;  // 3-space gap between cycles
                scrollOffset = (scrollOffset + 1) % cycleLen;
                if (scrollOffset == 0) {
                    holdCounter = 4;  // Pause at start of loop
                }
            }
        }

        char displaySlice[ITEM_DRAW_BUFFER_SIZE];
        uint16_t cycleLen = valLen + 3;
        for (uint8_t i = 0; i < availWidth; i++) {
            uint16_t idx = (scrollOffset + i) % cycleLen;
            if (idx < valLen) {
                displaySlice[i] = buffer[idx];
            } else {
                displaySlice[i] = ' ';
            }
        }
        displaySlice[availWidth] = '\0';
        renderer->drawItem(text, displaySlice);
    }

    uint8_t measureGraphicalValueWidth(GraphicalDisplayInterface* display) const override {
        if (display == NULL) {
            return 0;
        }
        char buffer[ITEM_DRAW_BUFFER_SIZE];
        snprintf(buffer, ITEM_DRAW_BUFFER_SIZE, format, value);
        return display->getTextWidth(buffer);
    }
};

/**
 * @brief Create a new item that displays a value.
 * @note If you want to display a value that changes over time or scrolls when long,
 *       call `LcdMenu::poll` in the loop function.
 *
 * @tparam T the type of the value to display
 * @param text the text to display for the item
 * @param value the value to display
 * @param format the format string to use when displaying the value
 * @param autoScroll if true, auto-scrolls horizontally when the cursor focuses on this item and value overflows
 * @param scrollSpeedMs ticker scroll speed in milliseconds per step (default 250ms)
 * @param overflowChar indicator character shown at end of value when unfocused and truncated (default '>')
 * @return MenuItem* the created item
 */
template <typename T>
inline MenuItem* ITEM_VALUE(
    const char* text,
    T& value,
    const char* format = "%s",
    bool autoScroll = false,
    uint16_t scrollSpeedMs = 250,
    char overflowChar = '>') {
    return new ItemValue<T>(text, value, format, autoScroll, scrollSpeedMs, overflowChar);
}
