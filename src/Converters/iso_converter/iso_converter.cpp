#include "iso_converter.h"

// Constructor to initialize the conversion table
ISOConverter::ISOConverter() {
    iso_to_value = {
        {"ISO AUTO", 0},
        {"ISO 80", 1},
        {"ISO 100", 2},
        {"ISO 125", 3},
        {"ISO 160", 4},
        {"ISO 200", 5},
        {"ISO 250", 6},
        {"ISO 320", 7},
        {"ISO 400", 8},
        {"ISO 500", 9},
        {"ISO 640", 10},
        {"ISO 800", 11},
        {"ISO 1,000", 12},
        {"ISO 1,250", 13},
        {"ISO 1,600", 14},
        {"ISO 2,000", 15},
        {"ISO 2,500", 16},
        {"ISO 3,200", 17},
        {"ISO 4,000", 18},
        {"ISO 5,000", 19},
        {"ISO 6,400", 20},
        {"ISO 8,000", 21},
        {"ISO 10,000", 22},
        {"ISO 12,800", 23},
        {"ISO 16,000", 24},
        {"ISO 20,000", 25},
        {"ISO 25,600", 26},
        {"ISO 32,000", 27},
        {"ISO 40,000", 28},
        {"ISO 51,200", 29},
        {"ISO 64,000", 30},
        {"ISO 80,000", 31},
        {"ISO 102,400", 32},
        {"ISO 128,000", 33},
        {"ISO 160,000", 34},
        {"ISO 204,800", 35},
        {"ISO 256,000", 36},
        {"ISO 320,000", 37},
        {"ISO 409,600", 38}
    };

    // Invert the map to create value to ISO mapping
    for (const auto& pair : iso_to_value) {
        value_to_iso[pair.second] = pair.first;
    }
}

// Function to convert ISO string to value
int ISOConverter::isoStringToValue(const std::string& iso) {
    if (iso_to_value.find(iso) != iso_to_value.end()) {
        return iso_to_value[iso];
    } else {
        // Handle invalid ISO string
        std::cerr << "Invalid ISO string: " << iso << std::endl;
        return -1; // or throw an exception
    }
}

// Function to convert ISO value to string
std::string ISOConverter::isoValueToString(int value) {
    if (value_to_iso.find(value) != value_to_iso.end()) {
        return value_to_iso[value];
    } else {
        // Handle invalid ISO value
        std::cerr << "Invalid ISO value: " << value << std::endl;
        return ""; // or throw an exception
    }
}
