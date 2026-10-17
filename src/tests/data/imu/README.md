# IMU parity fixtures

`session.swimu` is `tools/generate_imu_recording.py session.swimu --seconds 30 --seed 4`: lying
still, tremor, pitch and roll, sweeps, a flick, a knock, walking and a sampler stall. The text
files are what the **original C# `Sarwave.Core.Imu`** prints for it through
`src/tests/Parity/csharp-imu` (the twin of `ImuParityHarness.cpp`):

| file | command |
|---|---|
| `session.process10.txt` | `process session.swimu 10` -- every 10th processed state |
| `session.gate55.txt` | `gate session.swimu 55` -- the motion gate's verdict on a 55 ms frame stream |
| `mount.txt` | `mount` -- `ImuMounting.FromAxes` and level calibration for every axis triple |

`ImuTests` requires the C++ library to print exactly the same text.
