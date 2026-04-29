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

// THEME
struct Theme : TunerConfig{};

// BUTTONS

static bool ButtonClicked(Rectangle rect) {
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
           CheckCollisionPointRec(GetMousePosition(), rect);
}

static bool DrawButton(Font font, Rectangle rect, const char* label, int fontSize) {
    bool clicked = ButtonClicked(rect);
    bool hovered = CheckCollisionPointRec(GetMousePosition(), rect);
    Color fill   = hovered ? Theme::subtitleTextColor : (Color){60, 60, 68, 255};
    DrawRectangleRec(rect, fill);

    int textW = MeasureText(label, fontSize);
    int textX = (int)rect.x + ((int)rect.width  - textW) / 2;
    int textY = (int)rect.y + ((int)rect.height - fontSize) / 2;
    DrawTextEx(font, label, {(float)textX, (float)textY}, (float)fontSize, 1, Theme::titleTextColor);

    return clicked;
}

const float playW = 54, playH = 54;
const float sideW = 42, sideH = 42;
const float gap   = 16;

const int ctrlCentreX = (200 + 590) / 2;
const int ctrlCentreY = 155;

// FUNCTIONS
void createConfig();
int settings();
int checkConfig();

#endif // TUNER_HH
