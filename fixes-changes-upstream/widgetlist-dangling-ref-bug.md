Dangling Reference Lifetime Bug in WidgetList

  • Upstream stored const std::vector<T>& values; as a const reference.
  • Passing an inline temporary vector such as ITEM_LIST("Mode", {"AUTO", "ECO"}) immediately destroyed the vector when the constructor returned, leaving a
  dangling pointer.
  • Under AVR / Arduino's flat memory model without MMU protection, accessing deallocated stack/heap memory sometimes silently read garbage without an
  immediate crash. Under Linux (with virtual memory and MMU page protection), it produced an instant Segmentation fault.
  • Changing it to std::vector<T> values (by-value copy) fixed the issue for modern C++.