# Real-time thread priority on the Raspberry Pi

## What it is

By default Linux shares the CPU fairly among all running threads. The thread that reads the
radar's serial port then competes with the GUI, the desktop and every other program. If it waits
too long for its turn, the small serial buffer fills up and bytes are lost, which is how frames
get damaged.

Real-time priority is a Linux setting that lets a thread run the moment it has work, ahead of
every normal thread. `LiveRpiApp` asks for it on two threads:

| Thread | Priority | Why |
|---|---|---|
| Serial-port reader | 30 | Must empty the serial buffer as soon as radar data arrives |
| IMU sampler | 20 | Reads the IMU every 10 ms so the motion check stays on time |

Both threads sleep almost all the time and wake only briefly when data arrives, so they can't
hog the CPU. Both priorities sit below the kernel's own interrupt threads, which run at 50.

In the code: `ConstantCapture::readerPriority` and `ImuSampler::priority`, set in
`include/DevEnv/Gui/RunStuff.cpp`.

## Allowing it (once per Pi)

Linux doesn't let normal programs use real-time priority unless allowed. Run this on the Pi,
then log out and back in:

```bash
echo "$USER - rtprio 30" | sudo tee /etc/security/limits.d/sarwave.conf
```

This allows your user's programs to use priorities up to 30.

## Without it

The app still works, with both threads at normal priority. At start it prints:

```
[00:00:00.012] Note: the data-port reader runs at normal priority. To let it run real-time, allow it once with
    echo "$USER - rtprio 30" | sudo tee /etc/security/limits.d/sarwave.conf
and log in again.
```

## Checking it worked

1. `ulimit -r` should print `30` after you log back in.
2. `LiveRpiApp` should no longer print that note.
3. While it runs, list its threads:

   ```bash
   ps -L -o tid,cls,rtprio,comm -p "$(pidof LiveRpiApp)"
   ```

   Two threads should show class `FF` (real-time FIFO), one with priority 30 and one with 20.
   The rest show `TS` (normal).

If `ulimit -r` still prints `0` after logging back in (some desktop auto-logins skip the limits
file), reboot. As a fallback, give the program itself the permission. This has to be repeated
after every rebuild:

```bash
sudo setcap cap_sys_nice+ep build/bin/LiveRpiApp
```

## For the thesis

It makes a damaged-frame comparison possible. Record a long session without the setting and one
with it, then compare the `Malformed frame` counts. If they drop, the damaged frames were caused
by the reader being delayed rather than by power or cabling. See
[pi_frame_corruption.md](pi_frame_corruption.md) for the other causes and how to check them.
