@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
set PATH=C:\Program Files\CMake\bin;C:\ninja;C:\Program Files\Git\bin;%PATH%
cd /d C:\vcpkg
call vcpkg install vtk:x64-windows --keep-going