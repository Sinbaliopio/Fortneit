# Windows Screen Capture & Computer-Vision Object Tracking

A high-performance, console-only C++20 application for Windows screen capture using DXGI Desktop Duplication API with computer-vision-based object tracking and dynamic magnetic aim logic.

## Features

- **Ultra-low-latency screen capture** via DXGI Desktop Duplication API (DDA)
- **Purple-hue color detection** with multi-target clustering
- **Dynamic magnetic aim logic** with concentric radial zones:
  - Inner Lock Ring (hard stop)
  - Progressive Slowdown Ring (brake curve)
  - Acceleration Ring (magnetic pull)
  - Friction Ring (away resistance)
- **Y-axis freedom** for recoil compensation
- **Real-time console configuration** - all parameters adjustable at runtime
- **INI file configuration** with auto-reload on change
- **Multi-threaded architecture** - capture/processing separate from console input

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    THREAD 1: Console                        │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐     │
│  │   stdin     │───▶│  Config     │◀───│  config.ini │     │
│  │  Commands   │    │  Manager    │    │  (watcher)  │     │
│  └─────────────┘    └─────────────┘    └─────────────┘     │
└─────────────────────────────────────────────────────────────┘
                              │
                    Shared Config (atomic/mutex)
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                  THREAD 2: BackendLoop                      │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌────────────┐  │
│  │ Capture  │─▶│ Detect   │─▶│ Aim      │─▶│ RawAccel   │  │
│  │ (DXGI)   │  │ Color    │  │ Logic    │  │ Driver     │  │
│  └──────────┘  └──────────┘  └──────────┘  └────────────┘  │
│       │                                                       │
│       ▼                                                       │
│  ┌──────────┐                                                 │
│  │ Telemetry│──▶ stdout (carriage-return updated)            │
│  └──────────┘                                                 │
└─────────────────────────────────────────────────────────────┘
```

## Building

### Prerequisites

- Windows 10/11 (x64)
- Visual Studio 2022 with C++ workload
- CMake 3.20+
- Windows 10 SDK

### Quick Build

```cmd
build.bat
```

Or manually:

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

The executable will be at `build/bin/Release/ScreenCaptureTracking.exe`

## Configuration

All settings are in `config.ini` (auto-created on first run) and can be modified at runtime.

### Console Commands

| Command | Description |
|---------|-------------|
| `help` / `h` / `?` | Show help |
| `exit` / `quit` / `q` | Exit application |
| `reload` / `r` | Reload config from file |
| `save` / `s` | Save current config to file |
| `status` / `st` | Show current configuration |
| `fov <w> <h>` | Set FOV dimensions |
| `offset <x> <y>` | Set FOV center offset |
| `tracking <on\|off>` | Enable/disable aim tracking |
| `yfreedom <on\|off>` | Enable/disable Y-axis freedom |
| `magnetic <on\|off>` | Enable/disable magnetic assist |
| `headsens <val>` | Set head aim sensitivity |
| `accel <val>` | Set magnetic acceleration |
| `corr <val>` | Set magnetic correction |
| `lockradius <px>` | Set inner lock radius |
| `slowdown <px>` | Set slowdown radius |
| `accelradius <px>` | Set acceleration radius |
| `friction <px>` | Set friction radius |

### Example Session

```cmd
> status
=== Current Configuration ===
FOV: 320x320 (offset: 0,0)
Tracking: OFF | Y-Freedom: ON | Magnetic: ON
Sensitivity: head=0.35 accel=2.5 corr=0.15
Radii: lock=5 slowdown=80 accel=200 friction=150
===============================

> fov 400 400
[Config] FOV set to 400x400

> tracking on
[Config] Tracking enabled

> headsens 0.4
[Config] Head aim sensitivity set to 0.4

> save
[Config] Saved to file
```

## Color Detection

Targets **purple hues** with two threshold sets:

**Green ≥ 120:**
- Red-Blue delta ≤ 8
- Red - Green ≥ 50
- Blue - Green ≥ 50
- Red ≥ 105
- Blue ≥ 105

**Green < 120:**
- Red-Blue delta ≤ 13
- Red - Green ≥ 60
- Blue - Green ≥ 60
- Red ≥ 90
- Blue ≥ 90

Clusters merged within 35px, filtered for minimum 5 pixels, 3×3 size.

## Magnetic Aim Logic

### Concentric Radial Zones

```
Distance from crosshair to target:
│
│  0 ── Lock Radius (hard stop, head_aim_sensitivity)
│  │
│  ├─ Slowdown Radius (progressive curve to head_aim_sensitivity)
│  │
│  ├─ Acceleration Radius (pull up to magnetic_acceleration when moving TOWARDS)
│  │
│  └─ Friction Radius (resistance when moving AWAY, magnetic_correction factor)
│
└─▶ Distance (pixels)
```

### Y-Axis Freedom

When enabled and horizontally locked (within lock radius):
- Allows Y sensitivity to bypass slowdown **only when pulling down**
- Simulates vertical recoil compensation
- Activated via `bypassY` flag to RawAccel driver

## RawAccel Integration

The project expects a `SensitivityState` struct with:
```cpp
struct SensitivityState {
    void updateDynamic(float multiplier, bool bypassY);
};
```

Replace the mock implementation in `BackendLoop.cpp` and `main.cpp` with the actual RawAccel driver communication code (`movement.h/.cpp` and common headers).

## Telemetry Output

Real-time in-place updating (via `\r`):

```
[Telemetry] FPS: 144 | Targets: 3 | Found: YES | Dist:  45.2 | Mult:  0.35 | BypassY: OFF | Target: (160,120)
```

## Files

| File | Purpose |
|------|---------|
| `include/Config.hpp` / `src/Config.cpp` | Centralized config + INI loader |
| `include/GameCapture.hpp` / `src/GameCapture.cpp` | DXGI DDA capture engine |
| `include/ColorDetection.hpp` / `src/ColorDetection.cpp` | Purple detection + clustering |
| `include/AimLogic.hpp` / `src/AimLogic.cpp` | Magnetic radial aim logic |
| `include/BackendLoop.hpp` / `src/BackendLoop.cpp` | Background processing thread |
| `src/main.cpp` | Entry point + console loop |
| `config.ini` | Configuration file |
| `CMakeLists.txt` | Build configuration |

## License

Internal use only.