#!/bin/bash
# winFOAM WSL Deployment Script
# Run inside WSL Ubuntu to install OpenFOAM and dependencies

set -euo pipefail

echo "========================================"
echo "winFOAM WSL Deployment"
echo "========================================"

# Update system
echo "[1/6] Updating package lists..."
sudo apt update && sudo apt upgrade -y

# Install prerequisites
echo "[2/6] Installing prerequisites..."
sudo apt install -y \
    gnupg2 \
    lsb-release \
    wget \
    ca-certificates \
    software-properties-common \
    build-essential \
    cmake \
    ninja-build \
    git \
    python3 \
    python3-pip \
    paraview \
    libvtk9-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    freeglut3-dev

# Add OpenFOAM repository
echo "[3/6] Adding OpenFOAM repository..."
wget -O - https://dl.openfoam.org/gpg.key | sudo apt-key add -
echo "deb https://dl.openfoam.org/ubuntu $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/openfoam.list
sudo apt update

# Install OpenFOAM
echo "[4/6] Installing OpenFOAM v10..."
sudo apt install -y openfoam10

# Source OpenFOAM in bashrc
echo "[5/6] Configuring OpenFOAM environment..."
if ! grep -q "openfoam10/etc/bashrc" ~/.bashrc; then
    echo "source /opt/openfoam10/etc/bashrc" >> ~/.bashrc
fi

# Install additional tools
echo "[6/6] Installing additional CFD tools..."
pip3 install --user numpy scipy matplotlib pandas

# Verify installation
source /opt/openfoam10/etc/bashrc
echo "OpenFOAM version: $(foamVersion)"
echo "Available solvers: $(ls /opt/openfoam10/platforms/*/bin/ | grep Foam | head -5)"

echo ""
echo "========================================"
echo "WSL Deployment Complete!"
echo "========================================"
echo "Run 'source ~/.bashrc' or restart your terminal"
echo "Then test with: simpleFoam -help"