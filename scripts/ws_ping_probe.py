#!/usr/bin/env python3
# Copyright (c) ZeroC, Inc.

"""
Probe an Ice WebSocket server's handling of PING control frames.

Opens a TCP connection, performs the WebSocket upgrade, sends a masked PING
frame, and waits for the server's PONG, returning the echoed payload. This
exercises the WSTransceiver pong path, in particular the zero-length ping
(the common keep-alive case sent by browsers and load balancers).

It speaks just enough of RFC 6455 for this purpose; it does not interpret the
Ice protocol and skips any data frames (such as the Ice connection-validation
message) the server sends before the pong.
"""

import base64
import os
import socket

# WebSocket opcodes (RFC 6455 section 5.2).
_OP_CLOSE = 0x8
_OP_PING = 0x9
_OP_PONG = 0xA


class _Reader:
    """A buffered socket reader, primed with the bytes already read past the upgrade response."""

    def __init__(self, sock, buf=b""):
        self._sock = sock
        self._buf = buf

    def recvn(self, n):
        """Read exactly n bytes, raising if the connection closes first."""
        while len(self._buf) < n:
            chunk = self._sock.recv(n - len(self._buf))
            if not chunk:
                raise RuntimeError("connection closed while reading {0} bytes".format(n))
            self._buf += chunk
        buf, self._buf = self._buf[:n], self._buf[n:]
        return buf


def _upgrade(sock, host, port):
    """
    Perform the WebSocket upgrade handshake; raise unless the server returns 101.

    Returns the bytes read past the end of the response headers: the server can send its first
    frame in the same packet as the 101 response, and those bytes must not be dropped.
    """
    key = base64.b64encode(os.urandom(16)).decode("ascii")
    request = (
        "\r\n".join(
            [
                "GET / HTTP/1.1",
                "Host: {0}:{1}".format(host, port),
                "Upgrade: websocket",
                "Connection: Upgrade",
                "Sec-WebSocket-Key: {0}".format(key),
                "Sec-WebSocket-Version: 13",
                "Sec-WebSocket-Protocol: ice.zeroc.com",
            ]
        )
        + "\r\n\r\n"
    ).encode("ascii")
    sock.sendall(request)

    buf = b""
    while b"\r\n\r\n" not in buf and len(buf) < 8192:
        chunk = sock.recv(4096)
        if not chunk:
            break
        buf += chunk
    status = buf.split(b"\r\n", 1)[0].decode("ascii", errors="replace") if buf else "<no response>"
    if not status.startswith("HTTP/1.1 101"):
        raise RuntimeError("WebSocket upgrade rejected: {0}".format(status))
    return buf.partition(b"\r\n\r\n")[2]


def _client_frame(opcode, payload):
    """Build a masked client->server frame. Clients MUST mask (RFC 6455 5.3)."""
    # Control frames carry at most 125 bytes, so a 7-bit length is always enough here.
    assert len(payload) < 126
    mask = os.urandom(4)
    masked = bytes(b ^ mask[i % 4] for i, b in enumerate(payload))
    return bytes([0x80 | opcode, 0x80 | len(payload)]) + mask + masked


def _read_frame(reader):
    """Read one server->client frame and return (opcode, payload). Server frames are unmasked."""
    b0, b1 = reader.recvn(2)
    opcode = b0 & 0x0F
    masked = b1 & 0x80
    length = b1 & 0x7F
    if length == 126:
        length = int.from_bytes(reader.recvn(2), "big")
    elif length == 127:
        length = int.from_bytes(reader.recvn(8), "big")
    mask = reader.recvn(4) if masked else b""
    payload = reader.recvn(length) if length else b""
    if masked:
        payload = bytes(p ^ mask[i % 4] for i, p in enumerate(payload))
    return opcode, payload


def ping_pong(host, port, payload=b"", timeout=10.0):
    """
    Upgrade, send a PING with the given payload, and return the PONG payload.

    Non-PONG frames the server emits first (e.g. its Ice connection-validation
    message) are skipped. Raises if the server sends CLOSE or never pongs.
    """
    with socket.create_connection((host, port), timeout=timeout) as sock:
        reader = _Reader(sock, _upgrade(sock, host, port))
        sock.sendall(_client_frame(_OP_PING, payload))
        for _ in range(16):
            opcode, data = _read_frame(reader)
            if opcode == _OP_PONG:
                return data
            if opcode == _OP_CLOSE:
                raise RuntimeError("server sent CLOSE instead of PONG")
            # Otherwise a data frame (e.g. Ice connection validation); keep reading for the pong.
        raise RuntimeError("no PONG received after PING")
