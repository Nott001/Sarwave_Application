#!/usr/bin/env python3
"""Rewrites a hand-built .swrec so every frame has the layout the IWR6843 People Tracking demo
really sends, without changing what the frames say:

  - a Target Height TLV (1012) after every Target List (1010) that lacks one, for the same
    targets in the same order (maxZ 1.75 m, minZ 0.15 m);
  - numDetectedObj equal to the number of points in the Point Cloud TLV;
  - a presence value other than 0 or 1 written as 1 (what the decoder used to make of it);
  - the packet padded with 0x0F to a multiple of 32 bytes.

Since the malformed-frame checks (EntryPackage, FrameReader) such frames are the only ones the
decoder accepts. Usage: fixup_recording.py <in.swrec> <out.swrec>
"""

import struct
import sys

MAGIC = bytes([2, 1, 4, 3, 6, 5, 8, 7])


def fix_frame(frame):
    header = list(struct.unpack("<8I", frame[8:40]))
    tlvs = []
    offset = 40
    for _ in range(header[6]):
        tlv_type, length = struct.unpack("<II", frame[offset:offset + 8])
        tlvs.append((tlv_type, frame[offset + 8:offset + 8 + length]))
        offset += 8 + length

    types = [t for t, _ in tlvs]
    out = []
    points = 0
    for tlv_type, payload in tlvs:
        if tlv_type == 1021 and len(payload) == 4 and struct.unpack("<I", payload)[0] > 1:
            payload = struct.pack("<I", 1)
        out.append((tlv_type, payload))
        if tlv_type == 1010 and 1012 not in types:
            ids = [payload[i] for i in range(0, len(payload), 112)]
            out.append((1012, b"".join(struct.pack("<B3xff", tid, 1.75, 0.15) for tid in ids)))
        if tlv_type == 1020:
            points = (len(payload) - 20) // 8

    body = b"".join(struct.pack("<II", t, len(p)) + p for t, p in out)
    total = (40 + len(body) + 31) // 32 * 32
    header[1] = total
    header[5] = points
    header[6] = len(out)
    packet = MAGIC + struct.pack("<8I", *header) + body
    return packet + b"\x0f" * (total - len(packet))


def main():
    data = open(sys.argv[1], "rb").read()
    assert data[:8] == b"SWMMWREC", "not a recording"
    out = bytearray(data[:12])
    offset = 12
    while offset < len(data):
        ticks, length = struct.unpack("<qi", data[offset:offset + 12])
        frame = fix_frame(data[offset + 12:offset + 12 + length])
        out += struct.pack("<qi", ticks, len(frame)) + frame
        offset += 12 + length
    open(sys.argv[2], "wb").write(out)


if __name__ == "__main__":
    main()
