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

## Configuration

Find more about configuration and dependencies in the [Configuration](Configuration.md) section.

---

## Installation

### Using a package manager

```bash
# Arch Linux: 
yay -S tuner
# Other distributions coming soon
```

### Building from source

```bash
git clone https://github.com/Ametrine-cc/Tuner.git
cd tuner

chmod +x install.sh
./install.sh
```

### Running

```bash
tuner
```

or

```bash
./target/release/tuner
```

Make sure Spotify is running in the background or nothing will show on the window when it's opened.

---

## Acknowledgements

* [raylib](https://www.raylib.com/)
* Spotify
* playerctl developers

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

MIT [License](LICENSE.md)
---
