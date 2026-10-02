#!/usr/bin/env python3
"""Signature-preserving runtime patcher for the UMS9620 (T820) uboot.

On a fused T820 the SPL RSA-verifies uboot, so a modified+rehashed uboot is
rejected (confirmed on hardware). This tool applies the "magic" method from
unisoc_chipram_signcheck (Patch-Post-Verification): it keeps the original signed
payload byte-for-byte (so the SPL's RSA check still passes) and appends a small
AArch64 shellcode plus a patch table. At boot the SPL loads the image, verifies
the untouched payload, and jumps to a branch we place at the entry; the
shellcode copies the payload back into place, applies the patch words, then
jumps to the real _start.

Layout produced (matches magic64.cpp):

    0x000  sys_img_header (DHTB, 0x200)   mImgSize += 0x100
    0x200  jump code (0x10)               B -> shellcode
    0x210  payload (mImgSize bytes)       UNCHANGED (RSA stays valid)
    ...    shellcode (0x70) + patch_data + pad   (0xF0 total)
    ...    sprdsignedimageheader footer (0x60)   payload_offset=0x210, cert_offset+=0x100
    ...    cert / priv / dbg data
    then truncated back to the original partition size (drops only zero padding)

Patches are given as (code_offset, new_word) where code_offset is the offset
into the uboot payload (i.e. file offset - 0x200). The runtime address written
by the shellcode is LOAD_BASE + code_offset.

LOAD_BASE default 0xb5000000 was confirmed on hardware for this uboot (the
payload text base; PAC XML image base 0xb4fffe00 + 0x200 header). If a different
build relocates elsewhere, pass --load-base.

The shellcode is assembled in pure Python (no binutils dependency), so the
output is byte-for-byte deterministic across machines.

Usage:
    python3 tools/magic_pack_ums9620.py <orig_uboot.img> <out.img> \
        --patch 0xafe8=0xd503201f [--patch off=word ...] [--load-base 0xb5000000]

Known patches for this uboot (see docs/UMS9620_PORT.md):
    0xafe8=0xd503201f   remove unlock-warning message + 10s power-button timeout
    0xb2a4=0xd503201f   remove "SKIP VERIFY!!!" warning (UART print)
    0xb2ac=0xd503201f   remove "SKIP VERIFY!!!" warning (screen print)
    0x604c0=0xd503201f  force get_lock_status = UNLOCK
"""

import argparse
import struct
import sys

DEFAULT_LOAD_BASE = 0xB5000000
ADD_LENGTH = 0x100        # (0x10 + 0x70 shellcode + 0xFF) & ~0xFF

# The magic64 relocation+patch shellcode is a fixed 28-instruction (0x70) AArch64
# routine (from unisoc_chipram_signcheck's magic64.cpp). Only six words depend on
# the payload size; the rest are constant. This template is the constant form
# (taken from a size=0x100000 build); the size-dependent slots are overwritten by
# build_shellcode(). Hand-assembling in Python keeps the output deterministic and
# avoids a binutils dependency. Slot map (word index -> what it encodes):
#   0  : adrp x9, label_init(=payload base)   depends on link pc = size+0x10
#   2  : movz x11, #hi16(size/8), lsl #16
#   3  : movk x11, #lo16(size/8)
#   13 : adrp x9, label_patch(=size+0x80)     depends on link pc = size+0x44
#   14 : add  x9, x9, #((size+0x80) & 0xfff)
#   27 : b .-(size+0x7c)                       jump back to the real _start
_SHELLCODE_TEMPLATE = [
    0x90000009, 0x9100412a, 0xd2a0004b, 0xf280000b, 0xd280000c, 0xeb0b018d,
    0x540000e2, 0xf86c794d, 0xf82c792d, 0xd508711f, 0xd5033fdf, 0x9100058c,
    0x17fffff9, 0x90000009, 0x91020129, 0xb840452a, 0xb400016a, 0xb840452b,
    0xd280000c, 0xeb0b018d, 0x54ffff62, 0xb840452d, 0xb800454d, 0xd508711f,
    0xd5033fdf, 0x9100058c, 0x17fffff9, 0x17fbffe1,
]


def _adrp(rd: int, pc: int, target: int) -> int:
    """Encode `adrp Xrd, <target>` executed at address `pc`."""
    imm = ((target & ~0xFFF) - (pc & ~0xFFF)) >> 12
    return 0x90000000 | ((imm & 3) << 29) | (((imm >> 2) & 0x7FFFF) << 5) | rd


