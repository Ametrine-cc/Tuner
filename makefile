CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

# --- Branch Settings ---
KAMAKAZI_BRANCH := Updates
RAYLIB_BRANCH   := master

# --- Paths ---
VENDOR_DIR    := vendor
RAYLIB_DIR    := $(VENDOR_DIR)/raylib/src
RAYLIB_LIB    := $(RAYLIB_DIR)/libraylib.a
KAMAKAZI_PATH := $(VENDOR_DIR)/kamakazi
KAMAKAZI_LIB  := $(KAMAKAZI_PATH)/libKamakaziLib.a

# --- Includes and Libs ---
INCLUDE  := -I$(RAYLIB_DIR) -I$(KAMAKAZI_PATH)/src/KamakaziLib \
            $(shell pkg-config --cflags dbus-1)

LIBS     := -L$(RAYLIB_DIR) -lraylib \
            -L$(KAMAKAZI_PATH) -lKamakaziLib \
            -lpulse \
            $(shell pkg-config --libs dbus-1)

# System dependencies for Raylib (Linux)
SYS_LIBS := -lGL -lm -lpthread -ldl -lrt -lX11

# --- Source Layout ---
APP_DIR := Tuner
APP_SRC := $(APP_DIR)/main.cc
TARGET  := tuner
LICENSE := LICENSE
DESKTOP := Tuner.desktop
ICON    := icon.png
FONTS   := resources/Roboto-Black.ttf resources/Roboto-Italic.ttf

# --- Install Paths ---
PREFIX     := /usr/local
BINDIR     := $(PREFIX)/bin
DESKTOPDIR := /usr/share/applications
TUNERDIR   := /usr/share/Tuner
ICONDIR    := /usr/share/Tuner
DOCDIR     := /usr/share/Tuner
FONTDIR    := /usr/share/Tuner/resources

# --- Targets ---
.PHONY: all install uninstall clean update check-deps

all: check-deps $(TARGET)

# 0. Check system dependencies
check-deps:
	@pkg-config --exists libpulse  || (echo "ERROR: libpulse not found. Install pulseaudio or pipewire-pulse." && exit 1)
	@pkg-config --exists dbus-1    || (echo "ERROR: dbus-1 not found. Install libdbus." && exit 1)
	@echo "Dependencies OK."

# 1. Fetch and build Raylib
$(RAYLIB_LIB):
	@mkdir -p $(VENDOR_DIR)
	@if [ ! -d "$(VENDOR_DIR)/raylib" ]; then \
		echo "Cloning Raylib ($(RAYLIB_BRANCH))..."; \
		git clone --depth 1 -b $(RAYLIB_BRANCH) https://github.com/raysan5/raylib.git $(VENDOR_DIR)/raylib; \
	fi
	@echo "Building Raylib..."
	$(MAKE) -C $(RAYLIB_DIR) PLATFORM=PLATFORM_DESKTOP

# 2. Fetch and build KamakaziLib
$(KAMAKAZI_LIB):
	@mkdir -p $(VENDOR_DIR)
	@if [ ! -d "$(KAMAKAZI_PATH)" ]; then \
		echo "Cloning Kamakazi ($(KAMAKAZI_BRANCH))..."; \
		git clone -b $(KAMAKAZI_BRANCH) https://github.com/Ametrine-cc/Kamakazi.git $(KAMAKAZI_PATH); \
	fi
	@echo "Building Kamakazi..."
	$(MAKE) -C $(KAMAKAZI_PATH)

# 3. Build Main Target
$(TARGET): $(APP_SRC) $(RAYLIB_LIB) $(KAMAKAZI_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $(APP_SRC) $(LIBS) $(SYS_LIBS)

# --- Utility Targets ---

update:
	@echo "Pulling latest changes for dependencies..."
	cd $(VENDOR_DIR)/raylib && git pull origin $(RAYLIB_BRANCH)
	cd $(KAMAKAZI_PATH) && git pull origin $(KAMAKAZI_BRANCH)
	@echo "Rebuilding..."
	$(MAKE) -C $(RAYLIB_DIR) clean && $(MAKE) -C $(RAYLIB_DIR) PLATFORM=PLATFORM_DESKTOP
	$(MAKE) -C $(KAMAKAZI_PATH) clean && $(MAKE) -C $(KAMAKAZI_PATH)
	@echo "Dependencies updated and rebuilt."

install: all
	# Create directories
	install -d $(DESTDIR)$(DOCDIR)
	install -d $(DESTDIR)$(BINDIR)
	install -d $(DESTDIR)$(ICONDIR)
	install -d $(DESTDIR)$(DESKTOPDIR)
	install -d $(DESTDIR)$(FONTDIR)

	# Install files
	install -m644 $(LICENSE) $(DESTDIR)$(DOCDIR)/LICENSE
	install -m755 $(TARGET)  $(DESTDIR)$(BINDIR)/$(TARGET)
	install -m644 $(ICON)    $(DESTDIR)$(ICONDIR)/icon.png
	install -m644 $(DESKTOP) $(DESTDIR)$(DESKTOPDIR)/$(DESKTOP)

	# Install fonts
	for font in $(FONTS); do \
		install -m644 $$font $(DESTDIR)$(FONTDIR)/; \
	done

uninstall:
	rm -f  $(DESTDIR)$(DOCDIR)/LICENSE
	rm -f  $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f  $(DESTDIR)$(DESKTOPDIR)/$(DESKTOP)
	rm -f  $(DESTDIR)$(ICONDIR)/icon.png
	# Remove all fonts in the resource directory
	rm -f  $(DESTDIR)$(FONTDIR)/*.ttf
	# Clean up empty directories
	rmdir  $(DESTDIR)$(FONTDIR) 2>/dev/null || true
	rmdir  $(DESTDIR)$(TUNERDIR) 2>/dev/null || true

clean:
	rm -rf $(VENDOR_DIR)
	rm -f $(TARGET)
