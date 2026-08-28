#ifndef LCD_MENU_UTILS_H
#define LCD_MENU_UTILS_H

#include "lcd_menu_constants.h"
#include <chrono>
#include <algorithm>
#include <string.h>
#include <stdio.h>

template <typename T, typename L, typename H>
constexpr auto constrain(const T& v, const L& lo, const H& hi) -> typename std::common_type<T, L, H>::type {
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

inline unsigned long millis() {
    using namespace std::chrono;
    static const auto start = steady_clock::now();
    return static_cast<unsigned long>(
        duration_cast<milliseconds>(steady_clock::now() - start).count()
    );
}

inline void substring(const char* str, uint8_t start, uint8_t size, char* substr) {
    strncpy(substr, str + start, size);
    substr[size] = '\0';
}

inline void concat(const char* first, char second, const char* third, char* result) {
    size_t len1 = strlen(first);
    size_t len3 = strlen(third);

    memcpy(result, first, len1);
    result[len1] = second;
    memcpy(result + len1 + 1, third, len3);
    result[len1 + 1 + len3] = '\0';
}

inline void concat(const char* first, char second, char* result) {
    size_t len1 = strlen(first);
    memcpy(result, first, len1);
    result[len1] = second;
    result[len1 + 1] = '\0';
}

inline void concat(const char* first, const char* second, char* result) {
    strcpy(result, first);
    strcat(result, second);
}

inline void remove(char* str, uint8_t index, uint8_t count) {
    uint8_t len = strlen(str);
    if (index + count > len) {
        count = len - index;
    }
    memmove(str + index, str + index + count, len - count - index + 1);
}

#ifdef DEBUG
#include <iostream>
inline void log_msg(const char* msg) {
    std::cout << "#LOG# " << msg << "\n";
}
template <typename T>
inline void log_msg(const char* msg, const T& val) {
    std::cout << "#LOG# " << msg << "=" << val << "\n";
}
#define LOG(...) log_msg(__VA_ARGS__)
#else
#define LOG(...)  // No-op
#endif

#endif  // LCD_MENU_UTILS_H
