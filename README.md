# Tuner

**Tuner** is a lightweight desktop widget built in **Rust** using **raylib** that displays the currently playing Spotify track on Linux. It shows album art, song title, artist, an animated equalizer, and supports dark/light themes.

The application polls Spotify metadata via `playerctl` and renders a minimal, modern UI with smooth updates.

---

## Features

* 🎵 Displays current Spotify song and artist
* 🖼️ Downloads and shows album art asynchronously
* 📊 Animated equalizer visualization
* 🌗 Dark / light theme toggle
* 🎨 Gradient background with custom font rendering
* ⚡ Efficient update loop with configurable polling interval
* 🔒 Thread-safe async album art loading

---

## Requirements

### System

* **Linux** (required for playerctl and raylib)
* **Spotify desktop client**
* **playerctl**

Install `playerctl`:

```bash
sudo apt install playerctl
# or
sudo pacman -S playerctl
# or
sudo dnf install playerctl
```

---

### Rust

* Rust **1.70+** recommended
* Cargo

Install Rust:
[https://www.rust-lang.org/tools/install](https://www.rust-lang.org/tools/install)

---

## Dependencies

Key crates used:

* `raylib` – windowing, rendering, input
* `tokio` – async runtime
* `reqwest` – HTTP requests (album art)
* `serde` + `toml` – configuration loading
* `uuid` – temporary file naming

---

## Project Structure

```text
.
├── src/
│   └── main.rs
├── config.toml
├── Cargo.toml
└── README.md
```

---

## Configuration

The application loads an optional `config.toml` from the working directory.

### Example `config.toml`

```toml
dark_mode = true
window_width = 600
window_height = 200
update_interval = 2.0
```

### Options

| Key               | Type  | Description                      |
| ----------------- | ----- | -------------------------------- |
| `dark_mode`       | bool  | Start in dark mode               |
| `window_width`    | int   | Window width in pixels           |
| `window_height`   | int   | Window height in pixels          |
| `update_interval` | float | Seconds between metadata refresh |

If the file is missing or invalid, defaults are used.

---

## Building

```bash
git clone https://github.com/Ametrine-cc/Tuner.git
cd tuner

chmod +x build.sh
./build.sh
```

The binary will be located at:

```text
target/release/tuner
```

---

## Running

```bash
tuner
```

or

```bash
./target/release/tuner
```

Make sure Spotify is running and playing music.

---

## Controls

* **Left-click theme button (bottom-right)** – Toggle dark/light mode
* Window closes normally via the window manager

---

## Notes & Limitations

* Linux-only (depends on `playerctl`)
* Spotify desktop client required
* Album art is downloaded to a temporary file and cleaned up automatically
* Text is truncated (UTF-8 safe) rather than wrapped
* No tray icon or background mode (yet)
* No support for non-Spotify players (yet)
* No support for non-Linux platforms (yet)
* The C# version works but is not actively maintained and not recommended for use.
---

## Future Improvements

Planned or possible enhancements:

* Text wrapping and scrolling titles
* DPI-aware scaling
* Tray mode / always-on-top
* Support for non-Spotify players
* Windows/macOS support (alternative metadata backend)

---

## License

MIT License

---

## Acknowledgements

* [raylib](https://www.raylib.com/)
* Spotify
* playerctl developers

---

If you want, I can also:

* Add badges (build, license)
* Write a shorter README for releases
* Generate a `LICENSE` file
* Add contribution guidelines

Just tell me.
