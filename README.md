# Tuner

**Tuner** is a lightweight desktop widget built in **C++** using **Raylib** that displays the currently playing songs on Linux. It shows album art, song title, artist, an animated equalizer, and supports dark/light themes.

---

## Features

* 🎵 Displays current song and artist
* 🖼️ Downloads and shows album art asynchronously
* ⚡ Efficient update loop with configurable polling interval
---

## Requirements

### System

* **Linux** (or UNIX system)
* **clang** (to compile the code)
* **make** (automate build and install process)
* **Pulseaudio or Pipewire** (to get current playing songs)

More information about dependencies can be found in the [Configuration](Configuration.adoc) section.

Install `Tuner` with the from source:

```bash
# All distrobutions (only tested on Artix/Arch and Fedora currently)
git clone https://github.com/Ametrine-cc/Tuner.git
cd Tuner

make
sudo make install

# Installed run `tuner` or use the Tuner.desktop to launch `Tuner`
```

---

### Running

```bash
tuner
```

## Configuration

Find more about configuration and dependencies in the [Configuration](Configuration.adoc) section.

### Uninstallation

```bash
tuner --uninstall
```

---

## Notes & Limitations

* Linux-only (no planned support for other OS's)
* No tray icon or background mode (yet)
* All old versions of `Tuner`, can be found in .old, this includes the C# and Rust versions at this current moment.
---

## Future Improvements

Planned or possible enhancements:

* Fix Firefox issues with album art
* Text wrapping and scrolling titles
* DPI-aware scaling
* Tray mode / always-on-top
* MacOS and UNIX/UNIX Like OS's support

---

## License

GPL3 [License](LICENSE)
---
