#include "shutter_speed_converter.h"

// Constructor to initialize the conversion table
ShutterSpeedConverter::ShutterSpeedConverter() 
{
    shutter_to_value = 
    {
        {"1/4", 0},
        {"1/5", 1},
        {"1/6", 2},
        {"1/8", 3},
        {"1/10", 4},
        {"1/13", 5},
        {"1/15", 6},
        {"1/20", 7},
        {"1/25", 8},
        {"1/30", 9},
        {"1/40", 10},
        {"1/50", 11},
        {"1/60", 12},
        {"1/80", 13},
        {"1/100", 14},
        {"1/125", 15},
        {"1/160", 16},
        {"1/200", 17},
        {"1/250", 18},
        {"1/320", 19},
        {"1/400", 20},
        {"1/500", 21},
        {"1/640", 22},
        {"1/800", 23},
        {"1/1000", 24},
        {"1/1250", 25},
        {"1/1600", 26},
        {"1/2000", 27},
        {"1/2500", 28},
        {"1/3200", 29},
        {"1/4000", 30},
        {"1/5000", 31},
        {"1/6400", 32},
        {"1/8000", 33}
    };

    // Invert the map to create value to shutter mapping
    for (const auto& pair : shutter_to_value) 
    {
        value_to_shutter[pair.second] = pair.first;
    }
}

// Function to convert shutter speed string to value
int ShutterSpeedConverter::shutterStringToValue(const std::string& shutter) {
    std::string cleanedShutter;
    for (char c : shutter) 
    {
        if (isdigit(c) || c == '.') 
        {
            cleanedShutter += c;
        }
    }

    // Add '/' back to the string
    cleanedShutter.insert(cleanedShutter.begin() + 1, '/');
    if (shutter_to_value.find(cleanedShutter) != shutter_to_value.end()) 
    {
        return shutter_to_value[cleanedShutter];
    } 
    else 
    {
        // Handle invalid shutter speed string
        spdlog::error("Invalid shutter speed string: {}", shutter);
        return -1; // or throw an exception
    }
}

// Function to convert shutter speed value to string
std::string ShutterSpeedConverter::shutterValueToString(int value) 
{
    if (value_to_shutter.find(value) != value_to_shutter.end()) 
    {
        return value_to_shutter[value];
    } 
    else 
    {
        // Handle invalid shutter speed value
        spdlog::error("Invalid shutter speed value: {}", value);
        return ""; // or throw an exception
    }
}
