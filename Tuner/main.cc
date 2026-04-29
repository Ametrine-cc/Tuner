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

// Libraries
#include <pulse/pulseaudio.h>
#include <dbus-1.0/dbus/dbus.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <string>

#include "include/tuner.hh"
#include "raylib.h"

static bool ButtonClicked(Rectangle rect);
static bool DrawButton(Font font, Rectangle rect, const char* label, int fontSize);

// TunerConfig

Color TunerConfig::backgroundColor   = {30,  30,  35,  255};
Color TunerConfig::titleTextColor    = {240, 240, 245, 255};
Color TunerConfig::subtitleTextColor = {160, 160, 170, 255};

// Kamakazi settings

bool Kamakazi_Utils::ignore_dangerLevel  = false;
bool Kamakazi_Utils::show_dangerLevel    = true;
bool Kamakazi_Utils::show_makeLogEntry   = false;
bool Kamakazi_Utils::should_log          = false;

// Shared State

struct AudioInfo {
    std::string appName;
    std::string streamName;
    std::string mediaTitle;
    std::string mediaArtist;
    std::string artUrl;
    std::string playerBusName;
    uint32_t    sampleRate = 0;
    int         volumePct  = 0;
    bool        ready      = false;
};

static AudioInfo         g_audio;
static std::mutex        g_mutex;
static std::atomic<bool> g_running { true };

// MPRIS D-Bus bs

static std::string dict_find_string(DBusMessageIter* dict, const char* want_key) {
    while (dbus_message_iter_get_arg_type(dict) == DBUS_TYPE_DICT_ENTRY) {
        DBusMessageIter entry, value;
        dbus_message_iter_recurse(dict, &entry);

        const char* key = nullptr;
        dbus_message_iter_get_basic(&entry, &key);
        dbus_message_iter_next(&entry);          // move to value (variant)
        dbus_message_iter_recurse(&entry, &value);

        if (key && std::string(key) == want_key) {
            if (dbus_message_iter_get_arg_type(&value) == DBUS_TYPE_STRING) {
                const char* val = nullptr;
                dbus_message_iter_get_basic(&value, &val);
                return val ? val : "";
            }
            // artUrl may also be inside a variant-of-string
            if (dbus_message_iter_get_arg_type(&value) == DBUS_TYPE_VARIANT) {
                DBusMessageIter inner;
                dbus_message_iter_recurse(&value, &inner);
                if (dbus_message_iter_get_arg_type(&inner) == DBUS_TYPE_STRING) {
                    const char* val = nullptr;
                    dbus_message_iter_get_basic(&inner, &val);
                    return val ? val : "";
                }
            }
        }
        dbus_message_iter_next(dict);
    }
    return "";
}

static std::string mpris_get_art_url(DBusConnection* conn, const char* dest) {
    const char* iface = "org.mpris.MediaPlayer2.Player";
    const char* prop  = "Metadata";

    DBusMessage* msg = dbus_message_new_method_call(
        dest,
        "/org/mpris/MediaPlayer2",
        "org.freedesktop.DBus.Properties",
        "Get");
    if (!msg) return "";

    dbus_message_append_args(msg,
        DBUS_TYPE_STRING, &iface,
        DBUS_TYPE_STRING, &prop,
        DBUS_TYPE_INVALID);

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn, msg, 1000, &err);
    dbus_message_unref(msg);
    if (!reply) { dbus_error_free(&err); return ""; }

    DBusMessageIter iter, outer, dict;
    dbus_message_iter_init(reply, &iter);
    dbus_message_iter_recurse(&iter, &outer);
    dbus_message_iter_recurse(&outer, &dict);

    std::string url = dict_find_string(&dict, "mpris:artUrl");
    dbus_message_unref(reply);
    return url;
}

struct MprisData {
    std::string artUrl;
    std::string busName;
};

static MprisData query_mpris_data() {
    MprisData result;
    DBusError err;
    dbus_error_init(&err);
    DBusConnection* conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (!conn) { dbus_error_free(&err); return result; }

    DBusMessage* msg = dbus_message_new_method_call(
        "org.freedesktop.DBus", "/org/freedesktop/DBus",
        "org.freedesktop.DBus", "ListNames");

    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn, msg, 1000, &err);
    dbus_message_unref(msg);
    if (!reply) { dbus_error_free(&err); return result; }

    DBusMessageIter iter, array;
    dbus_message_iter_init(reply, &iter);
    dbus_message_iter_recurse(&iter, &array);

    while (dbus_message_iter_get_arg_type(&array) != DBUS_TYPE_INVALID) {
        const char* name = nullptr;
        dbus_message_iter_get_basic(&array, &name);

        if (name && std::string(name).find("org.mpris.MediaPlayer2.") == 0) {
            // Save the bus name so we can send commands to it later
            if (result.busName.empty()) result.busName = name;

            std::string url = mpris_get_art_url(conn, name);
            if (!url.empty()) {
                result.artUrl = url;
                result.busName = name; // Update to the player that actively has art
                break; // Take first player with art
            }
        }
        dbus_message_iter_next(&array);
    }

    dbus_message_unref(reply);
    return result;
}

