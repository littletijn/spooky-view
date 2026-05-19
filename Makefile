# Top-level convenience Makefile for Spooky View.
#
# This delegates to SpookyView/makefile (MinGW-w64 / g++) for Debug builds.
# Release builds and the MSI / MSIX packaging targets require MSBuild — see
# BUILDING.md.
#
# Usage:
#   make            # Debug build via MinGW-w64
#   make debug      # Same as `make`
#   make clean      # Remove obj/ and bin/ under SpookyView/
#   make rebuild    # clean + debug
#   make help       # Show targets
#
# Requirements (MinGW-w64 path):
#   * mingw-w64 (provides g++ and windres)
#   * GNU make
#   * NuGet-restored packages/  (run `nuget restore SpookyView.sln`)
# See BUILDING.md for `choco` / `scoop` setup instructions.

PROJECT_DIR := SpookyView
MAKE_BIN    ?= $(MAKE)

.PHONY: all debug build clean rebuild help check-tools restore

all: debug

debug build:
	$(MAKE_BIN) -C $(PROJECT_DIR) build

clean:
	$(MAKE_BIN) -C $(PROJECT_DIR) clean

rebuild: clean debug

check-tools:
	@command -v g++       >/dev/null 2>&1 || { echo "ERROR: g++ not on PATH. See BUILDING.md."; exit 1; }
	@command -v windres   >/dev/null 2>&1 || { echo "ERROR: windres not on PATH. See BUILDING.md."; exit 1; }
	@command -v mingw32-make >/dev/null 2>&1 || command -v make >/dev/null 2>&1 || { echo "ERROR: make not on PATH. See BUILDING.md."; exit 1; }
	@echo "OK: g++, windres, and make are on PATH."

restore:
	@command -v nuget >/dev/null 2>&1 || { echo "ERROR: nuget CLI not on PATH. Install via 'choco install nuget.commandline' or 'scoop install nuget'."; exit 1; }
	nuget restore SpookyView.sln

help:
	@echo "Targets:"
	@echo "  make / make debug   Build Debug SpookyView.exe via MinGW-w64"
	@echo "  make clean          Remove SpookyView/obj and SpookyView/bin"
	@echo "  make rebuild        clean + debug"
	@echo "  make restore        Run 'nuget restore' to fetch packages/"
	@echo "  make check-tools    Verify g++, windres, make are on PATH"
	@echo ""
	@echo "Release / MSI / MSIX builds require MSBuild — see BUILDING.md."
