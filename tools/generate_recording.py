#!/usr/bin/env python3
"""Writes a synthetic 3D People Tracking session in the .swrec recording format.

TestRpiApp (like the C# Sarwave.Source.TestRpiApp) replays `default_session.swrec` from its
working directory; the live app (RunStuff) records one from the sensor. Without a sensor at hand,
this script produces a believable stand-in:

    python3 tools/generate_recording.py default_session.swrec
    python3 tools/generate_recording.py out.swrec --frames 400 --seed 7

Every frame carries the TLVs the IWR6843 people tracking demo sends: target list (1010), target
index (1011, one entry per point of the *previous* frame's cloud), target height (1012), point
cloud (1020) and presence (1021). People walk in and out of view, so track ids get freed and
reused, points are sometimes rejected (253/254/255), and the odd frame comes without a cloud or
without targets.

Format (little endian): "SWMMWREC", int32 version 1, then per frame: int64 timestamp in 100 ns
ticks, int32 length, the raw frame (magic word 02 01 04 03 06 05 08 07 + 32-byte header + TLVs).
"""

import argparse
import math
import random
import struct

MAGIC = bytes([0x02, 0x01, 0x04, 0x03, 0x06, 0x05, 0x08, 0x07])

TLV_TARGET_LIST = 1010
TLV_TARGET_INDEX = 1011
TLV_TARGET_HEIGHT = 1012
TLV_POINT_CLOUD = 1020
TLV_PRESENCE = 1021

# Scale factors of the compressed point cloud (as the demo uses them).
ELEV_UNIT = 0.01
AZIM_UNIT = 0.01
DOPPLER_UNIT = 0.05
RANGE_UNIT = 0.00025
SNR_UNIT = 0.04


def tlv(tlv_type, payload):
    return struct.pack("<II", tlv_type, len(payload)) + payload


def frame_bytes(frame_number, tlvs, num_points):
    body = b"".join(tlvs)
    total = len(MAGIC) + 32 + len(body)
    header = struct.pack(
        "<8I",
        0x03060000,  # version
        total,  # totalPacketLen
        0x0A6843,  # platform
        frame_number,
        frame_number * 1_650_000 & 0xFFFFFFFF,  # timeCpuCycles
        num_points,
        len(tlvs),
        0,
    )
    return MAGIC + header + body


class Person:
    def __init__(self, rng, tid):
        self.tid = tid
        self.x = rng.uniform(-2.5, 2.5)
        self.y = rng.uniform(1.0, 6.5)
        heading = rng.uniform(0, 2 * math.pi)
        speed = rng.uniform(0.2, 1.1)
        self.vx = speed * math.sin(heading)
        self.vy = speed * math.cos(heading)
        self.height = rng.uniform(1.1, 1.9)
        self.life = rng.randint(60, 220)  # frames before walking off
        self.confidence = rng.uniform(0.3, 0.99)

    def step(self, rng, dt):
        self.vx += rng.gauss(0, 0.15) * dt
        self.vy += rng.gauss(0, 0.15) * dt
        self.x += self.vx * dt
        self.y += self.vy * dt
        if not -4 < self.x < 4:
            self.vx = -self.vx
        if not 0.5 < self.y < 8:
            self.vy = -self.vy
        self.life -= 1
        self.confidence = min(0.99, max(0.05, self.confidence + rng.gauss(0, 0.02)))


def compress_point(rng, x, y, z, doppler):
    rng_m = math.sqrt(x * x + y * y + z * z)
    azim = math.atan2(x, y)
    elev = math.asin(z / rng_m) if rng_m > 0 else 0.0
    clamp = lambda v, lo, hi: max(lo, min(hi, v))
    return struct.pack(
        "<bbhhh",
        clamp(round(elev / ELEV_UNIT), -128, 127),
        clamp(round(azim / AZIM_UNIT), -128, 127),
        clamp(round(doppler / DOPPLER_UNIT), -32768, 32767),
        clamp(round(rng_m / RANGE_UNIT), -32768, 32767),
        clamp(round(rng.uniform(6, 40) / SNR_UNIT), -32768, 32767),
    )


