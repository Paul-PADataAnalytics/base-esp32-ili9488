#!/usr/bin/env bash
# ==============================================================================
# ESP32 ILI9488 Game Engine & Framework Environment Setup Script
# ==============================================================================
# Sets up build dependencies, Python virtualenv/packages, PlatformIO CLI,
# serial permissions (dialout udev rules), and verifies environment setup.
# ==============================================================================

set -e

GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}======================================================================${NC}"
echo -e "${BLUE}   ESP32 ILI9488 Engine & Framework — Dev Environment Setup           ${NC}"
echo -e "${BLUE}======================================================================${NC}"

# 1. Check & Install System Packages (Linux / Ubuntu / Debian)
if command -v apt-get &> /dev/null; then
    echo -e "\n${GREEN}[1/5] Checking system dependencies via apt...${NC}"
    REQUIRED_PACKAGES="python3 python3-pip python3-venv git curl build-essential libsdl2-dev"
    MISSING_PACKAGES=""

    for pkg in $REQUIRED_PACKAGES; do
        if ! dpkg -s "$pkg" &> /dev/null; then
            MISSING_PACKAGES="$MISSING_PACKAGES $pkg"
        fi
    done

    if [ -n "$MISSING_PACKAGES" ]; then
        echo -e "${YELLOW}Installing missing packages:$MISSING_PACKAGES...${NC}"
        sudo apt-get update && sudo apt-get install -y $MISSING_PACKAGES
    else
        echo -e "${GREEN}All required apt system packages are installed.${NC}"
    fi
else
    echo -e "${YELLOW}[1/5] Non-Debian system detected. Please ensure python3, pip, venv, git, build-essential, and libsdl2-dev are installed.${NC}"
fi

# 2. Check / Install PlatformIO CLI
echo -e "\n${GREEN}[2/5] Checking PlatformIO CLI installation...${NC}"
if command -v pio &> /dev/null || [ -f "$HOME/.local/bin/pio" ]; then
    echo -e "${GREEN}PlatformIO CLI is already installed.${NC}"
else
    echo -e "${YELLOW}Installing PlatformIO Core CLI...${NC}"
    curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py -o /tmp/get-platformio.py
    python3 /tmp/get-platformio.py
    rm -f /tmp/get-platformio.py

    # Add PlatformIO to PATH in current shell if needed
    export PATH="$HOME/.local/bin:$PATH"
fi

# Ensure pio command is available in local scope
PIO_BIN="pio"
if [ -f "$HOME/.local/bin/pio" ]; then
    PIO_BIN="$HOME/.local/bin/pio"
fi

# 3. Setup Python Virtual Environment for Tools (TTF font converter, etc.)
echo -e "\n${GREEN}[3/5] Setting up Python virtual environment for dev tools...${NC}"
VENV_DIR="venv"
if [ ! -d "$VENV_DIR" ]; then
    python3 -m venv "$VENV_DIR"
    echo -e "${GREEN}Created virtual environment in ./${VENV_DIR}${NC}"
fi

# Install Python requirements (freetype-py, Pillow)
echo -e "${GREEN}Installing Python tool dependencies (freetype-py, Pillow)...${NC}"
./${VENV_DIR}/bin/pip install --upgrade pip setuptools wheel
./${VENV_DIR}/bin/pip install freetype-py Pillow

# 4. Check Serial Port Permissions (dialout group)
echo -e "\n${GREEN}[4/5] Checking USB serial permissions...${NC}"
if groups | grep -q "dialout"; then
    echo -e "${GREEN}User is already in 'dialout' group for serial port access.${NC}"
else
    echo -e "${YELLOW}User is not in 'dialout' group. Adding user to 'dialout' group...${NC}"
    sudo usermod -a -G dialout "$USER"
    echo -e "${YELLOW}NOTE: You may need to log out and log back in for dialout group changes to take effect.${NC}"
fi

# 5. Initialize PlatformIO Project Libraries & Test Build
echo -e "\n${GREEN}[5/5] Initializing PlatformIO toolchains and compiling project...${NC}"
$PIO_BIN run

echo -e "\n${BLUE}======================================================================${NC}"
echo -e "${GREEN}   SETUP COMPLETE! Dev environment is ready.                          ${NC}"
echo -e "${BLUE}======================================================================${NC}"
echo -e "Useful Commands:"
echo -e "  • Compile firmware:    ${YELLOW}pio run${NC} (or ${YELLOW}$HOME/.local/bin/pio run${NC})"
echo -e "  • Upload to ESP32:     ${YELLOW}pio run --target upload${NC}"
echo -e "  • Serial Monitor:      ${YELLOW}python3 monitor.py${NC} or ${YELLOW}pio device monitor${NC}"
echo -e "  • TTF Font Converter:  ${YELLOW}./venv/bin/python tools/font_converter.py <font.ttf> <size>${NC}"
echo -e "${BLUE}======================================================================${NC}"
