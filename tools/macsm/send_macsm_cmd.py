#!/usr/bin/env python3
import argparse
import socket
import struct

MACSM_MGR_CMD_MID = 0x18F0


def build_packet(fc, payload):
    length = len(payload) + 1  # sec hdr (2 bytes) -1
    header = struct.pack(">HHHBB", MACSM_MGR_CMD_MID, 0xC000, length, fc, 0)
    return header + payload


def main():
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="command", required=True)

    unload_p = sub.add_parser("unload")
    unload_p.add_argument("name")

    load_p = sub.add_parser("load")
    load_p.add_argument("name")
    load_p.add_argument("entry")
    load_p.add_argument("path")
    load_p.add_argument("stack", type=int, nargs="?", default=16384)
    load_p.add_argument("priority", type=int, nargs="?", default=100)

    reload_p = sub.add_parser("reload")
    reload_p.add_argument("name")
    reload_p.add_argument("path")

    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=1234)
    args = parser.parse_args()

    if args.command == "unload":
        payload = struct.pack(">20s", args.name.encode())
        pkt = build_packet(1, payload)
    elif args.command == "load":
        payload = struct.pack(">20s20s64sII", args.name.encode(), args.entry.encode(), args.path.encode(), args.stack, args.priority)
        pkt = build_packet(2, payload)
    elif args.command == "reload":
        payload = struct.pack(">20s64s", args.name.encode(), args.path.encode())
        pkt = build_packet(3, payload)

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(pkt, (args.host, args.port))
    sock.close()


if __name__ == "__main__":
    main()
