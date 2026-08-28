#pragma once

#include "BaseItemZeroWidget.h"
#include "LcdMenu.h"
#include "MenuScreen.h"
#include "renderer/GraphicalIndicatorRenderer.h"

/**
 * @class ItemSubMenu
 * @brief Represents a submenu item in a menu.
 *
 * This class extends the MenuItem class and provides functionality to navigate
 * to a different screen when the item is selected.
 */
class ItemSubMenu : public BaseItemZeroWidget {
  private:
    MenuScreen* screen = NULL;

  public:
    ItemSubMenu(const char* text, MenuScreen* screen) : BaseItemZeroWidget(text), screen(screen) {}
    ItemSubMenu(const char* text, MenuScreen& screen) : BaseItemZeroWidget(text), screen(&screen) {}

    void setScreen(MenuScreen* screen) {
        this->screen = screen;
    }
    void setScreen(MenuScreen& screen) {
        this->screen = &screen;
    }

  protected:
    void draw(MenuRenderer* renderer) override {
        renderer->drawItem(text, nullptr);
        GraphicalIndicatorRenderer* indicatorRenderer =
            static_cast<GraphicalIndicatorRenderer*>(renderer->queryExtension(GraphicalIndicatorRenderer::extensionId()));
        if (indicatorRenderer != NULL) {
            indicatorRenderer->drawSubMenuIndicator();
        }
    }

    void handleCommit(LcdMenu* menu) override {
        LOG(F("ItemSubMenu::changeScreen"), text);
        if (screen != NULL) {
            screen->setParent(menu->getScreen());
            menu->setScreen(screen);
        }
    }
};

inline MenuItem* ITEM_SUBMENU(const char* text, MenuScreen* screen) {
    return new ItemSubMenu(text, screen);
}

inline MenuItem* ITEM_SUBMENU(const char* text, MenuScreen& screen) {
    return new ItemSubMenu(text, screen);
}
