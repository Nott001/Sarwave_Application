# Frame corruption on the Raspberry Pi

The C++ counterpart of the C# solution's `docs/pi_frame_corruption.md`. That note lists the
possible causes and fixes. This one says what the C++ port does about each, and what is left to
check on the hardware.

## The problem

Frames recorded on the Raspberry Pi 5 arrive damaged or go missing; frames recorded on a laptop
don't (`recorded_data.txt`, "Damaged and Missing Frames"). In Pi recording D, frame 341 lost
7 bytes from the middle of its Target List TLV, frame 342 was lost because frame 341 swallowed
its Magic Word, and frame 355 never arrived.

The bytes were **lost**, not changed. That points at data dropped between the radar and the
program, by the USB link or because nobody read the port in time. A wrong baud rate or electrical
noise would garble bytes instead.

## Part 1: never freeze, always report (done, same as C#)

A damaged packet can no longer stop the app, crash it, or leave the tracker in a wrong state:

| Where | What it does |
|---|---|
| `FrameReader` | Rejects a packet whose header is implausible (length too small, too large, or not a multiple of 32), whose TLVs don't chain to its end, or that the next Magic Word interrupts. Throws `FrameReaderMalformedException` (`Header`, `TlvChain`, `Interrupted`) *after* skipping, so reading just continues. Only the bad packet's Magic Word is given up, so the packet after it is still found, and a partial Magic Word borrowed at the end of a short packet is left for the next one. |
| `EntryPackage::unpack` | Rejects a frame the device can't have sent: unknown or duplicate TLV, wrong record size, point count ≠ `numDetectedObj`, track ID > 249, heights that don't match the target list, an invalid point index, presence other than 0/1, TLVs truncated. Throws `FrameEntryMalformedException` before anything is applied. |
| `CombinedManager` | A gap in `frameNumber` (or `skipFrame()`) means the next Target Index TLV is not applied to an older cloud, which used to attach points to the wrong people. |
| `ConstantCapture` / `FramePlayback` | Hand both kinds of error to a `MalformedCallback` and carry on. Any other exception still ends the capture and is reported through `error()` / `onCompleted`. |
| `FrameRecorder` | `recordMalformed()` writes a note into the recording (`.swrec` version 2), so a replay shows the same errors at the same moments. Each frame is flushed to the OS at once, so a crash loses nothing. |
| `RunStuff` | Prints one line per malformed frame (`[00:01:23.456] Malformed frame: FrameReaderMalformedException (TlvChain): Frame 341: ...`), records it and calls `RadarData::skipFrame`. |

These checks stay even after the cause is fixed. They cost almost nothing, and the malformed-frame
count is how a fix is confirmed.

## Part 2: stop the corruption (implemented in the C++ port)

The C# note proposes these; the C++ port implements the software ones.

### Cause 2, the capture loop stopped reading while it handled each frame: fixed

Before this change, `ConstantCapture` read a frame, ran the callback (print, SD-card write, GUI,
IMU) and only then read again. Nobody read the port during the callback, and on the slower Pi
those pauses could overflow the kernel's serial buffer.

**`ConstantCapture` now runs two threads joined by a bounded queue:**

```
reader thread:  read frame -> stamp it on arrival -> queue it -> read the next frame at once
                                                       |
worker thread:      take from queue -> frame callback (print, record, IMU, display) -> dispose
```

- The reader never waits on the display, the SD card or the terminal, so the kernel buffer is
  always drained.
- Frames are timestamped **when they arrive**, not when they are processed, so the IMU stays
  lined up with them even when the worker falls behind.
- Skipped packets (`FrameReaderMalformedException`) go through the same queue. Frames and errors
  reach the callbacks in order, on the one worker thread, so `RadarData` is still only touched
  from one thread.
- If the worker falls far behind and the queue fills (`queueCapacity`, 32 frames ≈ 2 s), the
  oldest frame is dropped and reported through `onDropped` as **"Processing too slow: N frame(s)
  dropped unprocessed"**. That is kept separate from "Malformed frame", so the two problems
  can't be confused.
- `stopConstantCapture()` stops both threads and disposes whatever is still queued. Calling it
  from inside a callback still only signals.

The callbacks in `RunStuff` / `RunRecorded` did not change shape. The serial tests drive this
against a fake IWR6843 on pseudo-terminals, including a callback five times slower than the
frame rate (no malformed frames, overflow reported apart) and packets damaged the way recording D
was (reported in order, capture carries on).

### Verbose printing during recording: off by default

In C#, `RunStuff.OnOpened(window, save)` printed every frame in full whenever `save` was true.
On a Pi that printing is slow. The C++ `RunStuff::onOpened(window, engine, printFrames)` names
the flag for what it does, defaults it to off, and `LiveRpiApp` turns it on only with
`--print-frames`. Recording always happens; printing is separate.

### Cause 3, the serial driver: ruled out for this app

Boost.Asio puts the port into raw mode when `Context` opens it (no `ICRNL`, `IXON`, `OPOST`,
`ICANON`, `ECHO`, `ISIG`; 8N1, no flow control), so Linux cannot change or swallow bytes. You
can still check it while the app runs: `stty -F /dev/ttyUSB1 -a` should show
`-icrnl -ixon -ixoff -opost -isig -icanon -echo`.

### Bit-exact maths on the Pi

`-ffp-contract=off` is set project-wide. GCC fuses `a * b + c` into FMA instructions by default
on aarch64 but not on x86, so the Pi build would otherwise round the IMU and point-levelling
maths differently from the PC build and from the C# app.

## Part 3: on the hardware (still to do; software can't fix these)

1. **Power, the most likely cause.** A Pi 5 limits its USB ports to 600 mA in total unless it runs
   on the official 5 A supply or `usb_max_current_enable=1` is set in `/boot/firmware/config.txt`.
   While streaming, check `vcgencmd get_throttled` (anything but `0x0` means under-voltage) and
   `dmesg -w` (under-voltage, over-current, `usb ... reset`, `cp210x`/`xhci` errors). Fix: power
   the radar from its own supply or a powered hub.
2. **Driver error counters**, run after a session, before unplugging the radar:

   ```python
   import fcntl, struct, sys
   # struct serial_icounter_struct: cts dsr rng dcd rx tx frame overrun parity brk buf_overrun ...
   fd = open(sys.argv[1], 'rb', buffering=0)
   v = struct.unpack('20i', fcntl.ioctl(fd, 0x545D, bytes(80)))  # TIOCGICOUNT
   print(dict(zip('cts dsr rng dcd rx tx frame overrun parity brk buf_overrun'.split(), v)))
   ```

   `overrun`/`buf_overrun` going up means data arrived faster than it was read, which should now
   be gone. `frame`/`parity` going up means garbled characters (baud rate, noise).
3. **Cabling**: a short, good USB cable on a USB 2 port, and the IMU's jumper wires routed away
   from the radar.

## Confirming the fix

1. Record a long session on the Pi with `LiveRpiApp` (no `--print-frames`).
2. Count the `Malformed frame` lines on stderr, or the notes in the recording
   (`FrameRecording::malformedFrames()`). Count `Processing too slow` lines separately.
3. Zero malformed frames over a long session means the link is clean. Malformed frames with
   `vcgencmd get_throttled` ≠ `0x0` point at power.
