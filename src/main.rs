use raylib::prelude::*;
use serde::Deserialize;
use std::process::Command;
use std::sync::{Arc, Mutex};
use tokio::fs;

#[derive(Deserialize, Clone)]
struct Config {
    #[serde(default = "default_dark_mode")]
    dark_mode: bool,
    #[serde(default = "default_window_width")]
    window_width: i32,
    #[serde(default = "default_window_height")]
    window_height: i32,
    #[serde(default = "default_update_interval")]
    update_interval: f32,
}

fn default_dark_mode() -> bool {
    true
}
fn default_window_width() -> i32 {
    600
}
fn default_window_height() -> i32 {
    200
}
fn default_update_interval() -> f32 {
    2.0
}

impl Default for Config {
    fn default() -> Self {
        Config {
            dark_mode: true,
            window_width: 600,
            window_height: 200,
            update_interval: 2.0,
        }
    }
}

struct AppState {
    cached_song: String,
    cached_artist: String,
    cached_album_art: String,
    album_texture: Option<Texture2D>,
    last_update: f32,
    pending_texture_path: Arc<Mutex<Option<String>>>,
    is_dark_mode: bool,
    config: Config,
}

fn load_config() -> Config {
    match std::fs::read_to_string("~/.config/tuner/config.toml") {
        Ok(contents) => toml::from_str(&contents).unwrap_or_else(|e| {
            eprintln!("Failed to parse config.toml: {}", e);
            Config::default()
        }),
        Err(_) => {
            eprintln!("config.toml not found, using defaults");
            Config::default()
        }
    }
}

fn get_song_info() -> (String, String, String) {
    // Get artist
    let artist = Command::new("bash")
        .arg("-c")
        .arg("playerctl -p spotify metadata --format='{{ artist }}' 2>/dev/null || echo 'Unknown'")
        .output()
        .map(|output| String::from_utf8_lossy(&output.stdout).trim().to_string())
        .unwrap_or_else(|_| "Unknown".to_string());

    // Get title
    let title = Command::new("bash")
        .arg("-c")
        .arg("playerctl -p spotify metadata --format='{{ title }}' 2>/dev/null || echo 'No media playing'")
        .output()
        .map(|output| String::from_utf8_lossy(&output.stdout).trim().to_string())
        .unwrap_or_else(|_| "No media playing".to_string());

    // Get album art URL
    let art_url = Command::new("bash")
        .arg("-c")
        .arg("playerctl -p spotify metadata mpris:artUrl 2>/dev/null || echo ''")
        .output()
        .map(|output| String::from_utf8_lossy(&output.stdout).trim().to_string())
        .unwrap_or_else(|_| String::new());

    (artist, title, art_url)
}

async fn download_album_art(url: String, pending_path: Arc<Mutex<Option<String>>>) {
    if url.is_empty() {
        return;
    }

    match async {
        let response = reqwest::get(&url).await?;
        let image_data = response.bytes().await?;

        let temp_dir = std::env::temp_dir();
        let file_name = format!("spotify_album_{}.jpg", uuid::Uuid::new_v4());
        let temp_path = temp_dir.join(file_name);

        fs::write(&temp_path, &image_data).await?;

        Ok::<_, Box<dyn std::error::Error>>(temp_path.to_string_lossy().to_string())
    }
    .await
    {
        Ok(path) => {
            *pending_path.lock().unwrap() = Some(path);
        }
        Err(e) => {
            eprintln!("Failed to download album art: {}", e);
            *pending_path.lock().unwrap() = None;
        }
    }
}

struct ThemeColors {
    bg_color1: Color,
    bg_color2: Color,
    text_primary: Color,
    text_secondary: Color,
    placeholder_bg: Color,
    placeholder_border: Color,
    placeholder_icon: Color,
    button_bg: Color,
    button_hover: Color,
}

