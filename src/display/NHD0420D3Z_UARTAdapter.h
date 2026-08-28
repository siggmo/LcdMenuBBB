#pragma once

#include "CharacterDisplayInterface.h"
#include <string>
#include <vector>
#include <stdint.h>
#include <termios.h>

/**
 * @class NHD0420D3Z_UARTAdapter
 * @brief Adapter for Newhaven NHD-0420D3Z-NSW-BBW-V3 4x20 character LCD over UART on Linux.
 *
 * Implements CharacterDisplayInterface using POSIX serial port communications (termios).
 * Supports configurable serial port paths (default: "/dev/ttyS3"), baud rates (default: 9600 baud, 8N1),
 * differential shadow buffering to eliminate redundant serial writes, custom character creation,
 * backlight brightness control, and cursor positioning.
 */
class NHD0420D3Z_UARTAdapter : public CharacterDisplayInterface {
  private:
    std::string port;
    speed_t baudRate;
    uint8_t maxCols;
    uint8_t maxRows;
    int fd;
    bool backlightEnabled;
    bool blinkerEnabled;

    std::vector<std::string> lineBuffer;
    std::vector<std::string> shadowBuffer;
    uint8_t logicalCursorCol;
    uint8_t logicalCursorRow;

    bool openPort();
    void closePort();
    ssize_t sendRaw(const void* data, size_t size);
    void flushRow(uint8_t r);

  public:
    /**
     * @brief Constructor for NHD0420D3Z_UARTAdapter.
     * @param port Serial port device path (e.g. "/dev/ttyS3", "/dev/ttyO3", "/dev/ttyUSB0").
     * @param baudRate Baud rate constant (e.g. B9600, B19200, B115200). Default is B9600.
     * @param maxCols Number of display columns (default 20).
     * @param maxRows Number of display rows (default 4).
     */
    NHD0420D3Z_UARTAdapter(
        const std::string& port = "/dev/ttyS3",
        speed_t baudRate = B9600,
        uint8_t maxCols = 20,
        uint8_t maxRows = 4);

    virtual ~NHD0420D3Z_UARTAdapter();

    void begin() override;
    void clear() override;
    void show() override;
    void hide() override;
    void draw(uint8_t byte) override;
    void draw(const char* text) override;
    void setCursor(uint8_t col, uint8_t row) override;
    void setBacklight(bool enabled) override;
    void setBrightness(uint8_t level);

    void createChar(uint8_t id, uint8_t* c) override;
    void drawBlinker() override;
    void clearBlinker() override;

    bool isOpen() const { return fd >= 0; }
    int getFd() const { return fd; }
    const std::string& getPort() const { return port; }
};
