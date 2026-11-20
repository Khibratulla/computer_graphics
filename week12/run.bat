@echo off
setlocal
cd /d "%~dp0"
cmake -S . -B build -A x64
if errorlevel 1 goto fail
cmake --build build --config Release
if errorlevel 1 goto fail
build\Release\week12.exe
if errorlevel 1 goto fail
exit /b 0
:fail
echo Build or launch failed. See the message above.
pause
exit /b 1
