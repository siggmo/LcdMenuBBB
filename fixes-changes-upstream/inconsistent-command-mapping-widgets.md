Inconsistent Command Mapping Across Widgets

  • Upstream Origin: The original author designed WidgetRange to accept all 4 directions:
    case UP:
    case RIGHT: increment();
    case DOWN:
    case LEFT:  decrement();

  • Later, a contributor added WidgetList and WidgetBool, but only handled UP and DOWN.
  • The Problem:
      • On a 5-button D-pad keypad, users happened to press the physical UP/DOWN buttons, so it seemed to work.
      • On Rotary Encoders (which emit RIGHT for clockwise and LEFT for counter-clockwise in edit mode), WidgetRange worked, but WidgetList and WidgetBool
      broke because they rejected RIGHT and LEFT.