Unused Parameter Warning in WidgetList::updateValue

  • Upstream Origin: WidgetList::updateValue(const char* action) logged actions using LOG(action, ...).
  • The Problem:
      • In non-debug builds (when DEBUG is not defined), the LOG(...) macro expands to empty.
      • This left the action parameter completely unused, causing GCC/Clang with -Wall -Wextra to emit a -Wunused-parameter compiler warning.
  • The Fix:
      • Added (void)action; in WidgetList::updateValue to explicitly suppress unused parameter warnings in release builds.
