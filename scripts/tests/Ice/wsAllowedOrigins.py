# Copyright (c) ZeroC, Inc.

from Util import ClientServerTestCase, Server, TestSuite
from ws_origin_probe import probe
from ws_ping_probe import ping_pong

# An origin that the test server is configured to accept.
ALLOWED_ORIGIN = "https://allowed.example.com"


def _runCases(current, cases):
    host = current.host
    port = current.driver.getTestPort(0)
    for origin, expected_accepted, label in cases:
        accepted, status = probe(host, port, origin)
        if accepted != expected_accepted:
            action = "accepted" if accepted else "rejected"
            raise RuntimeError("{0}: server {1} unexpectedly (status: {2})".format(label, action, status))


class WSAllowedOriginsTestCase(ClientServerTestCase):
    def __init__(self):
        ClientServerTestCase.__init__(
            self,
            "explicit allowlist",
            server=Server(
                quiet=True,
                waitForShutdown=False,
                props={"TestAdapter.AllowedOrigins": ALLOWED_ORIGIN},
            ),
        )

    def runClientSide(self, current):
        current.write("testing AllowedOrigins enforcement... ")
        _runCases(
            current,
            [
                # (Origin header value, expected to be accepted, description)
                (None, True, "absent Origin (non-browser client)"),
                (ALLOWED_ORIGIN, True, "allowed Origin"),
                ("https://attacker.example.com", False, "foreign Origin"),
                ("not-a-uri", False, "malformed Origin"),
                # Per RFC 6454, a serialized origin is exactly scheme://host[:port]. A trailing slash is tolerated
                # (URI parsers normalize it to an empty path), but anything more (path, query, fragment, or userinfo)
                # must be rejected -- otherwise an attacker page can sneak past the allowlist by appending a path or
                # userinfo to a matching host.
                (ALLOWED_ORIGIN + "/", True, "trailing slash (same origin)"),
                (ALLOWED_ORIGIN + "/path", False, "Origin with path"),
                (ALLOWED_ORIGIN + "?q=1", False, "Origin with query"),
                (ALLOWED_ORIGIN + "#x", False, "Origin with fragment"),
                ("https://user@allowed.example.com", False, "Origin with userinfo"),
            ],
        )
        current.writeln("ok")


class WSAllowedOriginsWildcardTestCase(ClientServerTestCase):
    def __init__(self):
        ClientServerTestCase.__init__(
            self,
            "wildcard allowlist",
            server=Server(
                quiet=True,
                waitForShutdown=False,
                props={"TestAdapter.AllowedOrigins": "*"},
            ),
        )

    def runClientSide(self, current):
        current.write("testing AllowedOrigins=* is permissive... ")
        _runCases(
            current,
            [
                (None, True, "absent Origin"),
                (ALLOWED_ORIGIN, True, "any Origin (1)"),
                ("https://attacker.example.com", True, "any Origin (2)"),
            ],
        )
        current.writeln("ok")


class WSAllowedOriginsPortTestCase(ClientServerTestCase):
    # Exercises canonicalization of ports: the default port (80 for http, 443 for https) is omitted, so an entry with
    # an explicit default port matches an inbound Origin that omits it, and vice versa. Non-default ports must match
    # exactly.
    def __init__(self):
        ClientServerTestCase.__init__(
            self,
            "port canonicalization",
            server=Server(
                quiet=True,
                waitForShutdown=False,
                props={
                    "TestAdapter.AllowedOrigins": "https://web.example.com,https://api.example.com:443,http://dev.example.com:8080"
                },
            ),
        )

    def runClientSide(self, current):
        current.write("testing AllowedOrigins port canonicalization... ")
        _runCases(
            current,
            [
                # Default port stripped on both sides: bare and ':443' match the same entry.
                ("https://web.example.com", True, "implicit default port"),
                ("https://web.example.com:443", True, "explicit default port matches bare entry"),
                ("https://api.example.com", True, "bare Origin matches ':443' entry"),
                ("https://api.example.com:443", True, "explicit default port both sides"),
                # Non-default port must match exactly.
                ("http://dev.example.com:8080", True, "exact non-default port"),
                ("http://dev.example.com", False, "missing non-default port"),
                ("http://dev.example.com:8081", False, "wrong non-default port"),
                ("https://web.example.com:8443", False, "extra non-default port"),
            ],
        )
        current.writeln("ok")


class WSPingTestCase(ClientServerTestCase):
    # Exercises the WebSocket PING/PONG control-frame path. A zero-length ping (the common keep-alive case sent by
    # browsers and load balancers) must elicit an empty pong; before the corresponding fix the server formed
    # &_pingPayload[0] on an empty vector while building the pong -- undefined behavior that aborts under a hardened
    # standard library. A payload-bearing ping must be echoed back verbatim (RFC 6455 5.5.3): the client masks its
    # ping, so the server must unmask the payload before echoing it, otherwise the pong carries the masked bytes.
    def __init__(self):
        ClientServerTestCase.__init__(
            self,
            "ping/pong control frames",
            server=Server(quiet=True, waitForShutdown=False),
        )

    def runClientSide(self, current):
        current.write("testing WebSocket ping/pong control frames... ")
        host = current.host
        port = current.driver.getTestPort(0)
        if ping_pong(host, port, b"") != b"":
            raise RuntimeError("zero-length ping: server did not return an empty pong")
        # The probe masks its ping payload; the server must unmask it and echo it back unchanged in the pong.
        payload = b"ice ping payload \x00\x01\x02\xfe\xff"
        echoed = ping_pong(host, port, payload)
        if echoed != payload:
            raise RuntimeError("ping payload not echoed verbatim: sent {0!r}, got {1!r}".format(payload, echoed))
        current.writeln("ok")


TestSuite(
    __name__,
    [
        WSAllowedOriginsTestCase(),
        WSAllowedOriginsWildcardTestCase(),
        WSAllowedOriginsPortTestCase(),
        WSPingTestCase(),
    ],
)
