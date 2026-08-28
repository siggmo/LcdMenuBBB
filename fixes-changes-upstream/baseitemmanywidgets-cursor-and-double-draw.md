Incorrect Cursor Column Calculation and Duplicate drawItem in BaseItemManyWidgets

  • Upstream Origin: In BaseItemManyWidgets::draw(renderer):
      • renderer->drawItem() was called inside the widget iteration loop to sample renderer->getCursorCol(), and then called a second time after the loop.
      • cursorCol was computed as endCol - (1 + cursorOffset).
  • The Problem:
      • Double draw calls caused redundant screen drawing operations.
      • On character displays, the second drawItem call moved the cursor to the end of the row / indicator column (col 19), desynchronizing hardware blinkers.
      • The endCol - 1 formula placed the editing cursor at the last character of the value string (e.g. 'O' in "AUTO") rather than the first character ('A').
  • The Fix:
      • Directly calculated the starting column of the active widget:
        startCol = (text == NULL ? 0 : strlen(text)) + 2 + activeSegmentStart;
        cursorCol = startCol + widgets[i]->cursorOffset;
      • Removed the redundant drawItem call inside the loop, rendering cleanly in a single pass.
