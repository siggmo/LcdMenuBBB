#ifndef ItemToggle_H
#define ItemToggle_H

#include "LcdMenu.h"
#include "MenuItem.h"
#include "display/GraphicalDisplayInterface.h"
#include "renderer/GraphicalMenuItem.h"
#include <utils/lcd_menu_utils.h>

/**
 * @brief Item that allows user to toggle between ON/OFF states.
 *
 * ```
 * ┌────────────────────────────┐
 * │ > T E X T : O F F          │
 * └────────────────────────────┘
 * ```
 *
 * Additionally to `text` this item has ON/OFF `enabled` state.
 */
class ItemToggle : public MenuItem, public GraphicalMenuItem {
  private:
    bool enabled = false;
    const char* textOn = NULL;
    const char* textOff = NULL;
    fptrBool callback = NULL;

  public:
    /**
     * @brief Create an ItemToggle object with default values as `ON` and `OFF`.
     *
     * @param key key of the item
     * @param callback reference to callback function
     */
    ItemToggle(const char* key, fptrBool callback = NULL)
        : ItemToggle(key, false, callback) {}

    ItemToggle(const char* text, bool enabled, fptrBool callback = NULL)
        : ItemToggle(text, "ON", "OFF", callback) {
        this->enabled = enabled;
    }

    ItemToggle(const char* text, const char* textOn, const char* textOff, fptrBool callback = NULL)
        : MenuItem(text),
          textOn(textOn),
          textOff(textOff),
          callback(callback) {}

    fptrBool getCallbackInt() { return callback; }

    bool isOn() { return enabled; }

    void setIsOn(bool isOn) { this->enabled = isOn; }

    const char* getTextOn() { return this->textOn; }

    const char* getTextOff() { return this->textOff; }

    uint8_t measureGraphicalValueWidth(GraphicalDisplayInterface* display) const override {
        if (display == NULL) {
            return 0;
        }
        uint8_t toggleWidth = display->getFontHeight();
        if (toggleWidth > 4) {
            toggleWidth -= 4;
        }
        if (toggleWidth < 3) {
            toggleWidth = 3;
        }
        uint8_t onWidth = display->getTextWidth(textOn == NULL ? "" : textOn);
        uint8_t offWidth = display->getTextWidth(textOff == NULL ? "" : textOff);
        uint8_t textWidth = onWidth > offWidth ? onWidth : offWidth;
        return toggleWidth > textWidth ? toggleWidth : textWidth;
    }

    bool hasGraphicalToggle() const override { return true; }

    bool graphicalToggleState() const override { return enabled; }

    const void* queryCapability(uint8_t capabilityId) const override {
        if (capabilityId == GraphicalMenuItem::capabilityId()) {
            return static_cast<const GraphicalMenuItem*>(this);
        }
        return MenuItem::queryCapability(capabilityId);
    }

    void draw(MenuRenderer* renderer) override {
        renderer->drawItem(text, enabled ? textOn : textOff);
    };

  protected:
    bool process(LcdMenu* menu, const unsigned char command) override {
        MenuRenderer* display = menu->getRenderer();
        switch (command) {
            case ENTER:
                toggle(display);
                return true;
            default:
                return false;
        }
    };
    void toggle(MenuRenderer* renderer) {
        enabled = !enabled;
        LOG(F("ItemToggle::toggle"), enabled ? textOn : textOff);
        draw(renderer);
        if (callback != NULL) {
            callback(enabled);
        }
    }
};

#define ITEM_TOGGLE(...) (new ItemToggle(__VA_ARGS__))

#endif
