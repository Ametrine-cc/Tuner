#!/bin/bash

# Uninstall script for Tuner

# Remove the binary from the system path
sudo rm -f /usr/local/bin/tuner

# Remove the configuration file
rm -f ~/.config/tuner/config.toml

# Remove the log file
rm -f ~/.config/tuner/log.txt

rm -rf ~/.config/tuner

# Remove the uninstall script
sudo rm -f ~/.config/tuner/uninstall.sh

# Remove the directory

echo "Tuner has been uninstalled successfully."