// Sends PlayPause, Next, Previous to the active media player
static void send_mpris_command(const std::string& busName, const char* command) {
    if (busName.empty()) return;

    DBusError err;
    dbus_error_init(&err);
    DBusConnection* conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (!conn) { dbus_error_free(&err); return; }

    DBusMessage* msg = dbus_message_new_method_call(
        busName.c_str(),
        "/org/mpris/MediaPlayer2",
        "org.mpris.MediaPlayer2.Player",
        command);

    if (msg) {
        dbus_connection_send(conn, msg, nullptr);
        dbus_connection_flush(conn); // Ensure the message leaves immediately
        dbus_message_unref(msg);
    }
}

// PulseAudio

struct PAUserdata {
    AudioInfo* audio;
    pa_mainloop* loop;
};

static void sink_input_cb(pa_context*, const pa_sink_input_info* i, int eol, void* userdata) {
    PAUserdata* ud = static_cast<PAUserdata*>(userdata);
    if (eol) { pa_mainloop_quit(ud->loop, 0); return; }

    auto prop = [&](const char* key) -> std::string {
        const char* val = pa_proplist_gets(i->proplist, key);
        return val ? val : "";
    };

    std::string title = prop(PA_PROP_MEDIA_TITLE);
    if (title.empty()) title = prop(PA_PROP_MEDIA_NAME);
    if (title.empty()) title = i->name ? i->name : "";

    std::string artist = prop(PA_PROP_MEDIA_ARTIST);
    if (artist.empty()) artist = prop("media.album_artist");
    if (artist.empty()) artist = prop(PA_PROP_APPLICATION_NAME);

    if (!title.empty()) {
        AudioInfo tmp;
        tmp.appName     = prop(PA_PROP_APPLICATION_NAME);
        tmp.streamName  = i->name ? i->name : "";
        tmp.mediaTitle  = title;
        tmp.mediaArtist = artist;
        tmp.sampleRate  = i->sample_spec.rate;
        tmp.volumePct   = (int)(pa_cvolume_avg(&i->volume) * 100 / PA_VOLUME_NORM);
        tmp.ready       = true;

        std::lock_guard<std::mutex> lock(g_mutex);
        // Preserve D-Bus data — it comes from MPRIS, updated separately
        tmp.artUrl        = g_audio.artUrl;
        tmp.playerBusName = g_audio.playerBusName;
        *ud->audio = tmp;
    }
}

static void context_state_cb(pa_context* ctx, void* userdata) {
    if (pa_context_get_state(ctx) == PA_CONTEXT_READY)
        pa_context_get_sink_input_info_list(ctx, sink_input_cb, userdata);
}

// Background Poll Thread