def build_shellcode(size: int) -> bytes:
    """Build the magic64 relocation+patch shellcode for a given payload size."""
    n = size // 8
    w = list(_SHELLCODE_TEMPLATE)
    w[0] = _adrp(9, size + 0x10, 0)
    w[2] = 0xD2A00000 | (((n >> 16) & 0xFFFF) << 5) | 11
    w[3] = 0xF2800000 | ((n & 0xFFFF) << 5) | 11
    w[13] = _adrp(9, size + 0x44, size + 0x80)
    w[14] = 0x91000000 | (((size + 0x80) & 0xFFF) << 10) | 0x129
    off = -(size + 0x7C)
    w[27] = 0x14000000 | ((off >> 2) & 0x3FFFFFF)
    return struct.pack("<28I", *w)


def pack(data: bytearray, patches, load_base: int) -> bytes:
    if data[0:4] != b"DHTB":
        raise ValueError("not a DHTB image")
    size = struct.unpack_from("<I", data, 0x30)[0]

    foot_off = 0x200 + size
    footer = bytearray(data[foot_off:foot_off + 0x60])
    if footer[0:7] != b"SIMGHDR":
        raise ValueError("SIMGHDR footer not found")
    # stock footer payload_offset (footer+0x18) is 0x200; magic packing sets it
    # to 0x210, so that field is a reliable already-packed marker.
    if struct.unpack_from("<Q", footer, 0x18)[0] != 0x200:
        raise ValueError("image already magic-patched")

    shell = build_shellcode(size)

    # patch_data: [addr, len_words, word...] per entry, then zero terminator
    patch_data = b""
    for off, word in patches:
        patch_data += struct.pack("<III", load_base + off, 1, word)
    patch_data += struct.pack("<I", 0)
    if len(patch_data) > 0x80:
        raise ValueError(f"patch_data 0x{len(patch_data):x} exceeds 0x80; "
                         f"increase add_length (not implemented)")

    out = bytearray()
    hdr = bytearray(data[0:0x200])
    struct.pack_into("<I", hdr, 0x30, size + ADD_LENGTH)
    out += hdr
    jump = 0x14000000 | (((size + 0x10) // 4) & 0x3FFFFFF)
    out += struct.pack("<I", jump) + b"\x00" * 0x0C
    out += data[0x200:0x200 + size]                       # payload, unchanged
    magic = bytearray(ADD_LENGTH - 0x10)                  # 0xF0
    magic[0:0x70] = shell
    magic[0x70:0x70 + len(patch_data)] = patch_data
    out += magic
    struct.pack_into("<Q", footer, 0x18, 0x210)           # payload_offset
    for so, oo in [(0x20, 0x28), (0x30, 0x38), (0x40, 0x48), (0x50, 0x58)]:
        if struct.unpack_from("<Q", footer, so)[0]:
            struct.pack_into("<Q", footer, oo,
                             struct.unpack_from("<Q", footer, oo)[0] + ADD_LENGTH)
    out += footer
    out += data[foot_off + 0x60:]                         # cert + trailing padding

    # keep within the original partition size (drops only zero padding)
    part_size = len(data)
    if any(out[part_size:]):
        raise ValueError("packing would overflow the partition with non-zero data")
    return bytes(out[:part_size])


def parse_patch(s):
    off, _, word = s.partition("=")
    return (int(off, 0), int(word, 0))


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("input")
    ap.add_argument("output")
    ap.add_argument("--patch", action="append", required=True, metavar="OFF=WORD",
                    help="payload code offset = new 32-bit word (hex), repeatable")
    ap.add_argument("--load-base", type=lambda x: int(x, 0), default=DEFAULT_LOAD_BASE)
    args = ap.parse_args()

    patches = [parse_patch(p) for p in args.patch]
    data = bytearray(open(args.input, "rb").read())
    out = pack(data, patches, args.load_base)
    open(args.output, "wb").write(out)
    print(f"load base: 0x{args.load_base:x}")
    for off, word in patches:
        print(f"  patch @ code 0x{off:x} (runtime 0x{args.load_base + off:x}) = 0x{word:08x}")
    print(f"wrote {args.output} ({len(out)} bytes, == partition size)")


if __name__ == "__main__":
    main()
