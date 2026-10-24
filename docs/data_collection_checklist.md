# Data collection checklist (SVM training and testing)

What to record for the SVM, and how, so the data can be split, trained on and tested the way
the manuscript's methodology describes (`docs/thesis_context.md`, Data Gathering Procedure,
Data Collection Protocol and Test Scenarios). Items marked **(adviser)** go beyond the
manuscript as written and should be agreed on before collecting.

## 1. Before the first session (once)

- [ ] **IMU checks done:** X/Y arrows confirmed and level trim applied
      (`docs/imu_mounting_checks.md`). Changing the IMU calibration later shifts every point
      height the SVM was trained on.
- [ ] **Choose the radar cfg and freeze it.** Every training, validation and test recording,
      and the deployed device, must use the same cfg. Data recorded with another cfg cannot be
      mixed in.

      | cfg | Reaches | Range step | Fits |
      |---|---|---|---|
      | `configs/AOP_6m_default.cfg` | ~8.1 m | ~8.4 cm | The manuscript's zones (0–6 m); TI tuned it for this range |
      | `configs/Sarwave_12m.cfg` | ~12.3 m | ~12.8 cm | Only if the claimed range goes beyond 6 m |

      Both are TI's 6 m settings (CFAR threshold 5) and differ only in distance. Do not use
      the `fineMotion` / `staticRetention` cfgs: on a handheld, fine motion turns hand sway into
      points on walls and furniture.
- [ ] **Run a short pilot** with the chosen cfg, handheld, about 1 minute per scene: a still
      person at about 1, 3 and 5 m (or the furthest claimed distance), standing and **curled up
      on the floor** (fetal position), the same room empty, and the empty room with likely
      clutter (a fan, a curtain). Check that the person gives a cluster in most hold windows at
      the furthest distance, in both postures, and that the empty room gives few stray points.
      Curled up is the hardest case: a small, low target near the floor with little movement.
      `SarwavePilotCheck` counts the points for this; `docs/pilot_check.md` has the sessions
      to record and how to read and use the results. Only if not, adjust `rangeThre` in `dynamicRACfarCfg` (lower, e.g. 4, if far
      people are missed; higher, 6–8, if empty rooms are noisy) and pilot again. Then freeze.
- [ ] **Save the frozen cfg with the data:** copy it into the dataset folder (section 5), with
      the git commit it came from (`git rev-parse HEAD`). Recordings do not store the cfg, and
      replays use whichever cfg is selected at the time.
- [ ] **(adviser)** The literature synthesis (p. 34) says robustness requires explicit negative
      examples of fire-environment clutter, but the protocol trains on clean empty rooms only.
      Either add debris recordings to training (section 2, Model B) or reword the synthesis.
- [ ] **(adviser)** The minimum of 50 frames per class per zone is about one session each, too
      few for session-level 70/15/15 splits and 5-fold cross-validation. Plan on the session
      counts in section 2.

## 2. Training and validation recordings

All handheld at 1.0–1.2 m in scan-and-hold, never on a table or tripod: the data must contain
the operator's hand micro-movements in both classes, as in real use.

