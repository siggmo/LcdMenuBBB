Hardcoded Polling Interval Clamping in LcdMenu

  • Upstream Origin: Upstream implemented LcdMenu::poll(uint16_t pollInterval = 1000) with a hardcoded clamp:
      screen->poll(&renderer, pollInterval < 100 ? 100 : pollInterval);
  • The Problem:
      • The 100ms artificial lower bound prevented high-frequency, low-latency UI polling (such as 20ms–50ms for responsive marquee tickers, telemetry animations, and sensor updates).
      • The 1000ms default update rate caused sluggish value refreshes.
  • The Fix:
      • Removed the artificial 100ms clamp in LcdMenu::poll(), allowing callers to specify any non-zero poll interval.
      • Updated default poll interval to 50ms for responsive tick rates.
