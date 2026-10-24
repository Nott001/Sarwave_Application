# C# ↔ C++ parity

`CoreParityHarness` (C++) and `csharp/` (C#, built against the original
`Sarwave.Core.MMWave`, `Sarwave.Core.Imu` and `Sarwave.Integration.PeopleTracking` sources of the
C# solution) print the same text for the same input.
The C# projects expect the C# solution next to this repository (`../SarwaveApplication`);
pass `-p:SarwaveCSharpRoot=<path>` to `dotnet build` if it is elsewhere. Diffing the two
is how the port was verified to decode, track, log, record and replay exactly like the C# code.

```bash
# C++
cmake --build build --target CoreParityHarness
H=build/src/tests/CoreParityHarness
# C# (needs the .NET 10 SDK)
dotnet build src/tests/Parity/csharp -c Release -o /tmp/parity-cs
C="dotnet /tmp/parity-cs/ParityHarness.dll"

python3 tools/generate_recording.py /tmp/s.swrec --frames 600
for cmd in "log /tmp/s.swrec 0" "log /tmp/s.swrec 1" "manager /tmp/s.swrec 1000" "manager /tmp/s.swrec 120"; do
    diff <($C $cmd) <($H $cmd) && echo "same: $cmd"
done

# The manager driven by the IMU, as a replay is: the frame at t is gated by the IMU recording at
# offset + t, and targets are moved in the cfg's tracker frame.
python3 tools/generate_imu_recording.py /tmp/a.swimu --seconds 60 --seed 7
for off in 0 5000 12000 15000 19000; do
    cmd="imumanager /tmp/s.swrec /tmp/a.swimu 1000 $off configs/Sarwave_12m.cfg"
    diff <($C $cmd 2>/dev/null) <($H $cmd 2>/dev/null) && echo "same: $cmd"
done
```

`reader` prints a `malformed <Cause> <frame>: <detail> skipped=<n>` line for every packet the
reader skips. Damaged streams come from `tools/generate_stream.py`.

Commands: `log <rec> <hideBytes>`, `manager <rec> <retentionMs>`,
`imumanager <rec> <swimu> <retentionMs> <offsetMs> <cfg>`, `lanformat <swimu>`, `reader <raw> <seed>`,
`load <rec>`, `rewrite <in> <out>`, `playback <rec> <speed> <n>`, `prompt`, `timespan`, `fixed`.

Point order inside one target's group is not compared: the C# code sorts with an unstable
introsort, so that order is unspecified there.

The fixtures in `src/tests/data/parity` hold C# output captured this way; `CoreTests` compares
against them on every build, without needing .NET.

## The .cfg model

`ConfigParityHarness` (C++) and `csharp-config/` (C#) take `<corpus.txt> <cfg dir>... -- <cfg file>...`
and dump every parsed line, typed property, written-back text (LF and CRLF) and error message.
`data/config/golden_cs.txt` is the C# output for `data/config/corpus.txt` and `configs/`;
`ConfigTests` requires the C++ output to equal it.

## The IMU library

`ImuParityHarness` (C++) and `csharp-imu/` (C#, compiled from the original `Sarwave.Core.Imu`)
take `process <swimu> <every>`, `track <swimu> <every>`, `gate <swimu> <framePeriodMs>`,
`rewrite <in> <out>` and `mount`. Their output is identical to the last printed digit. That needs `Numerics.hpp` to
multiply quaternions as .NET 10's vectorised code does, `MathF.Atan2` computed in double, and
`-ffp-contract=off`.

```bash
cmake --build build --target ImuParityHarness
dotnet build src/tests/Parity/csharp-imu -c Release -o /tmp/parity-imu
python3 tools/generate_imu_recording.py /tmp/a.swimu --seconds 60 --seed 7
for cmd in "process /tmp/a.swimu 1" "track /tmp/a.swimu 1" "gate /tmp/a.swimu 55" mount; do
    diff <(dotnet /tmp/parity-imu/ImuParityHarness.dll $cmd) <(build/src/tests/ImuParityHarness $cmd) && echo "same: $cmd"
done
```

`src/tests/data/imu` holds a session and the C# output for it; `ImuTests` compares on every build.
