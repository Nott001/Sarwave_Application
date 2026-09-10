# Sarwave Application

Sensor data platform with Raspberry Pi (sensor + LCD) and Desktop app communicating over local network.

## Structure

- `RpiApp/` — Raspberry Pi application (QML + C++), runs on Pi with sensor and mini LCD
- `DesktopApp/` — Desktop application (QML + C++), displays sensor data and stats
- `common/` — Shared library: protocol definitions and networking layer
- `tests/` — Unit tests

## Build

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Binaries output to `build/bin/RpiApp/` and `build/bin/DesktopApp/`.

## Development

- Qt 6.5+ required
- CMake 3.16+ required
- C++17
- Install `cmake-tools` VS Code extension for best IntelliSense integration

## Frontend architecture

Both QML applications bind to `DashboardState`, a small C++ presentation model
in `common/`. It currently produces deterministic demo sensor readings so the
frontend can be developed without hardware or networking. When the network
layer is introduced, update this model with real readings rather than putting
transport code into QML.

- `DesktopApp` provides the larger monitoring dashboard.
- `RpiApp` provides a compact, touch-friendly sensor-node display.