def generate(frames, seed, period_ms):
    rng = random.Random(seed)
    people = []
    free_ids = list(range(0, 8))
    prev_point_owner = []  # owner wire id (or 253..255) of each point of the previous cloud
    sensor_height = 2.0
    out = []
    t_ticks = 0

    for n in range(frames):
        dt = period_ms / 1000.0
        # People come and go; freed track ids are reused by the tracker.
        for p in people:
            p.step(rng, dt)
        for p in [p for p in people if p.life <= 0]:
            people.remove(p)
            free_ids.append(p.tid)
        if free_ids and len(people) < 4 and rng.random() < 0.03:
            free_ids.sort()
            people.append(Person(rng, free_ids.pop(0)))

        tlvs = []
        num_points = 0
        drop_targets = rng.random() < 0.02
        drop_cloud = rng.random() < 0.03

        if not drop_targets and people:
            records = b""
            heights = b""
            for p in people:
                ec = [rng.uniform(-0.01, 0.01) for _ in range(16)]
                records += struct.pack(
                    "<B3x9f16f2f",
                    p.tid,
                    p.x, p.y, p.height / 2,
                    p.vx, p.vy, 0.0,
                    rng.gauss(0, 0.1), rng.gauss(0, 0.1), 0.0,
                    *ec,
                    rng.uniform(1, 4),
                    p.confidence,
                )
                heights += struct.pack("<B3xff", p.tid, p.height + rng.uniform(-0.05, 0.05), rng.uniform(0.0, 0.2))
            tlvs.append(tlv(TLV_TARGET_LIST, records))

            # Index TLV: owners of the previous frame's points, by this frame's track ids.
            if prev_point_owner:
                live = {p.tid for p in people}
                owners = bytes(o if (o >= 253 or o in live) else 255 for o in prev_point_owner)
                tlvs.append(tlv(TLV_TARGET_INDEX, owners))
            tlvs.append(tlv(TLV_TARGET_HEIGHT, heights))

        point_owner = []
        if not drop_cloud:
            points = b""
            for p in people:
                for _ in range(rng.randint(4, 18)):
                    px = p.x + rng.gauss(0, 0.18)
                    py = p.y + rng.gauss(0, 0.18)
                    pz = rng.uniform(0.1, p.height) - sensor_height
                    points += compress_point(rng, px, py, pz, (p.vy if py else 0) + rng.gauss(0, 0.1))
                    point_owner.append(p.tid if rng.random() > 0.08 else rng.choice([253, 254, 255]))
            for _ in range(rng.randint(0, 9)):  # clutter
                points += compress_point(
                    rng, rng.uniform(-5, 5), rng.uniform(0.3, 9), rng.uniform(-2, 0.8), rng.gauss(0, 0.3)
                )
                point_owner.append(rng.choice([253, 254, 255]))
            unit = struct.pack("<5f", ELEV_UNIT, AZIM_UNIT, DOPPLER_UNIT, RANGE_UNIT, SNR_UNIT)
            tlvs.append(tlv(TLV_POINT_CLOUD, unit + points))
            num_points = len(point_owner)

        tlvs.append(tlv(TLV_PRESENCE, struct.pack("<I", 1 if people else 0)))

        # Index entries only make sense against a cloud that was actually sent.
        prev_point_owner = point_owner if not drop_targets else []

        out.append((t_ticks, frame_bytes(n, tlvs, num_points)))
        jitter = rng.uniform(-3, 3)
        t_ticks += int((period_ms + jitter) * 10_000)

    return out


def write_recording(path, frames):
    with open(path, "wb") as f:
        f.write(b"SWMMWREC" + struct.pack("<i", 1))
        for ticks, data in frames:
            f.write(struct.pack("<qi", ticks, len(data)))
            f.write(data)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("output", help="the .swrec file to write")
    parser.add_argument("--frames", type=int, default=600, help="number of frames (default 600)")
    parser.add_argument("--seed", type=int, default=1, help="random seed (default 1)")
    parser.add_argument("--period-ms", type=float, default=55.0, help="frame period in ms (default 55)")
    args = parser.parse_args()

    frames = generate(args.frames, args.seed, args.period_ms)
    write_recording(args.output, frames)
    print(f"wrote {len(frames)} frames to {args.output}")


if __name__ == "__main__":
    main()
