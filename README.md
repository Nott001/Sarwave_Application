# Sarwave Application

C++23 / Qt 6 port of the C# **SarwaveApplication** solution: reads a TI IWR6843AOP mmWave radar
running the 3D People Tracking demo, tracks the people it reports, gates and levels each frame
with an ICM-42688-P IMU, survives damaged frames from the serial link, records and replays
sessions, and shows them on a radar dashboard. Logic, console output and GUI follow the C# version; the
core is verified to produce byte-identical output (see [Verification](#verification)).

## Layout

Every C# project has a counterpart:

| C# project | C++ | CMake target |
|---|---|---|
| `Sarwave.Core.MMWave` | `include/MMWave/` — serial ports, frame reassembly, TLV decoding, tracking association, recording/playback, `DebugLogger` (pure C++ + Boost.Asio, no Qt) | `SarwaveCoreMMWave` |
| `Sarwave.Core.Imu` | `include/Imu/` — ICM-42688-P driver (Linux spidev), FIFO sampler and clock recovery, tilt fusion, motion gate, `.swimu` recording (pure C++, no Qt) | `SarwaveCoreImu` |
| `Sarwave.Core.MMWave` (`Config/`, `PeopleTracking/Config/`) | `include/MMWave/Config/`, `include/MMWave/Configured/PeopleTracking/Config/` — typed `.cfg` model | `SarwaveCoreMMWaveConfig` |
| `Sarwave.Core.Gui` | `include/Gui/` — radar dashboards (MinimalGui, MaximalGui) in Qt Quick, custom-drawn radar plot | `SarwaveCoreGui` |
| `Sarwave.DevEnv` | `include/DevEnv/` — port and cfg dialogues, `RunStuff` (live), `RunRecorded` (replay), `ImuFrames`, `ConfigMemory` | `SarwaveDevEnv` |
| `Sarwave.Source.Terminal` | `src/Terminal/` | `SarwaveTerminal` |
| `Sarwave.Source.TestRpiApp` | `src/TestRpiApp/` | `TestRpiApp` |
| `Sarwave.Source.LiveRpiApp` | `src/LiveRpiApp/` | `LiveRpiApp` |
| `Sarwave.Source.RecordedRpiApp` | `src/RecordedRpiApp/` | `RecordedRpiApp` |
| `Sarwave.Core.Imu.Tests` | `src/tests/Imu/` | `ImuTests` |

Namespaces mirror the C# ones (`Sarwave::Core::MMWave::Streaming`, `Sarwave::Core::Gui::Views`, ...).

## Build

Requirements: CMake 3.25+, a C++23 compiler (GCC 14+), Boost (Asio, headers only), Qt 6.5+
(Core, Gui, Qml, Quick, QuickControls2, Test). `-DSARWAVE_BUILD_GUI=OFF` builds only the core
libraries, the terminal app and their tests (no Qt needed).

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure
```

## Run

The session file is `~/.local/share/Sarwave/default_session.swrec` (`%LOCALAPPDATA%\Sarwave` on
Windows), the same place the C# apps use, with its IMU recording beside it as
`default_session.swimu`.

```bash
# On the Pi, live: port + cfg dialogues, MinimalGui, records the session (and the IMU).
./build/bin/LiveRpiApp                  # IMU used if detected (--imu=auto)
./build/bin/LiveRpiApp --imu=required   # data collection: refuse to start without the IMU
./build/bin/LiveRpiApp --imu=off        # never touch SPI
./build/bin/LiveRpiApp --print-frames   # also print every frame (slow on a Pi)

# Replay the session on the MaximalGui dashboard, gated exactly as it was live. No sensor?
python3 tools/generate_recording.py ~/.local/share/Sarwave/default_session.swrec
python3 tools/generate_imu_recording.py ~/.local/share/Sarwave/default_session.swimu
./build/bin/RecordedRpiApp

# TestRpiApp replays too, printing every frame (what the C# TestRpiApp does); --live runs live.
./build/bin/TestRpiApp

# Terminal: prompts for port numbers and the cfg path, sends the cfg, starts the sensor.
./build/src/Terminal/SarwaveTerminal
```

The command-line options are the only additions to the C# behaviour; the C# apps set the same
things in code (`ImuFrames.Mode` in `App.cs`, `RunStuff.OnOpened(window, save)`).

### Damaged frames

The serial link has no checksum. Packets that lost bytes are rejected (`FrameReader`,
`EntryPackage`), printed as `Malformed frame` lines, recorded as notes in the `.swrec`, and the
dashboard skips them; the app never freezes on them. `ConstantCapture` reads the port on its own
thread, so a slow callback can no longer cause the losses. See
[docs/pi_frame_corruption.md](docs/pi_frame_corruption.md) for the causes, what the port does
about each, and what is left to check on the Pi's power supply and cabling.

### IMU

The ICM-42688-P sits on SPI0 CE0 (`/dev/spidev0.0`, mode 3). The user needs to be in the `spi`
group, with `dtparam=spi=on` in `/boot/firmware/config.txt`. Frames taken while the device moves
(average rate over 6 °/s or average acceleration over 0.08 g in the 150 ms before the frame, or a
jolt) are skipped, and the dashboard shows **HOLD STEADY** until it has settled for 0.3 s. Points
of accepted frames are rotated level with the measured pitch and roll. Without an IMU (a PC, SPI
off) everything runs as before. Mounting and thresholds are set in `ImuFrames::options()`. The C#
solution's `docs/sessions.md` (2026-10-15) explains the design; this port follows it to the bit.

## Verification

- `src/tests/Parity/` holds C++ harnesses and their C# twins (built against the original
  `Sarwave.Core.MMWave` and `Sarwave.Core.Imu`). They print `DebugLogger` output, tracker state after every frame,
  frame reassembly from a corrupted byte stream (every kind of malformed packet), recording I/O,
  playback timing, `.cfg` parsing and writing, and every error message. Across more than 600,000
  lines of output the two are identical. The IMU harness compares fused states, gate verdicts and
  mounting rotations. These match to the last bit as well, which needed .NET 10's own quaternion
  arithmetic (`MMWave/Common/Numerics.hpp`) and `-ffp-contract=off`.
- `src/tests/data/` keeps fixtures plus the C# output for them. `CoreTests`, `ImuTests` and
  `ConfigTests` compare against those on every build, without needing .NET.
- `CoreTests` also drives `Context`, `ConstantCapture` and `FrameRecorder` end to end against a
  fake IWR6843 made of two pseudo-terminals (Linux): damaged packets, a slow callback, a callback
  that throws.
- `ImuTests` is the C# `Sarwave.Core.Imu.Tests` (mounting, tilt, bias learning, every gate
  transition, the timeline, clock recovery, the recording round trip), ported one to one.
- `GuiTests` covers geometry, colours, view models, live updates across threads, and loading the
  dashboards and dialogues headless. `-DSARWAVE_BUILD_GUI_PREVIEW=ON` builds
  `SarwaveGuiPreview`, which renders both dashboards to PNG.

## Fonts

`assets/Fonts/Inter-*.ttf` are the files Avalonia.Fonts.Inter embeds (Avalonia's default UI
font; SIL Open Font License), so text renders as in the C# app. `CascadiaMono.ttf` is the
dashboard's monospace font.
