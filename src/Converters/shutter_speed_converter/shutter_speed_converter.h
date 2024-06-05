#ifndef SHUTTER_SPEED_CONVERTER_H
#define SHUTTER_SPEED_CONVERTER_H

#include <string>
#include <unordered_map>
#include <iostream>

/**
 * @brief The ShutterSpeedConverter class converts between shutter speed values and strings.
 */
class ShutterSpeedConverter {
private:
    // Private member variables
    std::unordered_map<std::string, int> shutter_to_value;
    std::unordered_map<int, std::string> value_to_shutter;

public:
    // Constructor
    ShutterSpeedConverter();

    /**
     * @brief Converts a shutter speed string to its corresponding value.
     * @param shutter The shutter speed string (e.g., "1/2000").
     * @return The corresponding value, or -1 if the input is invalid.
     */
    int shutterStringToValue(const std::string& shutter);

    /**
     * @brief Converts a shutter speed value to its corresponding string.
     * @param value The shutter speed value.
     * @return The corresponding string, or an empty string if the input is invalid.
     */
    std::string shutterValueToString(int value);
};

#endif // SHUTTER_SPEED_CONVERTER_H
