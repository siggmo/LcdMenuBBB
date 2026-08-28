Repeated display->show() on Input Resets Hardware Blinker

  • Upstream Origin: LcdMenu::process(cmd) calls renderer.restartTimer(), which calls display->show() on every single processed input event / rotary tick.
  • The Problem:
      • On HD44780 / Newhaven character LCDs over UART, the Display ON command (0xFE 0x41) writes to the LCD display control register with cursor and blink bits cleared (0).
      • Calling display->show() unconditionally on every rotary tick / keypress repeatedly turned OFF the hardware blinking cursor while editing widgets.
  • The Fix:
      • In NHD0420D3Z_UARTAdapter, tracked displayVisible state so display->show() only sends 0xFE 0x41 when transitioning from hidden to visible.
      • Automatically re-applied drawBlinker() if the display was re-enabled while in edit mode.
