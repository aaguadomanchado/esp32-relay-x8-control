#!/bin/bash
set -e

# Define virtual environment directory
VENV_DIR=".venv"

# Create venv if it doesn't exist
if [ ! -d "$VENV_DIR" ]; then
    echo "Creating local Python virtual environment..."
    python3 -m venv $VENV_DIR
fi

# Activate venv
source $VENV_DIR/bin/activate

# Install PlatformIO if not present
if ! command -v pio &> /dev/null; then
    echo "Installing PlatformIO Core..."
    pip install -U platformio
fi

# Run upload
echo "Starting compilation and upload..."
pio run -t upload
