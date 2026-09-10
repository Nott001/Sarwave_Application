# Sarwave Application

Sensor data platform with Raspberry Pi (sensor + LCD) and Desktop app communicating over local network.

## Structure

- `RpiApp/` — Raspberry Pi application (QML + C++), runs on Pi with sensor and mini LCD
- `DesktopApp/` — Desktop application (QML + C++), displays sensor data and stats
- `common/` — Shared library: protocol definitions and networking layer
- `tests/` — Unit tests

## Build

```bash
cd build
cmake -G Ninja ..
ninja
```

Binaries output to `build/bin/RpiApp/` and `build/bin/DesktopApp/`.

## Development

- Qt 6.5+ required
- CMake 3.16+ required
- C++17
- Install `cmake-tools` VS Code extension for best IntelliSense integration