**Clean (the manuscript's protocol, Model A):** open, unobstructed room, no environmental
factors.

| Class | What is recorded | Per zone (A 0–2 m, B 2–4 m, C 4–6 m) |
|---|---|---|
| 1, human present | Still: only breathing and involuntary micro-motion. Slow-moving: small position adjustments | At least 10 sessions, both states, spread across subjects and postures |
| 0, human absent | The subject has left the detection zone; the same empty room | At least 10 sessions |

- [ ] Postures across Class 1 sessions: **sitting, standing and prone** (the participants
      section lists all three; the protocol's Class 1 text mentions only standing).
- [ ] **(adviser)** Add **curled up** (fetal position on the floor) as a fourth posture, with
      sessions in every zone. Fire victims often stay low and curl up, and a curled-up body gives
      a smaller, lower cluster than any of the other three. An SVM that never saw one may call
      it "not human" even when the radar sees it.
- [ ] Every subject appears in several zones and both states; Class 0 is recorded in the same
      room and session days as Class 1.
- [ ] Subjects keep normal breathing; exclude anyone with a respiratory condition from smoke
      trials.

**Debris (Model B, (adviser)):** the same as clean, with plywood or drywall panels between the
radar and the subject.

- [ ] **Every debris layout is recorded in both classes**: with the subject behind it (Class 1)
      and with nobody there (Class 0). Debris only in Class 1 teaches the SVM "debris = person".
- [ ] **Training layouts differ from test layouts** (other panel positions, angles, distances
      from the radar, or another panel). Write each layout down (section 5) with a photo.
- [ ] At least 10 sessions per class per zone, as for clean.

**Moving clutter (optional, (adviser)):** a fan with paper strips, a swaying curtain, with
nobody there. Class 0 only, a few sessions, kept as its own labelled set so results can be
reported with and without it.

**Smoke:** not recorded for training. At 60 GHz it has negligible effect (manuscript p. 52).

## 3. Test-only recordings (held out, never trained on)

The manuscript's test matrix (Table 6): three zones × three environments.

| | Zone A (0–2 m) | Zone B (2–4 m) | Zone C (4–6 m) |
|---|---|---|---|
| Open | Scenario 1 | Scenario 2 | Scenario 3 |
| Partial debris | Scenario 4 | Scenario 5 | Scenario 6 |
| Smoke-filled | Scenario 7 | Scenario 8 | Scenario 9 |

- [ ] At least 50 usable frames per class per scenario (100 per scenario).
- [ ] **At least 3 sessions per class per scenario.** The Reliability Consistency Matrix needs the
      standard deviation of Accuracy and FNR across repeated sessions per scenario.
- [ ] Debris scenarios use the **test layouts only**. Smoke: introduced until optical visibility
      is gone.
- [ ] Test sessions are marked `test` in the log when recorded, and never moved to training.

## 4. Each session

1. Set up the scene. Mark the subject's position and measure it with the laser (ground truth
   for the Localization Acceptability Matrix); write it in the log.
2. Check the live app is using the frozen cfg.
3. Start the live app. Hold the device at 1.0–1.2 m, pointed at the zone, as in real use.
4. Hold still for at least 2 s, until **HOLD STEADY** is gone from the screen, before the window
   counts.
5. Record at least **10 s** (about 180 frames at 55 ms). Frames the motion gate refuses are
   still saved and are left out when features are extracted, so record longer if the operator
   had to steady the device more than once.
6. Close the live app. The recording is complete only once the app has closed.
7. **Save the session straight away.** The next session overwrites it:

   ```bash
   S=~/sarwave_data/sessions/S012_c1_zB_sit_still_open      # the session's name, section 5
   mkdir -p "$S" && cp ~/.local/share/Sarwave/default_session.{swrec,swimu} "$S"/
   ```

   Both files are needed: the `.swimu` next to the `.swrec` is what the motion gate and tilt
   correction replay from.
8. Fill in the session's line in the log.

To replay a saved session in the recorded app, copy its two files back to
`~/.local/share/Sarwave/` as `default_session.swrec` / `.swimu`, with the frozen cfg selected.

## 5. Dataset folder and session log

```
~/sarwave_data/
  cfg/            the frozen cfg, and COMMIT.txt with the git commit it came from
  layouts/        a photo and sketch of each debris layout (L01, L02, ...)
  sessions/       one folder per session: default_session.swrec + default_session.swimu
  session_log.csv
```

Session names: `S<number>_c<class>_z<zone>_<posture>_<state>_<environment>`, e.g.
`S012_c1_zB_sit_still_open`, `S013_c0_zB_none_empty_debrisL02`.

`session_log.csv` columns:

| Column | Example |
|---|---|
| session | S012 |
| date, time | 2026-11-03, 14:20 |
| operator | the operator's initials |
| subject | P2 (none for Class 0) |
| class | 1 |
| zone, distance_m (laser) | B, 3.10 |
| position_x_m, position_y_m (laser) | 0.40, 3.08 |
| posture, state | stand / sit / prone / curled, still / slow |
| environment, layout | open / debris / smoke / clutter, L02 |
| split | train / val / test |
| notes | e.g. "steadied twice", "door opened at 6 s" |

Back the folder up to the laptop at the end of each day.

## 6. Splitting

- [ ] **By session, never by frame.** All frames of a session go to one subset; consecutive
      frames are too alike to separate.
- [ ] About 70 % train, 15 % validation, 15 % test of the sessions in section 2, balanced by
      class, zone, environment and posture; decide the split before training and write it in
      the log.
- [ ] The section 3 test matrix stays separate from all of these.
- [ ] Each subject spread across subsets where possible, and noted, so a result is not just one
      subject's.
- [ ] Results reported **per posture and per state** (still / slow-moving) as well as overall,
      so a weak curled-up or still case does not hide behind good standing results.

## Do not

- Change the cfg, the radar firmware, the IMU mounting or calibration, or the enclosure during
  collection. If one has to change, the sessions before and after cannot be mixed.
- Record one class handheld and the other on a table, or in different rooms or on different
  days only.
- Reuse a test debris layout for training, or move a test session into training.
- Replay old sessions with a different cfg selected.
