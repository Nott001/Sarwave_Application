# Pilot check: can the radar see a still person?

`SarwavePilotCheck` (`src/tools/PilotCheck.cpp`, built to `build/bin/`) reads one recorded
session and counts how many radar points fall where the person was, per frame and per hold
window, against the points everywhere else. It answers the pilot's questions in
`docs/data_collection_checklist.md` before any filter, DBSCAN or SVM exists:

- At which distances, and in which postures, does a still person give points at all?
- Does an empty room give points in the same spot?
- How much does judging a whole hold window gain over a single frame?
- Can a Doppler filter separate the person from clutter, and where should it cut?
- Do frames hit the radar's 150-point cap?

It sees the points exactly as the app does: the same motion gate (refused frames are left out)
and the same tilt correction, from the `.swimu` saved beside the recording.

## 1. Record the pilot sessions

Use the frozen cfg candidate, the device handheld at 1.0–1.2 m in scan-and-hold, about **1
minute** per session (the more windows, the steadier the percentages). Follow section 4 of the
checklist for each session, and save its two files straight away.

| Session | Scene | Measure |
|---|---|---|
| P01–P03 | Person standing still at about 1, 3 and 5 m | Their X and Y |
| P04–P06 | The same spots, sitting | |
| P07–P09 | The same spots, lying (prone) | |
| P10–P12 | The same spots, curled up on the floor | |
| P13 | The room empty | Nothing; use the spots above |
| P14 | Empty, with a fan or a curtain moving | |

**Measuring the spot.** X and Y are the person's position from the radar, in metres, with the
radar where the operator holds it during the session:

- **Y**: straight ahead along the floor, to the middle of the person's body.
- **X**: to the side, **positive to the radar's right** (the operator's right, looking at the
  screen), negative to the left. 0 if the person is straight ahead.

Mark the operator's standing spot and the person's spot on the floor with tape, so the empty
room (P13) can be checked at exactly the same spots. Keep the radar pointed at the person for
the whole session: the tool counts in the radar's own directions, so turning away by 5° moves
the spot about 0.4 m sideways at 5 m.

## 2. Run it

From the repo root, on the laptop or the Pi, once the repo is built (`cmake --build build`):

```bash
./build/bin/SarwavePilotCheck ~/sarwave_data/sessions/P02 --at 0.0,3.0
```

The first argument is the session's folder (or its `default_session.swrec`). Options:

| Option | Default | Meaning |
|---|---|---|
| `--at X,Y` | required | Where the person was, in metres (see above) |
| `--radius R` | 0.5 | Radius of the person-sized cylinder around `--at`, in metres. 0.5 fits one person with some aiming slack; try 0.7 for lying or curled up, which spread wider |
| `--zmin`, `--zmax` | -0.2, 2.2 | Height band of the cylinder above the floor. Keep `--zmin` near 0 for lying or curled up: they are at 0–0.6 m |
| `--height H` | 1.2 | How high the radar was held. Must match the cfg's `sensorPosition` |
| `--window S` | 3 | Hold window length in seconds |
| `--min N` | 3 | What counts as "enough points" in the second table row |
| `--each` | off | Also list every window: frames kept, points at the person, points elsewhere |

For the empty room, run it with **the same `--at` as each person session**:

```bash
./build/bin/SarwavePilotCheck ~/sarwave_data/sessions/P13 --at 0.0,3.0
```

## 3. Read the output

Example (the numbers are made up):

```
~/sarwave_data/sessions/P02/default_session.swrec
  person at X +0.00 m, Y 3.00 m (radius 0.50 m, height -0.2 to 2.2 m), radar held at 1.20 m
  61.3 s, 1115 frames: kept 1032 (93 %), refused by the gate 83, malformed 0, without a point cloud 0
  20 of 21 3 s windows usable (steady for at least half of it)

                                     per frame      per 3 s window
  with a point at the person         31 %           90 %
  with 3 or more points there        12 %           75 %
  points at the person (median)      0              6
  points elsewhere (median)          1              14
  frames at the 150-point cap        0

                                     at the person          elsewhere
  speed |Doppler|, m/s, 10/50/90 %   0.09 / 0.14 / 0.35     0.10 / 0.22 / 0.60
  SNR (linear), 10/50/90 %           11 / 24 / 80           9 / 15 / 40
```

Line by line:

- **kept / refused by the gate**: frames left after the motion gate. A low kept share means
  the operator was not steady. As a rule of thumb, with more than about 30 % refused, record
  it again.
- **windows usable**: windows where the device was steady for at least half the time. Only
  these count in the window column.
- **with a point at the person**: the share of frames, and of windows, with at least one point
  in the cylinder. **This is the main answer.** For the empty room it is the false-alarm floor
  at that spot.
- **with N or more points there**: the same, for a cluster DBSCAN could plausibly form (one
  stray point is not a cluster).
- **points at the person / elsewhere (median)**: typical counts. "Elsewhere" is the room's own
  clutter while the person was there.
- **frames at the 150-point cap**: the radar sends at most 150 points a frame. If frames hit it,
  point density, the SVM's main feature, stops telling scenes apart.
- **speed |Doppler|**: how fast the points moved toward or away from the radar, 10th, 50th and
  90th percentile. Compare the person's with the empty room's (step 4).
- **SNR**: signal strength of the points, linear (not dB). Same comparison.

`--each` adds one line per window, to see *when* the person gave points: steadily, or only in a
few bursts (a swallow, a shift).

## 4. What to decide from it

Put the results in a table, one row per session:

| Session | Posture | Distance | Windows with a point | Windows with ≥ 3 | Same spot, empty room |
|---|---|---|---|---|---|
| P02 | standing | 3 m | 90 % | 75 % | 10 % |

1. **Range for still people.** Where "windows with a point" stays high and the empty room
   stays low, the radar sees still people. Where the two get close, it does not. That is the
   honest range to claim, per posture.
2. **Per frame or per window.** If the per-frame column is low but the window column is high,
   classifying whole hold windows would detect far more. That is evidence for the adviser.
3. **The cfg threshold.** Far people missed while the empty room is clean: lower `rangeThre` and
   **record again** (detection happens inside the radar, so the threshold cannot be changed on a
   recording). Empty room noisy: raise it, record again.
4. **The Doppler filter.** If the person's speeds sit clearly apart from the empty room's, a
   filter can cut between them. If they overlap, a Doppler filter would remove people with the
   clutter: leave it out, or filter on SNR if that separates better. Never set it from the
   moving-clutter session alone.
5. **The point cap.** Frames at 150 in ordinary scenes mean point density saturates there.
   The cap is the radar's own, not a cfg setting; tell the adviser before relying on that
   feature, and a less sensitive `rangeThre` is the way to get under it.

Everything after the radar (the region, the window length, and later the Doppler filter and
DBSCAN settings) can be tried again and again on the same recordings. Only cfg changes need new
ones.

## Limits

- One person per session: the tool counts one spot.
- It counts in the radar's own directions, levelled by the IMU but not turned back for any
  sideways turn of the device during the session; hence keeping the radar aimed at the person.
- Without a `.swimu` beside the recording there is no gate and no tilt correction; it warns, and
  heights are only right if the device was held level.
- Positions are only as good as the radar's angles, which are coarse on this small antenna
  board: one person's points scatter sideways by tens of centimetres, more further away. That is
  why the cylinder is 0.5 m and not tighter; widen it with `--radius` if a person's points fall
  just outside.
