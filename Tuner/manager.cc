#include "include/tuner.hh"
#include <filesystem>
#include <cstdio>

namespace fs = std::filesystem;

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

    return 0;
}
