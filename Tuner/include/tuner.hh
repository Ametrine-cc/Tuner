// Tuner | The Media Player
// Copyright (C) 2026  Ametrine Foundation

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef TUNER_HH
#define TUNER_HH

// LIBRARIES
#include "kamakazi"
#include "raylib.h"

// DEFINES
#define DEFAULT_CONFIG_FILE_PATH ".tuner/config.toml"
#define MAX_BUFFER_SIZE 1024

#define HOME std::getenv("HOME")
#define TUNER_DIR "/usr/share/Tuner"

// GLOBALS
struct TunerConfig {
    static Color backgroundColor;
    static Color titleTextColor;
    static Color subtitleTextColor;
};

// FUNCTIONS
void createConfig();
int checkConfig();

#endif // TUNER_HH
