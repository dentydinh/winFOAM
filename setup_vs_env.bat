@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
echo VC++ environment ready
cmake --version
ninja --version
cl 2>&1 | findstr /c:"Version"