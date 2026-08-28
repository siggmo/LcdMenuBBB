Pointer Reference Lifetime Bug in ItemSubMenu

  • Upstream Origin: Upstream stored MenuScreen*& screen; as a reference to a pointer variable.
  • The Problem:
      • Passing temporary or dynamically constructed screen instances caused compilation failures or dangling references if the pointer variable went out of scope.
      • Passing a standard C++ reference (MenuScreen&) was not supported.
  • The Fix:
      • Changed internal storage to MenuScreen* screen = NULL; (a direct pointer).
      • Added overloaded constructors accepting both MenuScreen* and MenuScreen& for seamless API usage.
