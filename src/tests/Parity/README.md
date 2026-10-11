# C# ↔ C++ parity

`CoreParityHarness` (C++) and `csharp/` (C#, built against the original
`Sarwave.Core.MMWave` sources of the C# solution) print the same text for the same input.
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
```

Commands: `log <rec> <hideBytes>`, `manager <rec> <retentionMs>`, `reader <raw> <seed>`,
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
