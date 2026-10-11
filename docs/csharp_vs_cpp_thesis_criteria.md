# C# vs C++ Against the Thesis Criteria

How the two implementations of the Sarwave application (`SarwaveApplication/`, C#, and this
repository, C++) compare against the quality criteria in `docs/thesis_context.md`.

## What the manuscript specifies

- **Software requirement (Conceptual Framework):** Python 3.11, scikit-learn (SVM), NumPy/SciPy,
  DBSCAN, RPLCD (LCD driver), Raspberry Pi OS (64-bit).
- **System construction procedure:** "complete real-time processing pipeline in Python 3.11 on
  the Raspberry Pi 5".
- **Instruments:** latency logged "via timestamps and system monitors embedded natively in the
  Python pipeline", using Python's `time.perf_counter()`.

The evaluated pipeline (clutter removal → DBSCAN → feature extraction → RBF-SVM → centroid
localization → I2C LCD) is implemented in **neither** the C# nor the C++ project. Both cover
radar acquisition, TLV decoding of TI's tracker output, recording/replay, and the dashboard. If the
SVM pipeline moves out of Python, Chapter III (software requirements, instruments, latency
measurement) must be revised to match.

## Comparison

| Criterion in the manuscript | C# | C++ |
|---|---|---|
| Processing time 30–70 ms, under 100 ms per frame | Easily met: decoding a frame takes far less than 1 ms | Easily met; smaller and steadier, no garbage-collection pauses |
| Reliability: "zero pipeline failures during testing" | Managed memory is safer in principle, but the C# code uses unsafe native memory and has a real double free (the terminal app) | Memory is managed automatically; covered by serial and C# parity tests |
| Dev machine and Pi give identical SVM results ("100% classification consistency") | ML.NET has no RBF SVM; the decision function would be hand-coded or a third-party library used | scikit-learn's SVC is built on libsvm, so libsvm in C++ gives the closest match |
| I2C LCD on the Pi | Microsoft's `Iot.Device.Bindings` has an I2C character-LCD driver | Raw `/dev/i2c` or a small library; more work |
| Deploying on Pi 5 | Needs the .NET runtime (supports 64-bit ARM) | Needs Qt 6.5+: fine on current Trixie-based Pi OS; older Bookworm ships Qt 6.4, which is too old |
| Team can maintain and defend it | Yes: it is the version the team has been developing | The team would be defending code it did not write |

## Recommendation

1. **Keep the SVM pipeline in Python, as the manuscript states.** Training and deployment use the
   same scikit-learn, which makes the cross-platform consistency check trivial. Use the C# or C++
   application only as the radar reader and dashboard, or leave it out of the thesis scope.
2. **If the deployed application must be one of the two, choose C#.** It is the version the team
   knows and will have to explain at the defense; performance does not separate the two at these
   latency targets. C++ is preferable only if the SVM is deployed natively (libsvm's exact match
   with scikit-learn) or the smallest footprint on the Pi is required.

None of the figures above were measured on a Raspberry Pi 5. If latency on the Pi matters to the
defense, profile it on the device.
