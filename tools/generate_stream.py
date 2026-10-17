#!/usr/bin/env python3
"""Writes a raw data-port byte stream for testing frame reassembly: People Tracking frames from
generate_recording.py, interleaved with every kind of damage a serial link produces.

Between and inside the frames, deterministically from the seed: garbage bytes, false and partial
magic words, headers with impossible lengths (too small, too large, not a multiple of 32), bytes
lost from the middle of a TLV, from the padding and from the end of a packet (so the next
magic word arrives inside it), and whole frames lost.

Usage: generate_stream.py <out.bin> [--frames N] [--seed S]
"""

import argparse
import random
import struct

from generate_recording import MAGIC, generate


def damage(rng, frame):
    kind = rng.random()
    if kind < 0.06:  # lost bytes inside the TLVs
        at = rng.randrange(48, max(49, len(frame) - 40))
        n = rng.randint(1, 12)
        return frame[:at] + frame[at + n:], "mid"
    if kind < 0.10:  # lost bytes in the padding / last TLV
        n = rng.randint(1, 7)
        return frame[:-n], "tail"
    if kind < 0.14:  # cut short: the next magic word arrives inside it
        return frame[: rng.randint(20, len(frame) - 9)], "cut"
    if kind < 0.17:  # a header announcing an impossible length
        bad = rng.choice([5, 1 << 21, len(frame) + 3])
        return frame[:12] + struct.pack("<I", bad) + frame[16:], "header"
    if kind < 0.19:  # lost entirely
        return b"", "lost"
    return frame, "ok"


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("output")
    parser.add_argument("--frames", type=int, default=120)
    parser.add_argument("--seed", type=int, default=5)
    args = parser.parse_args()

    rng = random.Random(args.seed)
    out = bytearray()
    for _, frame in generate(args.frames, args.seed, 55.0):
        r = rng.random()
        if r < 0.10:
            out += bytes(rng.randrange(256) for _ in range(rng.randint(1, 30)))
        elif r < 0.15:
            out += MAGIC[: rng.randint(1, 7)]  # a partial magic word
        elif r < 0.18:
            out += MAGIC[:5] + bytes([0xEE]) + MAGIC[6:]  # a false one
        frame, _ = damage(rng, frame)
        out += frame
    open(args.output, "wb").write(out)


if __name__ == "__main__":
    main()
