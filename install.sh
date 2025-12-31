#!/bin/bash

# Install dependencies:
echo "Installing dependencies..."
sudo pacman -S playerctl

echo "Making directories..."
mkdir -p ~/.config/tuner

echo "Copying files..."
cp src/config.toml ~/.config/tuner/config.toml
cp src/icon.png ~/.config/tuner/icon.png

echo "Installing Rust..."
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

echo "Installing crates..."
cargo install raylib
cargo install tokio
cargo install reqwest
cargo install serde
cargo install toml
cargo install uuid

echo "Building tuner..."
cargo build --release

echo "Installing tuner..."
sudo mv target/release/tuner /usr/local/bin/tuner

echo "Cleaning up..."
rm -rf ~/.cargo ~/.rustup

echo "Installation complete!"
