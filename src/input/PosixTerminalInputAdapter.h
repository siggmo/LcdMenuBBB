#pragma once

#include "InputInterface.h"
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>

/**
 * @class PosixTerminalInputAdapter
 * @brief Raw terminal keyboard input adapter for Linux console testing and interactive demos.
 *
 * Puts stdin in non-blocking raw mode, reads keyboard strokes and ANSI escape sequences,
 * and passes translated navigation commands (UP, DOWN, LEFT, RIGHT, ENTER, BACK, BACKSPACE)
 * to LcdMenu.
 */
class PosixTerminalInputAdapter : public InputInterface {
  private:
    struct termios origTermios;
    bool rawModeEnabled = false;
    bool quitRequested = false;

    void enableRawMode() {
        if (tcgetattr(STDIN_FILENO, &origTermios) == 0) {
            struct termios raw = origTermios;
            raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
            raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
            raw.c_cflag |= (CS8);
            raw.c_cc[VMIN] = 0;
            raw.c_cc[VTIME] = 0;
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
            rawModeEnabled = true;
        }
    }

    void disableRawMode() {
        if (rawModeEnabled) {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &origTermios);
            rawModeEnabled = false;
        }
    }

    bool readWithTimeout(char& out, int timeoutMs = 25) {
        struct pollfd pfd;
        pfd.fd = STDIN_FILENO;
        pfd.events = POLLIN;
        pfd.revents = 0;
        int res = poll(&pfd, 1, timeoutMs);
        if (res > 0 && (pfd.revents & POLLIN)) {
            return read(STDIN_FILENO, &out, 1) > 0;
        }
        return false;
    }

  public:
    PosixTerminalInputAdapter(LcdMenu* menu) : InputInterface(menu) {
        enableRawMode();
    }

    virtual ~PosixTerminalInputAdapter() {
        disableRawMode();
    }

    bool shouldQuit() const { return quitRequested; }

    void observe() override {
        if (menu == nullptr) return;

        char c;
        if (read(STDIN_FILENO, &c, 1) <= 0) {
            return;
        }

        if (c == 27) {  // ESC key or start of ANSI escape sequence
            char seq[2];
            if (!readWithTimeout(seq[0], 25)) {
                // Standalone ESC key pressed (Back / Cancel / Discard)
                menu->process(BACK);
                return;
            }
            if (!readWithTimeout(seq[1], 25)) {
                return;
            }

            if (seq[0] == '[') {
                switch (seq[1]) {
                    case 'A': menu->process(UP); break;
                    case 'B': menu->process(DOWN); break;
                    case 'C': menu->process(RIGHT); break;
                    case 'D': menu->process(LEFT); break;
                    case '3':  // Delete key (\033[3~)
                        {
                            char t;
                            if (readWithTimeout(t, 25) && t == '~') {
                                menu->process(CLEAR);
                            }
                        }
                        break;
                }
            }
        } else if (c == '\n' || c == '\r') {
            menu->process(ENTER);
        } else if (c == 127 || c == '\b') {
            menu->process(BACKSPACE);
        } else if (c == 'q' || c == 'Q') {
            quitRequested = true;
        } else {
            menu->process(static_cast<unsigned char>(c));
        }
    }
};
