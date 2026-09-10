# Sarwave DesktopApp — Handover Document

## Project Overview
**Sarwave** — SVM-based mmWave radar human presence detection & localization system for indoor fire search & rescue.

- **Repository**: `/home/nott/Sarwave_Application`
- **Stack**: C++17 + Qt 6.10 + QML + CMake + Ninja
- **Build**: `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build build`
- **Binaries**: `build/bin/DesktopApp/DesktopApp` and `build/bin/RpiApp/RpiApp`

---

## What Was Changed

### 1. Data Model (common/)
- **`protocol_definitions.h/.cpp`**: Replaced `SensorData` (temperature/humidity/pressure) with `RadarDetection` struct containing: `presenceDetected`, `centroidX/Y/Z`, `distance`, `dopplerVelocity`, `pointDensity`, `snr`, `spatialSpread`, `classificationConfidence`, `timestamp`.
- **`dashboard_state.h/.cpp`**: All `Q_PROPERTY` fields replaced with radar detection metrics. Demo data generator produces realistic radar patterns (sine-wave modulated values). `readingsChanged` signal renamed to `detectionsChanged`.

### 2. DesktopApp UI (`DesktopApp/main.qml`)
- Header updated: "SARwave | Detection Monitor" / "mmWave Radar Detection Monitor"
- Presence status shows DETECTED/NO TARGET with color-coded indicators
- Left column (420px fixed): Overview with presence status, classification confidence, 6 metric cards (Range, Doppler, Density, SNR, Spread, Centroid)
- Right column: Radar scope Canvas
- **Radar scope features**: Range grid (0–5m), ±45° FOV sector, azimuth markers (-45° to 45°), IWR6843 sensor icon, point cloud cluster of red dots clipped within FOV sector, rotating beam sweep with trails, OCCLUDED label inside cluster
- `import QtQml` added for `Timer` element

### 3. RpiApp UI (`RpiApp/main.qml`)
- Updated to show radar sensor node data: presence YES/NO, Range, Doppler, Density, SNR

### 4. Build Fix
- `common/CMakeLists.txt`: Changed `add_library` → `qt_add_library` to enable moc processing for `Q_OBJECT`/`Q_PROPERTY` macros.

---

## Known Issues (TODO — needs fixing)

### Issue 1: Radar scope layout is broken
- The right-side radar scope in `DesktopApp/main.qml` is compressed/not visible
- The `RowLayout` two-column layout causes the right column Canvas to be too narrow
- The radar Canvas inside the Rectangle is not rendering at proper size
- **Fix needed**: Restructure the layout so the radar scope has adequate width. Consider using `Layout.fillWidth: true` properly, or switch to a `SplitView`, or increase the left column `Layout.preferredWidth` constraint.

### Issue 2: Beam sweep lines still visible
- The green flickering sweep line and trail lines in the radar scope Canvas are still being drawn
- **Fix needed**: Remove lines 159–181 area (sweep angle calculation, beam line drawing, trail loop) from the `onPaint` handler in the Canvas.

### Issue 3: QML import
- `import QtQml` was added to `DesktopApp/main.qml` for the `Timer` type — this works but should be verified it doesn't cause import conflicts on some Qt builds.

---

## File Reference Map

| File | Purpose |
|------|---------|
| `common/protocol_definitions.h` | `RadarDetection` struct |
| `common/protocol_definitions.cpp` | JSON serialization/deserialization |
| `common/dashboard_state.h` | `DashboardState` QObject with radar properties |
| `common/dashboard_state.cpp` | Demo data generation logic |
| `common/network_manager.h/cpp` | TCP networking (not yet wired into apps) |
| `common/CMakeLists.txt` | Uses `qt_add_library` |
| `DesktopApp/main.cpp` | Sets `dashboard` context property |
| `DesktopApp/main.qml` | Full dashboard UI (left overview + right radar) |
| `DesktopApp/CMakeLists.txt` | DesktopApp build config |
| `RpiApp/main.qml` | Sensor node UI |
| `RpiApp/CMakeLists.txt` | RpiApp build config |
| `tests/test_main.cpp` | Empty stub |

---

## Key QML Property Bindings

The QML uses `dashboard.propertyName` to bind to `DashboardState` properties:
- `dashboard.presenceDetected` → bool
- `dashboard.centroidX`, `dashboard.centroidY`, `dashboard.centroidZ` → double
- `dashboard.distance`, `dashboard.dopplerVelocity`, `dashboard.pointDensity`, `dashboard.snr`, `dashboard.spatialSpread`, `dashboard.classificationConfidence` → double
- `dashboard.lastUpdated`, `dashboard.connectionStatus` → string
- `dashboard.demoMode` → bool (constant)
- `dashboard.refresh()` → Q_INVOKABLE method

Signal: `onDetectionsChanged` → triggers `radar.requestPaint()`

---

## Build & Run

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/bin/DesktopApp/DesktopApp
./build/bin/RpiApp/RpiApp
```
