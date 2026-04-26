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
KAMAKAZI_LIB  := $(KAMAKAZI_PATH)/lib/libKamakaziLib.a

# --- Includes and Libs ---
INCLUDE := -I$(RAYLIB_DIR) -I$(KAMAKAZI_PATH)/include
LIBS    := -L$(RAYLIB_DIR) -lraylib -L$(KAMAKAZI_PATH)/lib -lKamakaziLib

# System dependencies for Raylib (Linux)
SYS_LIBS := -lGL -lm -lpthread -ldl -lrt -lX11

# --- Source Layout ---
APP_DIR := Tuner
APP_SRC := $(APP_DIR)/main.cc
TARGET  := tuner
LICENSE := LICENSE
DESKTOP := Tuner.desktop
ICON    := icon.png

# --- Install Paths ---
PREFIX     := /usr/local
BINDIR     := $(PREFIX)/bin
DESKTOPDIR := /usr/share/applications
TUNERDIR   := /usr/share/Tuner
ICONDIR    := /usr/share/Tuner
DOCDIR     := /usr/share/Tuner

# --- Targets ---
.PHONY: all install uninstall clean update

all: $(TARGET)

# 1. Fetch and build Raylib
$(RAYLIB_LIB):
	@mkdir -p $(VENDOR_DIR)
	@if [ ! -d "$(VENDOR_DIR)/raylib" ]; then \
		echo "Cloning Raylib ($(RAYLIB_BRANCH))..."; \
		git clone --depth 1 -b $(RAYLIB_BRANCH) https://github.com/raysan5/raylib.git $(VENDOR_DIR)/raylib; \
	fi
	@echo "Building Raylib..."
	$(MAKE) -C $(RAYLIB_DIR) PLATFORM=PLATFORM_DESKTOP

# 2. Fetch and build KamakaziLib from 'Updates' branch
$(KAMAKAZI_LIB):
	@mkdir -p $(VENDOR_DIR)
	@if [ ! -d "$(KAMAKAZI_PATH)" ]; then \
		echo "Cloning Kamakazi($(KAMAKAZI_BRANCH))..."; \
		git clone -b $(KAMAKAZI_BRANCH) https://github.com/Ametrine-cc/Kamakazi.git $(KAMAKAZI_PATH); \
	fi
	@echo "Building Kamakazi..."
	$(MAKE) -C $(KAMAKAZI_PATH)

# 3. Build Main Target
$(TARGET): $(APP_SRC) $(RAYLIB_LIB) $(KAMAKAZI_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $(APP_SRC) $(LIBS) $(SYS_LIBS)

# --- Utility Targets ---

# Force an update of the vendored libraries
update:
	@echo "Pulling latest changes for dependencies..."
	cd $(VENDOR_DIR)/raylib && git pull origin $(RAYLIB_BRANCH)
	cd $(KAMAKAZI_PATH) && git pull origin $(KAMAKAZI_BRANCH)
	@echo "Rebuilding..."
	$(MAKE) -C $(RAYLIB_DIR) clean && $(MAKE) -C $(RAYLIB_DIR) PLATFORM=PLATFORM_DESKTOP
	$(MAKE) -C $(KAMAKAZI_PATH) clean && $(MAKE) -C $(KAMAKAZI_PATH)
	@echo "Dependencies updated and rebuilt."

install: all
	install -Dm644 $(LICENSE)               $(DESTDIR)$(DOCDIR)/LICENSE
	install -Dm755 $(TARGET)                $(DESTDIR)$(BINDIR)/$(TARGET)
	install -Dm644 $(ICON)                  $(DESTDIR)$(ICONDIR)/icon.png
	install -Dm644 $(DESKTOP)               $(DESTDIR)$(DESKTOPDIR)/$(DESKTOP)

uninstall:
	rm -f $(DESTDIR)$(DOCDIR)/LICENSE
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f $(DESTDIR)$(DESKTOPDIR)/$(DESKTOP)
	rm -f $(DESTDIR)$(ICONDIR)/$(ICON)
	rm -rf $(DESTDIR)$(TUNERDIR)

clean:
	rm -f $(TARGET)
	@if [ -d "$(RAYLIB_DIR)" ]; then $(MAKE) -C $(RAYLIB_DIR) clean; fi
	@if [ -d "$(KAMAKAZI_PATH)" ]; then $(MAKE) -C $(KAMAKAZI_PATH) clean; fi
