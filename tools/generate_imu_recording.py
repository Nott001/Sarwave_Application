#!/usr/bin/env python3
"""Writes a synthetic .swimu IMU recording (the C# and C++ apps' format) for a handheld session:
lying still, held with tremor, pitched and rolled, slow and fast sweeps, a knock, a walk-like
linear acceleration and a gap where the sampler stalled. Raw IMU axes (identity mounting),
200 Hz with the chip's slight timing jitter.

Format (little endian): "SWIMUREC", int32 version 1, float32 rate; per sample int64 timestamp in
100 ns ticks, float32 x3 acceleration (g), float32 x3 angular rate (deg/s), float32 temperature.

Usage: generate_imu_recording.py <out.swimu> [--seconds S] [--seed N]
"""

import argparse
import math
import random
import struct


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("output")
    parser.add_argument("--seconds", type=float, default=30.0)
    parser.add_argument("--seed", type=int, default=1)
    args = parser.parse_args()

    rng = random.Random(args.seed)
    rate = 200.0
    out = bytearray(b"SWIMUREC" + struct.pack("<if", 1, rate))
    pitch = 0.0  # radians, integrated from the gyro so the accelerometer agrees with it
    roll = 0.0
    t = 0.0123
    bias = (0.3, -0.2, 0.1)
    while t < args.seconds:
        s = t % 30.0
        rate_x = rate_y = rate_z = 0.0
        lin = [0.0, 0.0, 0.0]
        if 3 <= s < 6:  # held: 8 Hz tremor
            rate_x = 6 * math.sin(2 * math.pi * 8 * s)
            rate_z = 7 * math.sin(2 * math.pi * 9 * s + 1)
            lin[0] = 0.03 * math.sin(2 * math.pi * 8 * s)
        elif 6 <= s < 7:  # pitch up 15 degrees
            rate_x = 15.0
        elif 9 <= s < 10:  # roll right side down 10 degrees
            rate_y = 10.0
        elif 12 <= s < 13.5:  # slow sweep
            rate_z = 20.0
        elif 15 <= s < 15.3:  # fast flick
            rate_z = 120.0
        elif 17 <= s < 17.02:  # knock
            lin[2] = 0.6
        elif 19 <= s < 21:  # walking
            lin[1] = 0.15 * math.sin(2 * math.pi * 1.8 * s) + 0.1
        elif 24 <= s < 25:  # level again
            rate_x = -15.0
            rate_y = -10.0

        dt = 1 / rate
        pitch += math.radians(rate_x) * dt
        roll += math.radians(rate_y) * dt
        # Gravity seen by a device pitched up by `pitch` and rolled right-down by `roll`.
        g = (-math.sin(roll) * math.cos(pitch), math.sin(pitch), math.cos(roll) * math.cos(pitch))
        acc = [g[i] + lin[i] + rng.gauss(0, 0.004) for i in range(3)]
        gyro = [rate_x + bias[0] + rng.gauss(0, 0.08), rate_y + bias[1] + rng.gauss(0, 0.08),
                rate_z + bias[2] + rng.gauss(0, 0.08)]

        if not (22.0 <= s < 22.12):  # the sampler stalled: no samples here
            ticks = int(round(t * 10_000_000))
            out += struct.pack("<q7f", ticks, *acc, *gyro, 27.0 + 0.01 * s)
        t += dt * (1 + rng.uniform(-0.002, 0.002))

    open(args.output, "wb").write(out)


if __name__ == "__main__":
    main()
