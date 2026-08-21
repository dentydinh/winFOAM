@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
set PATH=C:\Program Files\CMake\bin;C:\ninja;%PATH%

echo ========================================
echo Cloning vendor submodules...
echo ========================================
cd /d D:\00.winFOAM\winFOAM
git submodule update --init --recursive

echo ========================================
echo Installing VTK via vcpkg...
echo ========================================
if not exist C:\vcpkg (
    git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
    cd /d C:\vcpkg
    call bootstrap-vcpkg.bat
)
cd /d C:\vcpkg
call vcpkg install vtk:x64-windows

echo ========================================
echo Configuring CMake (Debug)...
echo ========================================
cd /d D:\00.winFOAM\winFOAM
cmake --preset debug

echo ========================================
echo Building...
echo ========================================
cmake --build --preset build-debug

echo ========================================
echo DONE! Run: .\build\debug\bin\winFOAM.exe
echo ========================================