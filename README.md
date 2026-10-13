# Sarwave Application

C++23 / Qt 6 port of the C# **SarwaveApplication** solution: reads a TI IWR6843AOP mmWave radar
running the 3D People Tracking demo, tracks the people it reports, records and replays sessions,
and shows them on a radar dashboard. Logic, console output and GUI follow the C# version; the
core is verified to produce byte-identical output (see [Verification](#verification)).

## Layout

Every C# project has a counterpart:

| C# project | C++ | CMake target |
|---|---|---|
| `Sarwave.Core.MMWave` | `include/MMWave/` — serial ports, frame reassembly, TLV decoding, tracking association, recording/playback, `DebugLogger` (pure C++ + Boost.Asio, no Qt) | `SarwaveCoreMMWave` |
| `Sarwave.Core.MMWave` (`Config/`, `PeopleTracking/Config/`) | `include/MMWave/Config/`, `include/MMWave/Configured/PeopleTracking/Config/` — typed `.cfg` model | `SarwaveCoreMMWaveConfig` |
| `Sarwave.Core.Gui` | `include/Gui/` — radar dashboards (MinimalGui, MaximalGui) in Qt Quick, custom-drawn radar plot | `SarwaveCoreGui` |
| `Sarwave.DevEnv` | `include/DevEnv/` — port and cfg dialogues, `RunStuff` (live), `RunRecorded` (replay), `ConfigMemory` | `SarwaveDevEnv` |
| `Sarwave.Source.Terminal` | `src/Terminal/` | `SarwaveTerminal` |
| `Sarwave.Source.TestRpiApp` | `src/TestRpiApp/` | `TestRpiApp` |
| `Sarwave.Source.LiveRpiApp` | `src/LiveRpiApp/` — the live sensor run (`RunStuff`) in one bare window | `LiveRpiApp` |
| `Sarwave.Source.RecordedRpiApp` | `src/RecordedRpiApp/` — the recorded replay (`RunRecorded`) in one bare window | `RecordedRpiApp` |

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

```bash
# The live run: port + cfg dialogues, MinimalGui, records the session.
./build/bin/LiveRpiApp

# Replay the recorded session into the MaximalGui dashboard, printing every frame.
# No sensor? Generate a session first:
mkdir -p ~/.local/share/Sarwave
python3 tools/generate_recording.py ~/.local/share/Sarwave/default_session.swrec
./build/bin/RecordedRpiApp

# TestRpiApp does the same replay (what the C# TestRpiApp does); --live switches it to the live run.
./build/bin/TestRpiApp
./build/bin/TestRpiApp --live

# Terminal: prompts for port numbers and the cfg path, sends the cfg, starts the sensor.
./build/src/Terminal/SarwaveTerminal
```

The session is `default_session.swrec` in a `Sarwave` folder under the user's local application
data (`~/.local/share`, or `$XDG_DATA_HOME`, on Linux; `%LOCALAPPDATA%` on Windows), so the live
and recorded apps share it whichever folder they are started from.

`TestRpiApp --live` is the only addition to the C# behaviour: in C# switching `TestRpiApp` to
the live run means editing `MainWindow.axaml.cs`.

## Verification

- `src/tests/Parity/` holds a C++ harness and its C# twin (built against the original
  `Sarwave.Core.MMWave`). They print `DebugLogger` output, tracker state after every frame,
  frame reassembly from a corrupted byte stream, recording I/O, playback timing, `.cfg` parsing
  and writing, and every error message. Across more than 600,000 lines of output the two are identical.
- `src/tests/data/` keeps fixtures plus the C# output for them. `CoreTests` and `ConfigTests`
  compare against those on every build, without needing .NET.
- `CoreTests` also drives `Context`, `ConstantCapture` and `FrameRecorder` end to end against a
  fake IWR6843 made of two pseudo-terminals (Linux).
- `GuiTests` covers geometry, colours, view models, live updates across threads, and loading the
  dashboards and dialogues headless. `-DSARWAVE_BUILD_GUI_PREVIEW=ON` builds
  `SarwaveGuiPreview`, which renders both dashboards to PNG.

## Fonts

`assets/Fonts/Inter-*.ttf` are the files Avalonia.Fonts.Inter embeds (Avalonia's default UI
font; SIL Open Font License), so text renders as in the C# app. `CascadiaMono.ttf` is the
dashboard's monospace font.