static void poll_thread() {
    while (g_running) {
        // PA query
        AudioInfo    local;
        pa_mainloop* ml = pa_mainloop_new();
        PAUserdata   ud = { &local, ml };

        pa_context* ctx = pa_context_new(pa_mainloop_get_api(ml), "tuner");
        pa_context_set_state_callback(ctx, context_state_cb, &ud);
        pa_context_connect(ctx, nullptr, PA_CONTEXT_NOFLAGS, nullptr);

        int ret;
        pa_mainloop_run(ml, &ret);
        pa_context_unref(ctx);
        pa_mainloop_free(ml);

        // MPRIS art URL and Bus Name query
        MprisData mpris = query_mpris_data();

        {
            std::lock_guard<std::mutex> lock(g_mutex);
            if (local.ready) g_audio = local;
            g_audio.artUrl = mpris.artUrl;
            g_audio.playerBusName = mpris.busName;
        }

        for (int i = 0; i < 20 && g_running; i++)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Art URL → File Path

// Strips file:// prefix. Returns empty string for http (not locally loadable).
static std::string art_url_to_path(const std::string& url) {
    if (url.substr(0, 7) == "file://") return url.substr(7);
    return ""; // http(s) art not supported without curl — returns no art
}

// Main

int main() {
    char buffer[MAX_BUFFER_SIZE];
    kazi_log(__FUNCTION__, "Starting Tuner");

    if (checkConfig() != 0) {
        kamakazi("Failed to load config", 2);
    }

    std::thread bg(poll_thread);

    InitWindow(600, 200, "Tuner");
    SetTargetFPS(60);

    // Fonts

    snprintf(buffer, MAX_BUFFER_SIZE, "%s/resources/Roboto-Black.ttf", TUNER_DIR);

    Font font = LoadFontEx(buffer, 28, nullptr, 0);
    if (font.texture.id == 0) {
        TraceLog(LOG_WARNING, "FONT: Failed to load Roboto-Black.ttf (28px) — falling back to default");
        font = GetFontDefault();
    } else {
        SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);
    }

    snprintf(buffer, MAX_BUFFER_SIZE, "%s/resources/Roboto-Italic.ttf", TUNER_DIR);

    Font fontSmall = LoadFontEx(buffer, 18, nullptr, 0);
    if (fontSmall.texture.id == 0) {
        TraceLog(LOG_WARNING, "FONT: Failed to load Roboto-Italic.ttf (18px) — falling back to default");
        fontSmall = GetFontDefault();
    } else {
        SetTextureFilter(fontSmall.texture, TEXTURE_FILTER_POINT);
    }

    bool isPlaying = false;

    // Album art texture

    Texture2D artTex      = {0};
    std::string loadedUrl = "";

    Rectangle artRect  = {10, 10, 180, 180};
    Rectangle playRect = {
        (float)(ctrlCentreX - playW / 2),
        (float)(ctrlCentreY - playH / 2),
        playW, playH
    };
    Rectangle backRect = {
        playRect.x - gap - sideW,
        (float)(ctrlCentreY - sideH / 2),
        sideW, sideH
    };
    Rectangle nextRect = {
        playRect.x + playW + gap,
        (float)(ctrlCentreY - sideH / 2),
        sideW, sideH
    };
    Rectangle settingsRect = {
        (float)(GetScreenWidth() - sideW - gap),
        (float)(GetScreenHeight() - sideH - (gap + 8)),
        sideW, sideH
    };

    while (!WindowShouldClose()) {
        checkConfig();
        // Snapshot shared state
        AudioInfo audio;
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            audio = g_audio;
        }

        // Media Controls using D-Bus
        if (ButtonClicked(playRect)) {
            isPlaying = !isPlaying;
            kazi_log(__FUNCTION__, "Play/Pause");
            send_mpris_command(audio.playerBusName, "PlayPause");
        }
        if (ButtonClicked(backRect)) {
            kazi_log(__FUNCTION__, "Back");
            send_mpris_command(audio.playerBusName, "Previous");
        }
        if (ButtonClicked(nextRect)) {
            kazi_log(__FUNCTION__, "Next");
            send_mpris_command(audio.playerBusName, "Next");
        }

        // Swap art texture if URL changed

        if (audio.artUrl != loadedUrl) {
            if (artTex.id > 0) UnloadTexture(artTex);
            artTex = {0};
            loadedUrl = audio.artUrl;

            std::string path = art_url_to_path(audio.artUrl);
            if (!path.empty()) {
                Image img = LoadImage(path.c_str());
                if (img.data) {
                    ImageResize(&img, (int)artRect.width, (int)artRect.height);
                    artTex = LoadTextureFromImage(img);
                    UnloadImage(img);
                }
            }
        }

        BeginDrawing();
        ClearBackground(Theme::backgroundColor);

        // Album art or placeholder

        if (artTex.id > 0) {
            DrawTexture(artTex, (int)artRect.x, (int)artRect.y, WHITE);
        } else {
            DrawRectangleRec(artRect, Theme::subtitleTextColor);
        }

        const char* title  = audio.ready && !audio.mediaTitle.empty()
                             ? audio.mediaTitle.c_str()  : "Nothing Playing";
        const char* artist = audio.ready && !audio.mediaArtist.empty()
                             ? audio.mediaArtist.c_str() : "";

        DrawTextEx(font,      title,  {210.0f, 50.0f}, 28, 1, Theme::titleTextColor);
        DrawTextEx(fontSmall, artist, {210.0f, 88.0f}, 18, 1, Theme::subtitleTextColor);

        DrawButton(font, backRect, "<<",  16);
        DrawButton(font, playRect, isPlaying ? "||" : ">", 22);
        DrawButton(font, nextRect, ">>",  16);

        EndDrawing();
    }

    if (artTex.id > 0) UnloadTexture(artTex);
    UnloadFont(font);
    UnloadFont(fontSmall);

    g_running = false;
    bg.join();

    CloseWindow();
    return 0;
}
