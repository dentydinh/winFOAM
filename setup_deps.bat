@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
set PATH=C:\Program Files\CMake\bin;C:\ninja;%PATH%

echo ========================================
echo Installing Git...
echo ========================================
winget install Git.Git --accept-source-agreements --accept-package-agreements

echo ========================================
echo Installing Windows 10 SDK (via VS Installer)...
echo ========================================
"C:\Program Files\Microsoft Visual Studio\Installer\vs_installer.exe" modify --installPath "C:\Program Files\Microsoft Visual Studio\2022\Community" --add Microsoft.VisualStudio.Component.Windows10SDK.19041 --quiet --wait

echo ========================================
echo Setting up vcpkg...
echo ========================================
if not exist C:\vcpkg (
    git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
    cd /d C:\vcpkg
    call bootstrap-vcpkg.bat
)
cd /d C:\vcpkg
call vcpkg install vtk:x64-windows

echo ========================================
echo Cloning vendor submodules...
echo ========================================
cd /d D:\00.winFOAM\winFOAM
git submodule update --init --recursive

echo ========================================
echo ALL SETUP DONE. Now run build.bat
echo ========================================