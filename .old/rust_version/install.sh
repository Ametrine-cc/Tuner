#!/bin/bash

# Install dependencies:
echo "Installing dependencies..."
sudo pacman -S playerctl

echo "Making directories..."
mkdir -p ~/.config/tuner

echo "Copying files..."
cp src/config.toml ~/.config/tuner/config.toml
cp src/icon.png ~/.config/tuner/icon.png

echo "Building tuner..."
cargo build --release

echo "Installing tuner..."
sudo mv target/release/tuner /usr/local/bin/tuner

echo "Installing uninstall script..."
sudo cp src/uninstall.sh ~/.config/tuner/uninstall.sh

echo "Cleaning up..."
rm -rf ~/.cargo ~/.rustup

echo "Installation complete!"
