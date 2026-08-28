#pragma once

#include "CharacterDisplayInterface.h"
#include <string>
#include <vector>
#include <array>
#include <stdint.h>

/**
 * @class MockCharacterDisplayAdapter
 * @brief In-memory mock display adapter for testing, ANSI terminal UI rendering, and file output.
 *
 * Implements CharacterDisplayInterface. Simulates a 4x20 character display grid in memory,
 * provides programmatic inspection methods for unit test assertions, renders an interactive
 * box-framed Terminal User Interface (TUI), and can continuously dump the display contents
 * to a text file.
 */
class MockCharacterDisplayAdapter : public CharacterDisplayInterface {
  private:
    uint8_t maxCols;
    uint8_t maxRows;
    uint8_t cursorCol;
    uint8_t cursorRow;
    bool backlightEnabled;
    bool blinkerEnabled;
    bool displayVisible;
    bool ansiTuiEnabled;
    bool clearScreenOnRender;
    std::string dumpFilePath;

    std::vector<std::string> buffer;
    std::array<std::array<uint8_t, 8>, 8> customChars;
    std::array<char, 8> customCharSymbols;

  public:
    /**
     * @brief Constructor for MockCharacterDisplayAdapter.
     * @param maxCols Number of columns (default 20).
     * @param maxRows Number of rows (default 4).
     * @param ansiTuiEnabled If true, prints ANSI boxed frame to stdout on updates.
     * @param dumpFilePath Optional path to write/dump screen text on updates.
     */
    MockCharacterDisplayAdapter(
        uint8_t maxCols = 20,
        uint8_t maxRows = 4,
        bool ansiTuiEnabled = false,
        const std::string& dumpFilePath = "");

    virtual ~MockCharacterDisplayAdapter() = default;

    void begin() override;
    void clear() override;
    void show() override;
    void hide() override;
    void draw(uint8_t byte) override;
    void draw(const char* text) override;
    void setCursor(uint8_t col, uint8_t row) override;
    void setBacklight(bool enabled) override;

    void createChar(uint8_t id, uint8_t* c) override;
    void drawBlinker() override;
    void clearBlinker() override;

    // Configuration
    void setAnsiTuiEnabled(bool enabled, bool clearScreen = true);
    void setDumpFile(const std::string& filepath);
    void setCustomCharSymbol(uint8_t id, char symbol);
    void render();

    // Inspection & Testing API
    std::string getRow(uint8_t row) const;
    std::string getScreenText() const;
    std::string getFormattedBox() const;
    void dumpToFile(const std::string& filepath) const;

    uint8_t getCursorCol() const { return cursorCol; }
    uint8_t getCursorRow() const { return cursorRow; }
    bool isBacklightEnabled() const { return backlightEnabled; }
    bool isBlinkerEnabled() const { return blinkerEnabled; }
    bool isDisplayVisible() const { return displayVisible; }
};
