#pragma once

#include <ItemWidget.h>
#include <widget/WidgetRange.h>

/**
 * @brief ItemRange class that allows a user to select a value from a range.
 *
 * This class is a specialization of the ItemWidget class, which manages a value within a specified range.
 *
 * @note This is just a wrapper around the ItemWidget class. The same functionality can be achieved
 * by using the ItemWidget class directly with a WidgetRange as the widget.
 *
 * @tparam T the type of the values in the list
 * @tparam V the type of the stored value, which should be fully compatible with `T`
 */
template <typename T, typename V = T>
class ItemRange : public ItemWidget<V> {

  public:
    virtual ~ItemRange() = default;

    ItemRange(
        const char* text,
        const V value,
        const T step,
        const T min,
        const T max,
        const char* format = "%d",
        const uint8_t cursorOffset = 0,
        const bool cycle = false,
        typename ItemWidget<V>::CallbackType callback = NULL) : ItemWidget<V>(text,
                                                                       new WidgetRange<T, V>(value, step, min, max, format, cursorOffset, cycle),
                                                                       callback) {}
};

template <typename T>
inline ItemRange<T, T>* ITEM_RANGE(
    const char* text,
    const T value,
    const T step,
    const T min,
    const T max,
    const char* format = "%d",
    const uint8_t cursorOffset = 0,
    const bool cycle = false,
    void (*callback)(const T) = NULL) {
    return new ItemRange<T, T>(text, value, step, min, max, format, cursorOffset, cycle, callback);
}

template <typename T>
inline ItemRange<T, Ref<T>>* ITEM_RANGE_REF(
    const char* text,
    T& value,
    const T step,
    const T min,
    const T max,
    const char* format = "%d",
    const uint8_t cursorOffset = 0,
    const bool cycle = false,
    void (*callback)(const Ref<T>) = NULL) {
    return new ItemRange<T, Ref<T>>(text, Ref<T>(value), step, min, max, format, cursorOffset, cycle, callback);
}