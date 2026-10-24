# IMU mounting checks (to do on the real device)

Two things in the IMU calibration still need the assembled handheld:

1. **Check which way the X and Y arrows on the IMU board point.** The code assumes X forward and
   Y up; this is not confirmed.
2. **Trim the last few degrees** of tilt between the IMU and the radar.

Do both **before collecting training data**. Changing the calibration afterwards shifts the
heights the SVM was trained on.

## Background

The IMU (TDK EV_ICM-42688-P) is screwed to the inside of the side wall on the operator's left
(looking at the screen), chip facing into the device, header pins and wires pointing down. In
the code (`ImuFrames::handheldMounting()` in `include/Integration/PeopleTracking/ImuFrames.cpp`):

```cpp
ImuMounting::fromAxes(ImuAxis::PositiveZ, ImuAxis::PositiveX, ImuAxis::PositiveY)
//                    radar right         radar forward       radar up
```

- **Z points to the radar's right.** This follows from the chip facing in, so it is certain.
- **X forward and Y up are a guess.** TDK usually prints X to the right and Y away from the
  header. TDK does not publish this board's silkscreen, so it has to be checked.

If X and Y are wrong, tilting the radar's nose up reads as roll or as negative pitch. Tilt
correction would then level the points the wrong way.

Even with the right axes, the IMU is never perfectly square to the radar. The printed standoffs,
the slack in the screw holes and the chip's placement on its board add 1–3°. Left in, that error
tilts every levelled point by the same angle:

| Distance | Height error at 2° |
|---|---|
| 2 m | ~7 cm |
| 4 m | ~14 cm |
| 6 m | ~21 cm |

Motion gating ("HOLD STEADY") does not depend on it. Tilt correction and point heights do.

## What you need

- The Raspberry Pi with the IMU connected, and the repo built (`cmake --build build`, see the
  README).
- A flat table, and a soft cloth or towel.
- The **IMU monitor** in the terminal app, run from the repo root. It needs only the IMU, not
  the radar, shows live what the IMU reads, and can record it to a file:

  ```bash
  ./build/src/Terminal/SarwaveTerminal imu                       # watch only
  ./build/src/Terminal/SarwaveTerminal imu ~/imu_level_a.swimu   # watch and record
  ```

  Ctrl+C stops it. The `Raw, IMU axes` line shows what each axis reads and which one is
  `pointing up`; `Tilt` shows the pitch and roll with the current mounting.

  The same values (pitch, roll, `UP AXIS` and the gate) are also in the IMU strip under the top
  bar of the **wireless app** on a laptop, while the device's live app streams to it. Handy for
  the checks below without SSH; recording for the trim still needs the monitor.
- `SarwaveImuCheck`, also run from the repo root. It averages recordings for the trim:

  ```bash
  ./build/bin/SarwaveImuCheck a.swimu b.swimu    # several recordings, averaged
  ```

How to record one position: put the device in place, start the monitor with a file name, keep
your hands off the device for **at least 15 seconds** (the gate line reads `STEADY`), then press
Ctrl+C. The check uses only the middle half of a recording, so touching the device at the start
and end does not matter. If it prints `! the device was not still`, record that position again.

A live session of the radar app also records the IMU, to
`~/.local/share/Sarwave/default_session.swimu`, but overwrites it on the next session; the
monitor is simpler for these checks.

## 1. Check the X and Y arrows

**The quick way, if the board is visible:** take the front cover off (4 screws) and look at the
board on the left wall (seen from the screen side). Find the small X and Y arrows printed next
to the chip. The code is right if **X points to the front cover (the radar)** and **Y points to
the roof (the buttons)**. If they point elsewhere, still do steps 1–5 below, which tell you what
to write.

**The sure way, using gravity:** when the IMU is still, the axis that points up reads about
+1 g. The device does not have to be opened.

1. Start the monitor (`./build/src/Terminal/SarwaveTerminal imu`).

2. **Upright.** Stand the device on its feet on the table, the way it is normally held. Note what
   `pointing up:` shows.

