## Configuration

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
