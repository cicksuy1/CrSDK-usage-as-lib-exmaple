#ifndef ISO_CONVERTER_H
#define ISO_CONVERTER_H

#include <string>
#include <iostream>
#include <unordered_map>

/**
 * @brief The ISOConverter class converts between ISO values and strings.
 */
class ISOConverter {
private:
    // Private member variables
    std::unordered_map<std::string, int> iso_to_value;
    std::unordered_map<int, std::string> value_to_iso;

public:
    // Constructor
    ISOConverter();

    /**
     * @brief Converts an ISO string to its corresponding value.
     * @param iso The ISO string (e.g., "ISO 32000").
     * @return The corresponding value, or -1 if the input is invalid.
     */
    int isoStringToValue(const std::string& iso);

    /**
     * @brief Converts an ISO value to its corresponding string.
     * @param value The ISO value.
     * @return The corresponding string, or an empty string if the input is invalid.
     */
    std::string isoValueToString(int value);
};

#endif // ISO_CONVERTER_H
