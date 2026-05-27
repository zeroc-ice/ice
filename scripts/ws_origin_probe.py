#!/usr/bin/env python3
# Copyright (c) ZeroC, Inc.

"""
Probe an Ice WebSocket server's AllowedOrigins enforcement.

The script opens a TCP connection, sends a WebSocket upgrade request with an
optional Origin header, and reports whether the server accepted the upgrade
(HTTP/1.1 101 Switching Protocols) or rejected it (connection closed or
non-101 response). It does not interpret the Ice protocol; it stops at the
HTTP upgrade response.

Examples:

  # Single case: send a request with a specific Origin.
  ws_origin_probe.py 127.0.0.1 12345 --origin https://web.example.com

  # Standard battery: no Origin, allowed origins, foreign origin, malformed.
  ws_origin_probe.py 127.0.0.1 12345 --cases --allowed https://web.example.com

The server under test should be an Ice WS adapter configured with
'<AdapterName>.AllowedOrigins=<list>'. The probe works against any
implementation (C++, C#, Java).

Exit codes:
  0  upgrade accepted (or --cases all matched expectation, see below)
  1  upgrade rejected
  2  connection/IO error or unexpected response shape
"""

import argparse
import base64
import os
import socket
import sys


def probe(host, port, origin, timeout=5.0):
    """
    Send a WebSocket upgrade and return (accepted, status_line).

    'accepted' is True iff the server responded with HTTP/1.1 101 ...
    'status_line' is the first line of the response, or a short error
    string if the server closed the connection without writing one.
    """
    key = base64.b64encode(os.urandom(16)).decode("ascii")
    lines = [
        "GET / HTTP/1.1",
        f"Host: {host}:{port}",
        "Upgrade: websocket",
        "Connection: Upgrade",
        f"Sec-WebSocket-Key: {key}",
        "Sec-WebSocket-Version: 13",
        "Sec-WebSocket-Protocol: ice.zeroc.com",
    ]
    if origin is not None:
        lines.append(f"Origin: {origin}")
    request = ("\r\n".join(lines) + "\r\n\r\n").encode("ascii")

    with socket.create_connection((host, port), timeout=timeout) as s:
        s.sendall(request)
        buf = b""
        # Read until we have the response headers or the connection closes.
        while b"\r\n\r\n" not in buf and len(buf) < 8192:
            chunk = s.recv(4096)
            if not chunk:
                break
            buf += chunk
        if not buf:
            return False, "<connection closed with no response>"
        status_line = buf.split(b"\r\n", 1)[0].decode("ascii", errors="replace")
        return status_line.startswith("HTTP/1.1 101"), status_line


def run_cases(host, port, allowed_origins):
    """
    Run the standard battery of cases. Each case has an expected outcome.
    Returns 0 if all matched expectations, 1 otherwise.
    """
    cases = [
        (None, "absent Origin (non-browser client)", True),
    ]
    for origin in allowed_origins:
        cases.append((origin, f"allowed Origin: {origin}", True))
    cases.append(("https://attacker.example.com", "foreign Origin", False))
    cases.append(("not-a-uri", "malformed Origin", False))

    failures = 0
    for origin, label, expected_accepted in cases:
        try:
            accepted, status = probe(host, port, origin)
        except Exception as e:
            print(f"  [ERR ] {label}: {e}")
            failures += 1
            continue
        ok = accepted == expected_accepted
        verdict = "PASS" if ok else "FAIL"
        action = "accepted" if accepted else "rejected"
        print(f"  [{verdict}] {label}: {action} ({status})")
        if not ok:
            failures += 1
    return 0 if failures == 0 else 1


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("host")
    p.add_argument("port", type=int)
    p.add_argument("--origin", help="Value for the Origin request header")
    p.add_argument(
        "--cases",
        action="store_true",
        help="Run a standard battery of cases; pass --allowed for known-good origins",
    )
    p.add_argument(
        "--allowed",
        action="append",
        default=[],
        metavar="ORIGIN",
        help="(With --cases) An origin known to be in the server's allowlist (can be repeated)",
    )
    args = p.parse_args()

    if args.cases:
        return run_cases(args.host, args.port, args.allowed)

    try:
        accepted, status = probe(args.host, args.port, args.origin)
    except Exception as e:
        print(f"error: {e}", file=sys.stderr)
        return 2
    print(status)
    return 0 if accepted else 1


if __name__ == "__main__":
    sys.exit(main())
