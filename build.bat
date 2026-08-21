@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
set PATH=C:\Program Files\CMake\bin;C:\ninja;C:\Program Files\Git\bin;%PATH%
set VCPKG_ROOT=C:\vcpkg

cd /d D:\00.winFOAM\winFOAM

echo ========================================
echo Configuring CMake (Debug) with vcpkg toolchain...
echo ========================================
cmake --preset debug -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake

if errorlevel 1 (
    echo Config failed, trying with CMAKE_PREFIX_PATH...
    cmake --preset debug -DCMAKE_PREFIX_PATH=C:\vcpkg\installed\x64-windows
)

echo ========================================
echo Building...
echo ========================================
cmake --build --preset build-debug

echo ========================================
echo DONE! Run: .\build\debug\bin\winFOAM.exe
echo ========================================