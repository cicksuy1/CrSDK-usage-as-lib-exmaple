#ifndef CONFIGREADER_H
#define CONFIGREADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>

/**
 * @class ConfigReader
 * @brief Utility class to read key-value pairs from a configuration file.
 */
class ConfigReader {
public:
    /**
     * @brief Reads the configuration file and returns a map of key-value pairs.
     * @param filename The name of the configuration file.
     * @return A map of key-value pairs.
     */
    static std::unordered_map<std::string, std::string> readConfigFile(const std::string& filename);
};

#endif // CONFIGREADER_H