3. **Nose up.** Lay the device on its back on the towel (screen down, radar pointing at the
   ceiling). Note what `pointing up:` shows.

4. Find your result:

   | Upright reads | Nose up reads | Mounting in `ImuFrames::handheldMounting()` |
   |---|---|---|
   | `+Y` | `+X` | `fromAxes(ImuAxis::PositiveZ, ImuAxis::PositiveX, ImuAxis::PositiveY)` (current, nothing to change) |
   | `+X` | `-Y` | `fromAxes(ImuAxis::PositiveZ, ImuAxis::NegativeY, ImuAxis::PositiveX)` |
   | `-Y` | `-X` | `fromAxes(ImuAxis::PositiveZ, ImuAxis::NegativeX, ImuAxis::NegativeY)` |
   | `-X` | `+Y` | `fromAxes(ImuAxis::PositiveZ, ImuAxis::PositiveY, ImuAxis::NegativeX)` |

   In general it is `fromAxes(radar right, <nose-up result>, <upright result>)`. If either
   result is `+Z` or `-Z`, the board is not mounted as the model shows (chip facing in, pins
   down): stop and check how it is screwed in.

5. If your row is not the current one, change that line in
   `include/Integration/PeopleTracking/ImuFrames.cpp` and rebuild. The C# app has the same line
   in `Sarwave.Integration.PeopleTracking/ImuFrames.cs`; change both, so the two read the same.

6. **Confirm** with the monitor running again: tilt the device's nose up a little and the
   `Tilt` line's pitch must go **positive**; lower its right side (seen from the screen) and the
   roll must go positive. Upright, pitch and roll should be within a few degrees of 0. Nose up,
   the pitch should be near **+90°**; ignore the roll there, which has no
   meaning with the radar pointing straight up. An upright roll near ±90° or 180°, or a nose-up
   pitch near −90°, means the axes are still wrong.

## 2. Trim the last few degrees

Do this only after part 1 is right.

1. Put the device on its feet in one spot on the flat table, handles pushed in. Record:

   ```bash
   ./build/src/Terminal/SarwaveTerminal imu ~/imu_level_a.swimu
   ```

2. Turn the device round 180° on the same spot, so the radar faces the other way. Record:

   ```bash
   ./build/src/Terminal/SarwaveTerminal imu ~/imu_level_b.swimu
   ```

   Turning it round flips the table's own slope. Averaging the two recordings cancels the slope,
   so the table does not need to be perfectly level, only flat.

3. Run the check on both together:

   ```bash
   ./build/bin/SarwaveImuCheck ~/imu_level_a.swimu ~/imu_level_b.swimu
   ```

   It prints the remaining tilt and the line to add, for example:

   ```
   With the current mounting: pitch -2.00°, roll +0.00°
   Level calibration:  .withLevelCalibration({0.0000f, -0.0349f, 0.9994f})
   ```

4. If pitch and roll are both under 0.5°, the mounting is already good enough; stop here.
   Otherwise, add the printed `.withLevelCalibration(...)` after `fromAxes(...)` in
   `ImuFrames::handheldMounting()`:

   ```cpp
   auto m = Imu::Fusion::ImuMounting::fromAxes(Imu::Fusion::ImuAxis::PositiveZ,
                                               Imu::Fusion::ImuAxis::PositiveX,
                                               Imu::Fusion::ImuAxis::PositiveY)
                .withLevelCalibration({0.0000f, -0.0349f, 0.9994f});
   m.radarOffset = {0.089f, 0.017f, 0.035f};
   ```

   Use your own printed numbers, not the example's. In the C# app the same numbers go in
   `ImuFrames.HandheldMounting` as `.WithLevelCalibration(new Vector3(...))`.

5. Rebuild and run step 3 again on the same two files. Pitch and roll should now read about
   0.00°.

The trim levels the IMU to the enclosure's feet, which the design keeps square to the radar
board. Redo it whenever the IMU board or the radar is unscrewed and mounted again, or the
enclosure is reprinted.

## Recordings are not lost

IMU recordings store the raw readings. Sessions recorded before a mounting change are replayed
with the new mounting automatically, so there is no need to record them again.
