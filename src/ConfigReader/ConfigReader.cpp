#include "ConfigReader.h"

std::unordered_map<std::string, std::string> ConfigReader::readConfigFile(const std::string& filename) {
    std::unordered_map<std::string, std::string> configMap;
    std::ifstream file(filename);
    std::string line;

    if (file.is_open()) {
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string key, value;
            if (std::getline(iss, key, '=') && std::getline(iss, value)) {
                configMap[key] = value;
            }
        }
        file.close();
    } else {
        throw std::runtime_error("Could not open config file: " + filename);
    }

    return configMap;
}
