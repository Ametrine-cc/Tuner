#include "raylib.h"
#include "kamakazi"

bool Kamakazi_Utils::ignore_dangerLevel = false;
bool Kamakazi_Utils::show_dangerLevel = true;
bool Kamakazi_Utils::show_makeLogEntry = true;
bool Kamakazi_Utils::should_log = true;

struct ThemeColors {
    static Color background;
    static Color titleText;
    static Color subtitleText;
};

Color ThemeColors::background = {30, 30, 35, 255};
Color ThemeColors::titleText = {240, 240, 245, 255};
Color ThemeColors::subtitleText = {160, 160, 170, 255};


int main() {
    kazi_log(__FUNCTION__, "Starting Tuner");
    InitWindow(600, 200, "Tuner");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(ThemeColors::background);
        DrawText("Congrats! Raylib is running locally!", 20, 20, 20, ThemeColors::titleText);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
