# Build and Run

## Prerequisites

You need exactly one C++ compiler. The project has been tested with MinGW g++.
CMake is optional — you can compile directly with g++ using a single command.

| Tool | Required for | Minimum version |
|---|---|---|
| g++ (MinGW) | Direct compilation | 6.3+ (C++17 support) |
| CMake | `build.bat` / CMake workflow | 3.16+ |

Check what you have:

```cmd
g++ --version
cmake --version
```

If only g++ is available (no CMake), use the direct compilation method below.

---

## Project Structure Expected by the Build

```
Delivery Tracking System/
├── include/        ← header files (must be present)
├── src/            ← source files (must be present)
├── CMakeLists.txt  ← CMake config
└── build.bat       ← convenience script
```

The executable must be run from the project root directory so it can find
`packages.csv` in the working directory.

---

## Method 1 — Direct g++ Compilation (recommended, no CMake needed)

Open a terminal in the project root and run:

```cmd
g++ -std=c++17 -Wall -Wextra -Iinclude src/main.cpp src/Package.cpp src/DeliveryTracker.cpp src/UI.cpp -o delivery_tracker.exe
```

On success the terminal returns with exit code 0 and no output.
`delivery_tracker.exe` is created in the project root.

If g++ is not on your PATH, use its full path. On a standard MinGW installation:

```cmd
C:\MinGW\bin\g++.exe -std=c++17 -Wall -Wextra -Iinclude src/main.cpp src/Package.cpp src/DeliveryTracker.cpp src/UI.cpp -o delivery_tracker.exe
```

---

## Method 2 — `build.bat`

`build.bat` uses CMake to configure and build. It tries three CMake generators in order:

1. `"MinGW Makefiles"` — for MinGW/g++ toolchain
2. `"Visual Studio 17 2022"` — for MSVC
3. No `-G` flag — CMake auto-detects

```bat
@echo off
echo Building Delivery Tracking System...

if not exist build mkdir build
cd build

cmake .. -G "MinGW Makefiles" 2>nul || cmake .. -G "Visual Studio 17 2022" 2>nul || cmake ..
cmake --build . --config Release

if %ERRORLEVEL% == 0 (
    echo Build successful!
    echo Run:  build\delivery_tracker.exe
) else (
    echo Build failed. Check errors above.
)
cd ..
```

Run it from the project root:

```cmd
build.bat
```

On success, the executable is at `build\delivery_tracker.exe` (not the project root).

---

## `CMakeLists.txt` Explained

```cmake
cmake_minimum_required(VERSION 3.16)
project(DeliveryTrackingSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)          # require C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON) # error if compiler doesn't support it

# warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(include)        # makes #include "Package.h" work

add_executable(delivery_tracker
    src/main.cpp
    src/Package.cpp
    src/DeliveryTracker.cpp
    src/UI.cpp
)
```

The output executable is named `delivery_tracker` (`.exe` on Windows).

---

## Running the Application

Always run from the project root so the process finds `packages.csv`:

```cmd
cd "d:\C++FILES\Delivery Tracking System"
delivery_tracker.exe
```

If you built with `build.bat` (CMake), run:

```cmd
cd "d:\C++FILES\Delivery Tracking System"
build\delivery_tracker.exe
```

`packages.csv` is created automatically in the current working directory the first
time a package is added. If the file does not exist at startup, the program starts
with an empty package list — this is normal.

---

## Common Problems

### `Permission denied` when compiling

The `.exe` is locked because it is currently running.
Close the running instance, then recompile.

### `cmake` is not recognized

CMake is not installed or not on your PATH. Use Method 1 (direct g++) instead.

### `g++` is not recognized

MinGW is not on your PATH. Either:
- Use the full path: `C:\MinGW\bin\g++.exe ...`
- Add `C:\MinGW\bin` to your system PATH environment variable.

### `packages.csv` not found at startup

This is not an error. The file is created the first time you add a package.
If you previously had data and the file is missing, the data is gone (there is no
backup mechanism).

### Garbled characters in the console

The UI calls `SetConsoleOutputCP(CP_UTF8)` on Windows to set UTF-8 output. If
characters still appear garbled, change your terminal font to one that supports
Unicode (e.g. Consolas or Cascadia Code).

### Build succeeds but exe crashes immediately

Verify you are running from the project root directory. If the working directory is
wrong, no other error will appear — the program will simply start with no data, which
is fine, but paths relative to the working directory must be correct.
