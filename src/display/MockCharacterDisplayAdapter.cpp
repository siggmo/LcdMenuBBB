#include "MockCharacterDisplayAdapter.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>

MockCharacterDisplayAdapter::MockCharacterDisplayAdapter(
    uint8_t maxCols,
    uint8_t maxRows,
    bool ansiTuiEnabled,
    const std::string& dumpFilePath)
    : maxCols(maxCols),
      maxRows(maxRows),
      cursorCol(0),
      cursorRow(0),
      backlightEnabled(true),
      blinkerEnabled(false),
      displayVisible(true),
      ansiTuiEnabled(ansiTuiEnabled),
      clearScreenOnRender(true),
      dumpFilePath(dumpFilePath) {
    buffer.resize(maxRows, std::string(maxCols, ' '));
    for (auto& slot : customChars) {
        slot.fill(0);
    }
    // Default symbol representation for custom characters (0=Up arrow, 1=Down arrow)
    customCharSymbols = {'^', 'v', '#', '$', '%', '&', '@', '!'};
}

void MockCharacterDisplayAdapter::begin() {
    clear();
}

void MockCharacterDisplayAdapter::clear() {
    for (auto& row : buffer) {
        row.assign(maxCols, ' ');
    }
    cursorCol = 0;
    cursorRow = 0;
    render();
}

void MockCharacterDisplayAdapter::show() {
    displayVisible = true;
    render();
}

void MockCharacterDisplayAdapter::hide() {
    displayVisible = false;
    render();
}

void MockCharacterDisplayAdapter::draw(uint8_t byte) {
    if (cursorRow < maxRows && cursorCol < maxCols) {
        char ch;
        if (byte < 8) {
            ch = customCharSymbols[byte];
        } else {
            ch = static_cast<char>(byte);
        }
        buffer[cursorRow][cursorCol] = ch;
        cursorCol++;
        if (cursorCol >= maxCols) {
            cursorCol = 0;
            cursorRow = (cursorRow + 1) % maxRows;
        }
    }
}

void MockCharacterDisplayAdapter::draw(const char* text) {
    if (text == nullptr) return;
    while (*text) {
        draw(static_cast<uint8_t>(*text++));
    }
    render();
}

void MockCharacterDisplayAdapter::setCursor(uint8_t col, uint8_t row) {
    cursorCol = col < maxCols ? col : maxCols - 1;
    cursorRow = row < maxRows ? row : maxRows - 1;
}

void MockCharacterDisplayAdapter::setBacklight(bool enabled) {
    backlightEnabled = enabled;
    render();
}

void MockCharacterDisplayAdapter::createChar(uint8_t id, uint8_t* c) {
    if (c == nullptr) return;
    uint8_t slot = id & 0x07;
    for (size_t i = 0; i < 8; i++) {
        customChars[slot][i] = c[i];
    }
}

void MockCharacterDisplayAdapter::drawBlinker() {
    blinkerEnabled = true;
    render();
}

void MockCharacterDisplayAdapter::clearBlinker() {
    blinkerEnabled = false;
    render();
}

void MockCharacterDisplayAdapter::setAnsiTuiEnabled(bool enabled, bool clearScreen) {
    ansiTuiEnabled = enabled;
    clearScreenOnRender = clearScreen;
}

void MockCharacterDisplayAdapter::setDumpFile(const std::string& filepath) {
    dumpFilePath = filepath;
}

void MockCharacterDisplayAdapter::setCustomCharSymbol(uint8_t id, char symbol) {
    if (id < 8) {
        customCharSymbols[id] = symbol;
    }
}

std::string MockCharacterDisplayAdapter::getRow(uint8_t row) const {
    if (row < maxRows) {
        return buffer[row];
    }
    return "";
}

std::string MockCharacterDisplayAdapter::getScreenText() const {
    std::string text;
    for (size_t r = 0; r < maxRows; r++) {
        text += buffer[r];
        if (r + 1 < maxRows) {
            text += '\n';
        }
    }
    return text;
}

std::string MockCharacterDisplayAdapter::getFormattedBox() const {
    std::ostringstream ss;
    // Top border
    ss << "+";
    for (uint8_t c = 0; c < maxCols; c++) ss << "-";
    ss << "+\n";

    // Rows
    for (uint8_t r = 0; r < maxRows; r++) {
        ss << "|";
        if (displayVisible) {
            ss << buffer[r];
        } else {
            ss << std::string(maxCols, ' ');
        }
        ss << "|\n";
    }

    // Bottom border
    ss << "+";
    for (uint8_t c = 0; c < maxCols; c++) ss << "-";
    ss << "+\n";

    return ss.str();
}

void MockCharacterDisplayAdapter::dumpToFile(const std::string& filepath) const {
    std::ofstream out(filepath);
    if (out.is_open()) {
        out << getFormattedBox();
        out << "Cursor: (" << (int)cursorCol << ", " << (int)cursorRow << ")"
            << " | Backlight: " << (backlightEnabled ? "ON" : "OFF")
            << " | Blinker: " << (blinkerEnabled ? "ON" : "OFF")
            << " | Visible: " << (displayVisible ? "YES" : "NO") << "\n";
    }
}

void MockCharacterDisplayAdapter::render() {
    if (ansiTuiEnabled) {
        if (clearScreenOnRender) {
            // ANSI clear screen & home cursor
            std::cout << "\033[2J\033[H";
        }
        std::cout << getFormattedBox();
        std::cout << "Status: Backlight=" << (backlightEnabled ? "ON" : "OFF")
                  << " | Blinker=" << (blinkerEnabled ? "ON" : "OFF")
                  << " | Cursor=[" << (int)cursorCol << "," << (int)cursorRow << "]\n";
        std::cout.flush();
    }

    if (!dumpFilePath.empty()) {
        dumpToFile(dumpFilePath);
    }
}
