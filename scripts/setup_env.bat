@echo off
REM winFOAM Windows Environment Setup
REM Run as Administrator for WSL installation

echo ========================================
echo winFOAM Environment Setup
echo ========================================

REM Check Windows version
ver | findstr /r "10\.0\." >nul
if %errorlevel% neq 0 (
    echo [WARN] Windows 10/11 recommended for WSL2
)

REM Enable WSL and Virtual Machine Platform
echo [1/5] Enabling WSL and Virtual Machine Platform...
dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart >nul 2>&1
dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart >nul 2>&1

REM Set WSL2 as default
echo [2/5] Setting WSL2 as default version...
wsl --set-default-version 2

REM Install Ubuntu if not present
echo [3/5] Checking for Ubuntu WSL distro...
wsl -l -q | findstr /i "Ubuntu" >nul
if %errorlevel% neq 0 (
    echo Installing Ubuntu 22.04...
    wsl --install -d Ubuntu-22.04
) else (
    echo Ubuntu already installed.
)

REM Update WSL kernel
echo [4/5] Updating WSL kernel...
wsl --update

echo [5/5] Setup complete!
echo.
echo Next steps:
echo 1. Restart your computer
echo 2. Run 'wsl_deploy.sh' inside WSL to install OpenFOAM
echo 3. Launch winFOAM and configure WSL path in Settings
echo.
pause