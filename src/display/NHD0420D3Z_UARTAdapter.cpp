#include "NHD0420D3Z_UARTAdapter.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cerrno>

NHD0420D3Z_UARTAdapter::NHD0420D3Z_UARTAdapter(
    const std::string& port,
    speed_t baudRate,
    uint8_t maxCols,
    uint8_t maxRows)
    : port(port),
      baudRate(baudRate),
      maxCols(maxCols),
      maxRows(maxRows),
      fd(-1),
      backlightEnabled(true),
      blinkerEnabled(false),
      physicalCursorCol(0xFF),
      physicalCursorRow(0xFF),
      logicalCursorCol(0),
      logicalCursorRow(0) {
    shadowBuffer.resize(maxRows, std::string(maxCols, ' '));
}

NHD0420D3Z_UARTAdapter::~NHD0420D3Z_UARTAdapter() {
    closePort();
}

bool NHD0420D3Z_UARTAdapter::openPort() {
    if (fd >= 0) {
        return true;
    }

    // Open non-blocking so the Linux kernel buffers writes to UART hardware FIFO asynchronously
    fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        fprintf(stderr, "NHD0420D3Z_UARTAdapter: Failed to open %s: %s\n", port.c_str(), strerror(errno));
        return false;
    }

    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "NHD0420D3Z_UARTAdapter: tcgetattr error: %s\n", strerror(errno));
        close(fd);
        fd = -1;
        return false;
    }

    cfsetispeed(&tty, baudRate);
    cfsetospeed(&tty, baudRate);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;  // 8-bit characters
    tty.c_cflag |= (CLOCAL | CREAD);             // Ignore modem controls, enable reading
    tty.c_cflag &= ~(PARENB | PARODD);           // No parity
    tty.c_cflag &= ~CSTOPB;                      // 1 stop bit
    tty.c_cflag &= ~CRTSCTS;                     // No hardware flow control

    // Raw input & output
    tty.c_lflag = 0;  // Non-canonical, no echo
    tty.c_oflag = 0;  // Raw output
    tty.c_iflag = 0;  // Raw input

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 5;  // 0.5s read timeout

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        fprintf(stderr, "NHD0420D3Z_UARTAdapter: tcsetattr error: %s\n", strerror(errno));
        close(fd);
        fd = -1;
        return false;
    }

    return true;
}

void NHD0420D3Z_UARTAdapter::closePort() {
    if (fd >= 0) {
        tcdrain(fd);
        close(fd);
        fd = -1;
    }
}

ssize_t NHD0420D3Z_UARTAdapter::sendRaw(const void* data, size_t size) {
    if (fd < 0 || data == nullptr || size == 0) {
        return -1;
    }
    return write(fd, data, size);
}

void NHD0420D3Z_UARTAdapter::begin() {
    if (!openPort()) {
        return;
    }

    // Allow display controller to stabilize after power-on
    usleep(100000);  // 100ms

    show();
    setBacklight(true);
    clear();
}

void NHD0420D3Z_UARTAdapter::clear() {
    uint8_t cmd[2] = {0xFE, 0x51};
    sendRaw(cmd, sizeof(cmd));
    for (auto& row : shadowBuffer) {
        row.assign(maxCols, ' ');
    }
    logicalCursorCol = 0;
    logicalCursorRow = 0;
    physicalCursorCol = 0;
    physicalCursorRow = 0;
    usleep(2000);  // Screen clear takes ~1.5ms
}

void NHD0420D3Z_UARTAdapter::show() {
    uint8_t cmd[2] = {0xFE, 0x41};
    sendRaw(cmd, sizeof(cmd));
}

void NHD0420D3Z_UARTAdapter::hide() {
    uint8_t cmd[2] = {0xFE, 0x42};
    sendRaw(cmd, sizeof(cmd));
}

void NHD0420D3Z_UARTAdapter::setCursor(uint8_t col, uint8_t row) {
    uint8_t clampedCol = (col < maxCols) ? col : maxCols - 1;
    uint8_t clampedRow = (row < maxRows) ? row : maxRows - 1;

    logicalCursorCol = clampedCol;
    logicalCursorRow = clampedRow;

    if (physicalCursorCol != clampedCol || physicalCursorRow != clampedRow) {
        static const uint8_t rowOffsets[4] = {0x00, 0x40, 0x14, 0x54};
        uint8_t pos = rowOffsets[clampedRow % 4] + clampedCol;
        uint8_t cmd[3] = {0xFE, 0x45, pos};
        sendRaw(cmd, sizeof(cmd));
        physicalCursorCol = clampedCol;
        physicalCursorRow = clampedRow;
    }
}

void NHD0420D3Z_UARTAdapter::draw(uint8_t byte) {
    if (logicalCursorRow < maxRows && logicalCursorCol < maxCols) {
        char ch = static_cast<char>(byte);
        // Only transmit byte over UART if it differs from physical display content
        if (shadowBuffer[logicalCursorRow][logicalCursorCol] != ch) {
            // Position physical cursor if not already at logical target
            if (physicalCursorRow != logicalCursorRow || physicalCursorCol != logicalCursorCol) {
                static const uint8_t rowOffsets[4] = {0x00, 0x40, 0x14, 0x54};
                uint8_t pos = rowOffsets[logicalCursorRow % 4] + logicalCursorCol;
                uint8_t cmd[3] = {0xFE, 0x45, pos};
                sendRaw(cmd, sizeof(cmd));
                physicalCursorRow = logicalCursorRow;
                physicalCursorCol = logicalCursorCol;
            }
            sendRaw(&byte, 1);
            shadowBuffer[logicalCursorRow][logicalCursorCol] = ch;
            physicalCursorCol++;
            if (physicalCursorCol >= maxCols) {
                physicalCursorCol = 0;
                physicalCursorRow = (physicalCursorRow + 1) % maxRows;
            }
        }
        logicalCursorCol++;
        if (logicalCursorCol >= maxCols) {
            logicalCursorCol = 0;
            logicalCursorRow = (logicalCursorRow + 1) % maxRows;
        }
    }
}

void NHD0420D3Z_UARTAdapter::draw(const char* text) {
    if (text == nullptr) return;
    while (*text) {
        draw(static_cast<uint8_t>(*text++));
    }
}

void NHD0420D3Z_UARTAdapter::setBacklight(bool enabled) {
    backlightEnabled = enabled;
    setBrightness(enabled ? 8 : 1);
}

void NHD0420D3Z_UARTAdapter::setBrightness(uint8_t level) {
    if (level < 1) level = 1;
    if (level > 8) level = 8;
    uint8_t cmd[3] = {0xFE, 0x53, level};
    sendRaw(cmd, sizeof(cmd));
}

void NHD0420D3Z_UARTAdapter::createChar(uint8_t id, uint8_t* c) {
    if (c == nullptr) return;
    uint8_t cmd[11];
    cmd[0] = 0xFE;
    cmd[1] = 0x54;
    cmd[2] = id & 0x07;
    memcpy(cmd + 3, c, 8);
    sendRaw(cmd, sizeof(cmd));
    usleep(200);
}

void NHD0420D3Z_UARTAdapter::drawBlinker() {
    blinkerEnabled = true;
    uint8_t cmd[2] = {0xFE, 0x4B};
    sendRaw(cmd, sizeof(cmd));
}

void NHD0420D3Z_UARTAdapter::clearBlinker() {
    blinkerEnabled = false;
    uint8_t cmd[2] = {0xFE, 0x4C};
    sendRaw(cmd, sizeof(cmd));
}
