@echo off
REM Build script for Windows Screen Capture & Object Tracking

echo Building ScreenCaptureTracking...

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
cmake .. -G "Visual Studio 17 2022" -A x64
if errorlevel 1 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build
cmake --build . --config Release
if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo Build successful!
echo Executable: build\bin\Release\ScreenCaptureTracking.exe
echo.

REM Copy config.ini if it doesn't exist
if not exist "bin\Release\config.ini" (
    copy ..\config.ini "bin\Release\config.ini"
)

cd ..
pause