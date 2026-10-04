@echo off
echo Building Delivery Tracking System...

if not exist build mkdir build
cd build

cmake .. -G "MinGW Makefiles" 2>nul || cmake .. -G "Visual Studio 17 2022" 2>nul || cmake ..
cmake --build . --config Release

if %ERRORLEVEL% == 0 (
    echo.
    echo Build successful!
    echo Run:  build\delivery_tracker.exe
) else (
    echo.
    echo Build failed. Check errors above.
)
cd ..
