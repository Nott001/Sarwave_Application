# Sarwave Application

C++23 / Qt 6 port of the C# **SarwaveApplication** solution: reads a TI IWR6843AOP mmWave radar
running the 3D People Tracking demo, tracks the people it reports, gates and levels each frame
with an ICM-42688-P IMU, survives damaged frames from the serial link, records and replays
sessions, streams them over the LAN, and shows them on a radar dashboard. Logic, console output and GUI follow the C# version; the
core is verified to produce byte-identical output (see [Verification](#verification)).

## Layout

Every C# project has a counterpart:

| C# project | C++ | CMake target |
|---|---|---|
| `Sarwave.Core.MMWave` | `include/MMWave/` — serial ports, frame reassembly, TLV decoding, recording/playback, `DebugLogger` (pure C++ + Boost.Asio, no Qt) | `SarwaveCoreMMWave` |
| `Sarwave.Core.Imu` | `include/Imu/` — ICM-42688-P driver (Linux spidev), FIFO sampler and clock recovery, tilt fusion, motion gate, `.swimu` recording (pure C++, no Qt) | `SarwaveCoreImu` |
| `Sarwave.Core.MMWave` (`Config/`, `PeopleTracking/Config/`) | `include/MMWave/Config/`, `include/MMWave/Configured/PeopleTracking/Config/` — typed `.cfg` model | `SarwaveCoreMMWaveConfig` |
| `Sarwave.Integration.PeopleTracking` | `include/Integration/` — tracking association moved with the device (`CombinedManager`, `TrackerFrame`), `EntryPackage`, `ImuFrames`, LAN streaming (`Lan/`, Boost.Asio) (pure C++, no Qt) | `SarwaveIntegrationPeopleTracking` |
| `Sarwave.Core.Gui` | `include/Gui/` — radar dashboards (MinimalGui, MaximalGui) in Qt Quick, custom-drawn radar plot | `SarwaveCoreGui` |
| `Sarwave.DevEnv` | `include/DevEnv/` — port, cfg and LAN dialogues, `RunStuff` (live), `RunRecorded` (replay), `RunLan` (wireless), `ConfigMemory` | `SarwaveDevEnv` |
| `Sarwave.Source.Terminal` | `src/Terminal/` | `SarwaveTerminal` |
| `Sarwave.Source.TestRpiApp` | `src/TestRpiApp/` | `TestRpiApp` |
| `Sarwave.Source.LiveRpiApp` | `src/LiveRpiApp/` | `LiveRpiApp` |
| `Sarwave.Source.RecordedRpiApp` | `src/RecordedRpiApp/` | `RecordedRpiApp` |
| `Sarwave.Source.WirelessRpiApp` | `src/WirelessRpiApp/` | `WirelessRpiApp` |
| `Sarwave.Core.Imu.Tests` | `src/tests/Imu/` | `ImuTests` |

Namespaces mirror the C# ones (`Sarwave::Core::MMWave::Streaming`, `Sarwave::Core::Gui::Views`, ...).

## Build

Requirements: CMake 3.25+, a C++23 compiler (GCC 14+), Boost (Asio, headers only), Qt 6.5+
(Core, Gui, Network, Qml, Quick, QuickControls2, Test). `-DSARWAVE_BUILD_GUI=OFF` builds only the core
libraries, the terminal app and their tests (no Qt needed).

```bash
cmake -S . -B build -G Ninja             # Release by default; add -DCMAKE_BUILD_TYPE=Debug to debug
cmake --build build
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure
```

Build the Pi's binaries in Release (the default). A Debug build does the per-frame work about
five times slower. `-DSARWAVE_NATIVE_CPU=ON` also tunes the code for the CPU it is built on; the
gain is small and those binaries may not run on another machine. `-DSARWAVE_LTO=ON` turns on
link-time optimisation; it measured no faster here and makes rebuilds much slower, so it is off.

## Run

The session file is `~/.local/share/Sarwave/default_session.swrec` (`%LOCALAPPDATA%\Sarwave` on
Windows), the same place the C# apps use, with its IMU recording beside it as
`default_session.swimu`.

```bash
# On the Pi, live: port, cfg and LAN dialogues, MinimalGui, records the session (and the IMU)
# and, if asked, streams it to the wireless app.
./build/bin/LiveRpiApp                  # IMU used if detected (--imu=auto)
./build/bin/LiveRpiApp --imu=required   # data collection: refuse to start without the IMU
./build/bin/LiveRpiApp --imu=off        # never touch SPI
./build/bin/LiveRpiApp --print-frames   # also print every frame (slow on a Pi)
./build/bin/LiveRpiApp --timing         # print processing and drawing times every 10 s

# Replay the session on the MaximalGui dashboard, gated exactly as it was live. No sensor?
python3 tools/generate_recording.py ~/.local/share/Sarwave/default_session.swrec
python3 tools/generate_imu_recording.py ~/.local/share/Sarwave/default_session.swimu
./build/bin/RecordedRpiApp

# On a laptop on the same network: the MaximalGui dashboard of the session the live app streams
# (its address and port, 5577 by default). Reconnects by itself; the title says how it stands.
./build/bin/WirelessRpiApp

# TestRpiApp replays too, printing every frame (what the C# TestRpiApp does); --live runs live.
./build/bin/TestRpiApp

# Terminal: prompts for port numbers and the cfg path, sends the cfg, starts the sensor.
./build/src/Terminal/SarwaveTerminal
```

The command-line options are the only additions to the C# behaviour; the C# apps set the same
things in code (`ImuFrames.Mode` in `App.cs`, `RunStuff.OnOpened(window, save)`).

### Speed on the Pi

`SarwaveBench` times the live app's per-frame work on a recording, step by step. Run it on the
Pi with a busy session (about 140 points and 8 people per frame, more than any real recording):

```bash
python3 tools/generate_recording.py busy.swrec --busy --frames 3000
python3 tools/generate_imu_recording.py busy.swimu --seconds 170
./build/bin/SarwaveBench busy.swrec --imu busy.swimu
```

On a desktop PC (Release) the reader takes about 2 µs per frame and the worker (record, IMU gate,
tracking, dashboard snapshot) about 20 µs; drawing the points takes about 0.5 ms. A frame arrives
every 55 ms. The Pi is a few times slower, which still leaves the budget almost untouched.

To see the real figures with the radar attached, run `LiveRpiApp --timing`. Every 10 seconds it
prints one line:

```
[00:00:20.004] Timing, last 10 s: 182 frames, processing avg 19 us / max 85 us, waiting avg 4 us / max 40 us; drawn 181 times, avg 0.62 ms / max 2.10 ms
```

*Processing* is the worker's time per frame, *waiting* is how long a frame queued before the
worker took it, and *drawn* is the radar view's paints. Measuring costs well under a microsecond
per frame, on the worker and GUI threads only; the data-port reader is not touched, so it can't
cause damaged frames.

What keeps it that way:

- **Real-time reader.** `LiveRpiApp` asks Linux to run the data-port reader (and the IMU sampler)
  ahead of every normal thread, so the GUI or other programs can't delay it. That needs
  permission once, otherwise the app says so at start and runs them normally:

  ```bash
  echo "$USER - rtprio 30" | sudo tee /etc/security/limits.d/sarwave.conf   # then log in again
  ```

- **The dashboard skips stale frames.** If drawing ever falls behind, it draws the newest frame
  and drops the ones it missed, instead of queueing them and lagging further and further behind.
- **Cheap point drawing.** Each colour's dot is drawn once and copied, 3 to 4 times faster than
  drawing thousands of small circles each frame, and it looks the same.
- **On the Pi itself:** the official 27 W power supply and a cooler (under-voltage and heat both
  slow the CPU; `vcgencmd get_throttled` shows either), and no `--print-frames`.

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
jolt) are left out until it has settled for 0.3 s; the dashboard shows **HOLD STEADY** once frames
have been refused for 0.6 s, so a knock passes silently. Points of accepted frames are rotated
level with the measured pitch and roll, and what the dashboard holds from earlier frames (point
clouds, targets the radar stopped reporting) is moved by how the device turned and moved since, so
it stays where it was in the room. Targets are placed using the cfg's `sensorPosition`
(`TrackerFrame`). Without an IMU (a PC, SPI off) everything runs as before. Mounting, radar
offset and thresholds are set in `ImuFrames::options()`. The desktop dashboard (recorded and
wireless apps) shows pitch, roll, turning rate, the IMU axis pointing up and the gate's verdict in
a strip under its top bar. The C# solution's `docs/sessions.md` explains the design; this port
follows it to the bit.

### LAN streaming

`LiveRpiApp` asks, after the cfg, whether to offer the session on the network (Don't Stream to
skip). If so it listens on the address and port given (0.0.0.0 for every network; port 5577 by
default) and sends each frame as the radar sent it, malformed-frame notes and the raw IMU samples
to one `WirelessRpiApp` at a time; a newer one takes the place of the old. The wireless app gates,
levels and moves each frame with its own `ImuFrames::options()`, exactly as a replay, so its
mounting must match the device's. Nothing is queued without a receiver, and if the network falls
behind by more than 256 messages (about 10 s), the newest ones are dropped. Both apps remember
their last address in `config.json`. The wire format (`Lan/LanFormat.hpp`) is byte for byte the C#
one, so either app works with the other's C# counterpart.

## Verification

- `src/tests/Parity/` holds C++ harnesses and their C# twins (built against the original
  `Sarwave.Core.MMWave`, `Sarwave.Core.Imu` and `Sarwave.Integration.PeopleTracking`). They print
  `DebugLogger` output, tracker state after every frame (also gated and moved by an IMU recording),
  LAN messages,
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
- `CoreTests` also streams a session from a `LanSender` to a `LanReceiver` over loopback.
- `GuiTests` covers geometry, colours, view models, live updates across threads, loading the
  dashboards and dialogues headless, and the wireless app showing what a sender streams. `-DSARWAVE_BUILD_GUI_PREVIEW=ON` builds
  `SarwaveGuiPreview`, which renders both dashboards to PNG.

## Fonts

`assets/Fonts/Inter-*.ttf` are the files Avalonia.Fonts.Inter embeds (Avalonia's default UI
font; SIL Open Font License), so text renders as in the C# app. `CascadiaMono.ttf` is the
dashboard's monospace font.
