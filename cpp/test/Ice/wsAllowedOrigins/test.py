# Copyright (c) ZeroC, Inc.

from Util import ClientServerTestCase, Server, TestSuite
from ws_origin_probe import probe


# An origin that the test server is configured to accept.
ALLOWED_ORIGIN = "https://allowed.example.com"


def _runCases(current, cases):
    host = current.host
    port = current.driver.getTestPort(0)
    for origin, expected_accepted, label in cases:
        accepted, status = probe(host, port, origin)
        if accepted != expected_accepted:
            action = "accepted" if accepted else "rejected"
            raise RuntimeError(
                "{0}: server {1} unexpectedly (status: {2})".format(label, action, status)
            )


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
                    "TestAdapter.AllowedOrigins":
                        "https://web.example.com,https://api.example.com:443,http://dev.example.com:8080"
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


TestSuite(
    __name__,
    [WSAllowedOriginsTestCase(), WSAllowedOriginsWildcardTestCase(), WSAllowedOriginsPortTestCase()],
)
