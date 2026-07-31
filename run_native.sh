#!/usr/bin/env bash
# ==============================================================================
# Build and Run Native Linux Simulator Script
# ==============================================================================
# Builds the native Linux target via PlatformIO and executes the simulator.
# ==============================================================================

set -e

GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}======================================================================${NC}"
echo -e "${BLUE}   Building Native Linux Desktop Simulator (desktop_linux target)... ${NC}"
echo -e "${BLUE}======================================================================${NC}"

# Find pio binary
PIO_BIN="pio"
if command -v pio &> /dev/null; then
    PIO_BIN="pio"
elif [ -f "$HOME/.local/bin/pio" ]; then
    PIO_BIN="$HOME/.local/bin/pio"
else
    echo -e "${RED}PlatformIO CLI ('pio') not found. Please run ./scripts/setup_dev_env.sh first.${NC}"
    exit 1
fi

# Build native Linux executable
$PIO_BIN run -e desktop_linux

PROGRAM_BIN=".pio/build/desktop_linux/program"

if [ ! -f "$PROGRAM_BIN" ]; then
    echo -e "${RED}Build error: Executable $PROGRAM_BIN not found!${NC}"
    exit 1
fi

echo -e "\n${GREEN}Build successful! Launching native simulator...${NC}"
echo -e "${BLUE}======================================================================${NC}\n"

# Execute the desktop simulator
exec "$PROGRAM_BIN" "$@"