fn get_theme_colors(is_dark: bool) -> ThemeColors {
    if is_dark {
        ThemeColors {
            bg_color1: Color::new(30, 30, 35, 255),
            bg_color2: Color::new(15, 15, 20, 255),
            text_primary: Color::new(240, 240, 245, 255),
            text_secondary: Color::new(160, 160, 170, 255),
            placeholder_bg: Color::new(40, 40, 45, 255),
            placeholder_border: Color::new(80, 80, 85, 255),
            placeholder_icon: Color::new(100, 100, 105, 255),
            button_bg: Color::new(50, 50, 55, 255),
            button_hover: Color::new(70, 70, 75, 255),
        }
    } else {
        ThemeColors {
            bg_color1: Color::new(245, 245, 250, 255),
            bg_color2: Color::new(230, 230, 240, 255),
            text_primary: Color::new(20, 20, 25, 255),
            text_secondary: Color::new(80, 80, 90, 255),
            placeholder_bg: Color::new(220, 220, 230, 255),
            placeholder_border: Color::new(180, 180, 190, 255),
            placeholder_icon: Color::new(150, 150, 160, 255),
            button_bg: Color::new(210, 210, 220, 255),
            button_hover: Color::new(190, 190, 200, 255),
        }
    }
}

fn main() {
    let config = load_config();

    let (mut rl, thread) = raylib::init()
        .size(config.window_width, config.window_height)
        .title("Tuner")
        .build();

    rl.set_target_fps(60);

    // Load icon if it exists
    if std::path::Path::new("~/.config/tuner/icon.png").exists() {
        let icon = Image::load_image("~/.config/tuner/icon.png").expect("Failed to load icon");
        rl.set_window_icon(icon);
    }

    let mut state = AppState {
        cached_song: String::new(),
        cached_artist: String::new(),
        cached_album_art: String::new(),
        album_texture: None,
        last_update: 0.0,
        pending_texture_path: Arc::new(Mutex::new(None)),
        is_dark_mode: config.dark_mode,
        config: config.clone(),
    };

    let rt = tokio::runtime::Runtime::new().unwrap();

    while !rl.window_should_close() {
        let current_time = rl.get_time() as f32;

        // Load pending texture on main thread
        if let Some(path) = state.pending_texture_path.lock().unwrap().take() {
            // Unload previous texture if exists
            state.album_texture = None;

            match rl.load_texture(&thread, &path) {
                Ok(texture) => {
                    state.album_texture = Some(texture);
                }
                Err(e) => {
                    eprintln!("Failed to load texture: {}", e);
                }
            }

            // Clean up temp file
            let _ = std::fs::remove_file(&path);
        }

        // Update song info
        if current_time - state.last_update > state.config.update_interval {
            let (artist, title, art_url) = get_song_info();

            if state.cached_song != title || state.cached_artist != artist {
                state.cached_song = title;
                state.cached_artist = artist;

                // Download new album art if URL changed
                if state.cached_album_art != art_url && !art_url.is_empty() {
                    state.cached_album_art = art_url.clone();
                    let pending_clone = Arc::clone(&state.pending_texture_path);
                    rt.spawn(async move {
                        download_album_art(art_url, pending_clone).await;
                    });
                }
            }

            state.last_update = current_time;
        }

        let mut d = rl.begin_drawing(&thread);

        let colors = get_theme_colors(state.is_dark_mode);

        // Background gradient
        d.clear_background(Color::new(18, 18, 18, 255));
        d.draw_rectangle_gradient_v(
            0,
            0,
            state.config.window_width,
            state.config.window_height,
            colors.bg_color1,
            colors.bg_color2,
        );

        let left_margin = 20;
        let album_size = 160;

        // Draw album art with shadow
        if let Some(ref texture) = state.album_texture {
            // Shadow
            d.draw_rectangle(
                left_margin + 3,
                23,
                album_size,
                album_size,
                Color::new(0, 0, 0, 100),
            );

            // Album art
            let scale = album_size as f32 / texture.width as f32;
            d.draw_texture_ex(
                texture,
                Vector2::new(left_margin as f32, 20.0),
                0.0,
                scale,
                Color::WHITE,
            );

            // Border
            d.draw_rectangle_lines(
                left_margin,
                20,
                album_size,
                album_size,
                Color::new(255, 255, 255, 30),
            );
        } else {
            // Placeholder album art
            d.draw_rectangle(
                left_margin,
                20,
                album_size,
                album_size,
                colors.placeholder_bg,
            );
            d.draw_rectangle_lines(
                left_margin,
                20,
                album_size,
                album_size,
                colors.placeholder_border,
            );
            d.draw_text("♪", left_margin + 60, 80, 60, colors.placeholder_icon);
        }

        let text_x = left_margin + album_size + 25;

        // Draw song title
        let display_title = if state.cached_song.len() > 30 {
            format!("{}...", &state.cached_song[..27])
        } else {
            state.cached_song.clone()
        };
        d.draw_text(display_title.as_str(), text_x, 50, 28, colors.text_primary);

        // Draw artist
        let display_artist = if state.cached_artist.len() > 35 {
            format!("{}...", &state.cached_artist[..32])
        } else {
            state.cached_artist.clone()
        };
        d.draw_text(
            display_artist.as_str(),
            text_x,
            85,
            20,
            colors.text_secondary,
        );

        // Animated equalizer bars
        for i in 0..5 {
            let bar_height = 10.0 + (current_time * 3.0 + i as f32 * 0.5).sin().abs() * 20.0;
            d.draw_rectangle(
                text_x + i * 8,
                (155.0 - bar_height) as i32,
                5,
                bar_height as i32,
                Color::new(100, 200, 100, 255),
            );
        }

        // Theme toggle button in bottom right
        let button_size = 40;
        let button_x = state.config.window_width - button_size - 15;
        let button_y = state.config.window_height - button_size - 15;

        let mouse_pos = d.get_mouse_position();
        let is_hovering = mouse_pos.x >= button_x as f32
            && mouse_pos.x <= (button_x + button_size) as f32
            && mouse_pos.y >= button_y as f32
            && mouse_pos.y <= (button_y + button_size) as f32;

        let current_button_color = if is_hovering {
            colors.button_hover
        } else {
            colors.button_bg
        };

        // Draw button background
        d.draw_rectangle(
            button_x,
            button_y,
            button_size,
            button_size,
            current_button_color,
        );
        d.draw_rectangle_lines(
            button_x,
            button_y,
            button_size,
            button_size,
            if state.is_dark_mode {
                Color::new(100, 100, 110, 255)
            } else {
                Color::new(160, 160, 170, 255)
            },
        );

        // Draw icon (sun or moon)
        let icon_center_x = button_x + button_size / 2;
        let icon_center_y = button_y + button_size / 2;

        if state.is_dark_mode {
            // Draw sun icon
            d.draw_circle(
                icon_center_x,
                icon_center_y,
                8.0,
                Color::new(255, 220, 100, 255),
            );
            for i in 0..8 {
                let angle = (i * 45) as f32 * std::f32::consts::PI / 180.0;
                let x1 = icon_center_x as f32 + angle.cos() * 10.0;
                let y1 = icon_center_y as f32 + angle.sin() * 10.0;
                let x2 = icon_center_x as f32 + angle.cos() * 14.0;
                let y2 = icon_center_y as f32 + angle.sin() * 14.0;
                d.draw_line_ex(
                    Vector2::new(x1, y1),
                    Vector2::new(x2, y2),
                    2.0,
                    Color::new(255, 220, 100, 255),
                );
            }
        } else {
            // Draw moon icon
            d.draw_circle(
                icon_center_x - 2,
                icon_center_y - 2,
                10.0,
                Color::new(100, 120, 180, 255),
            );
            d.draw_circle(icon_center_x + 3, icon_center_y - 2, 10.0, colors.bg_color1);
        }

        if is_hovering && d.is_mouse_button_pressed(MouseButton::MOUSE_BUTTON_LEFT) {
            state.is_dark_mode = !state.is_dark_mode;
            println!("TOGGLED: {}", state.is_dark_mode);
        }
    }
}
