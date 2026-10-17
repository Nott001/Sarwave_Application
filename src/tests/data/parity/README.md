# Parity fixtures

Recordings and byte streams together with the text the **original C# code**
(`SarwaveApplication/Sarwave.Core.MMWave`) prints for them, through the C# twin of
`src/tests/Parity/CoreParityHarness.cpp`. `CoreTests` decodes the same inputs with the C++ port
and requires identical text, so the port is checked against C# behaviour on every build.

| file | made by |
|---|---|
| `session.swrec` | `tools/generate_recording.py session.swrec --frames 40 --seed 3` |
| `reuse.swrec` | hand-built frames: track ids freed and reused, rejected points, presence toggles (then `tools/fixup_recording.py`) |
| `unknown_tlv.swrec` | a frame with a TLV type the people-tracking demo never sends (then `tools/fixup_recording.py`) |
| `stream.bin` | `tools/generate_stream.py stream.bin --frames 120 --seed 5`: frames with garbage, partial/false magic words, corrupt headers, bytes lost mid-TLV, in the padding and at the end, lost frames |
| `<name>.log0.txt` / `.log1.txt` | C# `DebugLogger.PrintFrameDataVerbose`, bytes shown / hidden |
| `<name>.mgr1000.txt` / `.mgr120.txt` | C# `CombinedManager` state after each frame, 1000 / 120 ms retention (session: 120 ms only, to keep it small) |
| `stream.reader7.txt` | C# `FrameReader` over `stream.bin` fed in pseudo-random chunks (seed 7) |

Since the malformed-frame checks (C# commit 5a3a6ab), only frames laid out as the device sends
them decode: heights alongside targets, `numDetectedObj` equal to the point count, 32-byte
padding. `tools/fixup_recording.py` brought the two hand-built recordings to that layout without
changing what they say. All text files were regenerated with the current C# code.
