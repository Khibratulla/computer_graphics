@echo off
setlocal
cd /d "%~dp0"
if not exist build\Release\week11.exe (
    cmake -S . -B build -A x64
    if errorlevel 1 exit /b 1
    cmake --build build --config Release
    if errorlevel 1 exit /b 1
)
build\Release\week11.exe
