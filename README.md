# Tuner

**Tuner** is a lightweight desktop widget built in **C++** using **SDL** that displays the currently playing songs on Linux. It shows album art, song title, artist, an animated equalizer, and supports dark/light themes.

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

* **Linux** (or UNIX system)
* **gcc or clang** (to compile the code)
* **make** (automate build and install process)

More information about dependencies can be found in the [Configuration](Configuration.md) section.

Install `Tuner` with the AUR:

```bash
# Arch Linux:
yay -S tuner
```

Install `Tuner` with the from source:

```bash
# All distrobutions (only tested on artix currently)
git clone https://github.com/Ametrine-cc/Tuner.git
cd Tuner

make
sudo make install
```

---

## Configuration

Find more about configuration and dependencies in the [Configuration](Configuration.md) section.

---

### Uninstallation

```bash
Tuner --uninstall
```

### Running

```bash
Tuner
```

---

## Notes & Limitations

* Linux-only (no planned support for other OS's)
* Album art is downloaded to a temporary file and cleaned up automatically
* Text is truncated (UTF-8 safe) rather than wrapped
* No tray icon or background mode (yet)
---

## Future Improvements

Planned or possible enhancements:

* Text wrapping and scrolling titles
* DPI-aware scaling
* Tray mode / always-on-top
* MacOS support

---

## License

GPL3 [License](LICENSE)
---
