#include "include/tuner.hh"
#include <filesystem>
#include <cstdio>
#include <fstream>

namespace fs = std::filesystem;

void createConfig() {
    char buffer[MAX_BUFFER_SIZE];

    snprintf(buffer, MAX_BUFFER_SIZE, "createTunerDir | Creating Tuner directory: %s", (fs::path(HOME) / ".tuner").c_str());
    kazi_log(__FUNCTION__, buffer);

    try {
        fs::create_directory(fs::path(HOME) / ".tuner");
        std::ofstream(fs::path(HOME) / ".tuner" / "config");
    } catch (const fs::filesystem_error& e) {
        snprintf(buffer, MAX_BUFFER_SIZE, "%s", e.what());
        kamakazi(buffer, 1);
    }
}

void configManager(std::string configPath) {
    char buffer[MAX_BUFFER_SIZE];
    kazi_log(__FUNCTION__, configPath.c_str());

    std::ifstream configFile(configPath);
    std::string line;
    while (std::getline(configFile, line)) {
        // Trim whitespace from the beginning and end of the line
        line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);

        if (line.empty() || line[0] == '#') { // Skip empty lines and comments
            continue;
        }

        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            // Trim whitespace from key and value
            key.erase(0, key.find_first_not_of(" \t\n\r\f\v"));
            key.erase(key.find_last_not_of(" \t\n\r\f\v") + 1);
            value.erase(0, value.find_first_not_of(" \t\n\r\f\v"));
            value.erase(value.find_last_not_of(" \t\n\r\f\v") + 1);

            snprintf(buffer, MAX_BUFFER_SIZE, "%s", value.c_str());
            kazi_log(__FUNCTION__, buffer);
        } else {
            snprintf(buffer, MAX_BUFFER_SIZE, "Skipping malformed line: %s", line.c_str());
            kazi_log(__FUNCTION__, buffer);
        }
    }
    configFile.close();
}

int checkConfig() {
    char buffer[MAX_BUFFER_SIZE];
    std::string configPath = fs::path(HOME) / ".tuner" / "config";

    try {
        if (fs::exists(configPath)) {
            snprintf(buffer, MAX_BUFFER_SIZE, "configManager | Config file found: %s", configPath.c_str());
            kazi_log(__FUNCTION__, buffer);
        } else {
            throw fs::filesystem_error("Config file not found", configPath, std::error_code());
        }
    } catch (const fs::filesystem_error& e) {
        snprintf(buffer, MAX_BUFFER_SIZE, "%s", e.what());
        kamakazi(buffer, 1);

        return 1;
    }

    configManager(configPath);

    return 0;
}
