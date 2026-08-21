@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
set PATH=C:\Program Files\CMake\bin;C:\ninja;C:\Program Files\Git\bin;%PATH%

if exist C:\vcpkg rmdir /s /q C:\vcpkg
mkdir C:\vcpkg
cd /d C:\vcpkg
git clone https://github.com/microsoft/vcpkg.git .
call bootstrap-vcpkg.bat
call vcpkg install vtk:x64-windows