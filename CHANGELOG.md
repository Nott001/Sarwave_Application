# Changelog

All notable changes to **Sarwave** are documented in this file.

Format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0).
Versioning follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

> Changes staged locally that have not yet been tagged as a release.

### Changed — C++ port of the C# SarwaveApplication
- The whole C++ code base is replaced by a port of the C# solution (`SarwaveApplication/`), one
  CMake target per C# project: `SarwaveCoreMMWave`, `SarwaveCoreMMWaveConfig`, `SarwaveCoreGui`,
  `SarwaveDevEnv`, `SarwaveTerminal`, `TestRpiApp`. Namespaces mirror the C# ones.
- Core: serial `Context` (Boost.Asio, cancellable reads), `FrameReader` (C# resync state machine),
  `ConstantCapture`, TLV views, People Tracking TLV records, `CombinedManager` association with
  .NET-ordered collections, `DebugLogger`, recording format `SWMMWREC` v1 (`FrameRecorder`,
  `FrameRecording`, `FramePlayback`), port prompts.
- Config: typed `.cfg` model with all 32 commands, registry, layout, reader, `AopBoard`.
- GUI: Avalonia dashboards ported to Qt Quick; radar grid, data overlay and Z-range bar are
  QPainter items ported from the Avalonia `Render` code. Inter fonts embedded.
- DevEnv: port and cfg dialogues, `RunStuff`, `RunRecorded`, `ConfigMemory` (`config.json`
  written like System.Text.Json).
- `.NET`-compatible number/time formatting so console output, `.cfg` files and on-screen figures
  match the C# app character for character.
- Tests: C# parity fixtures, pseudo-terminal serial tests, GUI tests; C#/C++ parity harnesses.
- `tools/generate_recording.py` writes synthetic People Tracking sessions.
- CI installs Qt 6.8 and runs the GUI tests offscreen.

### Added
- `LiveRpiApp` (`src/LiveRpiApp/`) and `RecordedRpiApp` (`src/RecordedRpiApp/`), ports of
  `Sarwave.Source.LiveRpiApp` and `Sarwave.Source.RecordedRpiApp`: one bare window each, filled by
  the dev environment's live sensor run (`RunStuff`) or recorded replay (`RunRecorded`).

### Changed
- The recorded session moved from the working directory to `Sarwave/default_session.swrec`
  under the user's local application data (`RunRecorded::sessionFile()`), as in the C# version,
  so the live and recorded apps share it wherever they are started from.
- The live run (`RunStuff`) now prints every frame with the `DebugLogger`, as in the C# version.

### Removed
- `RpiApp`, `DesktopApp`, `MMWaveTerminal`, `QtCommon` (`DashboardState`, `NetworkManager`,
  `ProtocolDefinitions`, `MMWavePrompt`) and the old `MMWave` headers -- superseded by the port.
  They remain in git history.

### Earlier unreleased changes
### Added
- **PeopleTracking module** (`include/MMWave/Configured/PeopleTracking/`) — new `Manager.hpp`, `TargetObject.hpp`, `EntryDecompiler.hpp`, `PointCloud.hpp`, `CloudZip.hpp`, `Debug.cpp/.hpp`, `TlvOutput.hpp`, `TlvTypes.hpp`
- **Dashboard files** — `include/QtCommon/Dashboard/DashboardAssist.hpp`, `DashboardState.cpp`, `DashboardState.h`
- **Configuration-specific TlvOutput/TlvTypes** under `include/MMWave/Configured/OutOfBox/` and `include/MMWave/Configured/PeopleTracking/`
- **`.qtcreator/CMakeLists.txt.user`** for QtCreator project integration
- **`BUILD_TEST_RPI_APP`** directory existence check in root `CMakeLists.txt` to gracefully skip if `src/TestRpiApp` is absent

### Changed
- Split `include/MMWave/Tlv/TlvOutput.hpp` and `include/MMWave/Tlv/TlvTypes.hpp` into configuration-specific versions under `include/MMWave/Configured/`
- Moved `QtCommon/DashboardState.*` to `QtCommon/Dashboard/DashboardState.*` to prepare for adding related files
- Renamed `include/MMWave/Tlv/TlvCore.hpp` to `include/MMWave/TlvCore.hpp`
- Removed `m_drawings` in favour of storing point data directly
- Updated dashboards and changed time tracking type
- `.clang-format` compliance: set `IndentAccessModifiers: false` and `BreakBeforeBraces: Attach` across all `include/MMWave/Configured` files
- Fixed include directories and added indexers on TlvOutput.hpp
- QoL improvements to GUI to minimise interactions
- Updated `MMWaveTerminal/main.cpp`, `RpiApp/main.cpp`, `DesktopApp/main.cpp`, `tests/test_main.cpp`, `MMWavePrompt/CfgDialog.qml`, `PortDialog.qml`
- Multiple `CMakeLists.txt` updates for `MMWave`, `QtCommon`, `DesktopApp`, `RpiApp`

### Removed
- `include/MMWave/Tlv/TlvOutput.hpp` and `include/MMWave/Tlv/TlvTypes.hpp` (replaced by configuration-specific versions)
- `include/QtCommon/DashboardState.cpp` and `include/QtCommon/DashboardState.h` (moved to `QtCommon/Dashboard/`)

---

## [0.3.0] — 2026-09-11

### Changed — RpiApp UI (`RpiApp/main.qml`)
- Complete layout redesign optimised for small RPi LCD screens (800×480)
- Radar scope is now the hero element, filling all available vertical space between the header and the metrics strip
- Replaced four separate metric card frames (Range, Doppler, Density, SNR) with a single unified metrics strip — thin vertical dividers separate each cell, no individual card borders
- Removed oversized `YES` / `NO` presence text (54 px) and the `PRESENCE` section label
- Presence state is now indicated solely by the animated dot badge (`DETECTED` / `CLEAR`) in the header
- Header bar condensed to 36 px; includes brand, chip ID, scan status, timestamp, and presence badge in one row
- Radar canvas paint logic ported from DesktopApp: ±45° FOV sector, 5-ring range grid, azimuth markers, IWR6843 sensor icon, FOV-clipped point cloud with radial halo glow, OCCLUDED / distance·doppler annotation

---

## [0.2.1] — 2026-09-11

### Fixed — Demo data generator (`QtCommon/DashboardState.cpp`)
- `centroidY` previously oscillated −1.0 → +1.0 m (`cos` with no absolute value), placing the simulated target behind the sensor 50 % of the time.  Now uses `std::abs(std::cos(...)) * 1.5 + 0.8`, guaranteeing Y ∈ [0.8, 2.3] m (always forward of sensor)
- Reduced `centroidZ` from `sin * 0.5 + 2.0` to `sin * 0.3 + 0.5` so the 3-D Euclidean distance stays in the 1–3 m range rather than being dominated by a 2 m altitude offset
- `centroidX` amplitude reduced from ±1.5 m to ±1.2 m to keep the azimuth inside ±40° at all times

### Fixed — Radar canvas azimuth formula (`DesktopApp/main.qml`)
- `Math.atan2(centroidX, -centroidY)` was negating the Y axis, producing bearings of 90°–180° (off-screen) whenever `centroidY > 0` (i.e. when the target was in front of the sensor)
- Corrected to `Math.atan2(centroidX, centroidY)` — standard radar convention where Y is the forward (range) axis and X is lateral

---

## [0.2.0] — 2026-09-11

### Changed — DesktopApp UI (`DesktopApp/main.qml`)
- Rewrote root layout from `RowLayout` (two-column) to `Item`-based anchoring: left panel has a fixed 400 px width; right panel anchors to `parent.right` and fills all remaining space — eliminates the canvas compression bug reported in HANDOVER Issue 1
- Radar `Canvas` now receives its full allocated width and height every repaint
- Removed the `50 ms` polling `Timer` (Issue 2); repaints are now signal-driven via `Connections { onDetectionsChanged }` only — eliminates ~20 idle repaints per second
- Removed `import QtQml` (was only required for the now-deleted `Timer` element)
- Design tokens renamed to `clrXxx` prefix to avoid collision with the built-in `color` keyword
- Added `SequentialAnimation` pulse on the presence status dot when target is detected
- Added Demo Mode badge in the header toolbar
- Radar canvas: radial gradient halo added around the point cloud cluster; sensor icon updated to dual-ring style; azimuth guide lines added; `textBaseline` set explicitly for all canvas text draws
- Footer of left panel now shows connection status dot + label alongside the last-updated timestamp
- `MetricCard` height reduced from 120 px to 100 px; font sizes tightened for visual balance
- Added a fallback "Resize window" overlay when the canvas is too small to render

---

## [0.1.1] — 2026-09-11

### Changed — Repository hygiene
- `.gitignore`: added `HANDOVER.md` and `thesis_context.md` (local reference docs, not for VCS)

---

## [0.1.0] — 2026-09-11

### Added — Data model rewrite (`QtCommon/`)
- `RadarDetection` struct in `ProtocolDefinitions.h/.cpp` replacing the legacy `SensorData` (temperature / humidity / pressure) struct; fields: `presenceDetected`, `centroidX/Y/Z`, `distance`, `dopplerVelocity`, `pointDensity`, `snr`, `spatialSpread`, `classificationConfidence`, `timestamp`
- `DashboardState` (`DashboardState.h/.cpp`) exposes all radar metrics as `Q_PROPERTY` bindings; demo data generator uses sine-wave modulation to simulate realistic radar patterns; `readingsChanged` signal renamed to `detectionsChanged`
- `NetworkManager.h/.cpp` — TCP networking skeleton (not yet wired into either app)

### Added — DesktopApp initial UI (`DesktopApp/main.qml`)
- Header: "SARWAVE | Detection Monitor" / "mmWave Radar Detection Monitor"
- Presence badge (DETECTED / NO TARGET) with colour-coded background
- Left column (fixed width): Overview section — presence status, classification confidence, six metric cards (Range, Doppler, Density, SNR, Spread, Centroid)
- Right column: Radar scope Canvas with range grid (0–5 m), ±45° FOV sector, azimuth markers, IWR6843 sensor icon, point cloud cluster clipped within FOV

### Added — RpiApp initial UI (`RpiApp/main.qml`)
- Compact sensor-node dashboard for Raspberry Pi: presence YES/NO, Range, Doppler, Density, SNR metric tiles

### Fixed — Build system (`QtCommon/CMakeLists.txt`)
- Changed `add_library` → `qt_add_library` to enable MOC processing for `Q_OBJECT` / `Q_PROPERTY` macros

---

## [0.0.1] — 2026-09-10

### Added — Project scaffold
- CMake + Ninja build system (`CMakeLists.txt`, `DesktopApp/CMakeLists.txt`, `RpiApp/CMakeLists.txt`, `QtCommon/CMakeLists.txt`)
- `DesktopApp/main.cpp` and `RpiApp/main.cpp` entry points; `dashboard` set as QML context property
- Initial `.gitignore`, `.editorconfig`, `.clang-format`
- `tests/test_main.cpp` stub
- `README.md`

---

[Unreleased]: https://github.com/user/Sarwave_Application/compare/v0.3.0...HEAD
[0.3.0]: https://github.com/user/Sarwave_Application/compare/v0.2.1...v0.3.0
[0.2.1]: https://github.com/user/Sarwave_Application/compare/v0.2.0...v0.2.1
[0.2.0]: https://github.com/user/Sarwave_Application/compare/v0.1.1...v0.2.0
[0.1.1]: https://github.com/user/Sarwave_Application/compare/v0.1.0...v0.1.1
[0.1.0]: https://github.com/user/Sarwave_Application/compare/v0.0.1...v0.1.0
[0.0.1]: https://github.com/user/Sarwave_Application/releases/tag/v0.0.1
