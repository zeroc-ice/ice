# Copyright (c) ZeroC, Inc.

from Util import ClientServerTestCase, Server, TestSuite
from ws_origin_probe import probe


# An origin that the test server is configured to accept.
ALLOWED_ORIGIN = "https://allowed.example.com"


class WSAllowedOriginsTestCase(ClientServerTestCase):
    def __init__(self):
        ClientServerTestCase.__init__(
            self,
            "server",
            server=Server(
                quiet=True,
                waitForShutdown=False,
                props={"TestAdapter.AllowedOrigins": ALLOWED_ORIGIN},
            ),
        )

    def runClientSide(self, current):
        host = current.host
        port = current.driver.getTestPort(0)

        cases = [
            # (Origin header value, expected to be accepted, description)
            (None, True, "absent Origin (non-browser client)"),
            (ALLOWED_ORIGIN, True, "allowed Origin"),
            ("https://attacker.example.com", False, "foreign Origin"),
            ("not-a-uri", False, "malformed Origin"),
        ]

        current.write("testing AllowedOrigins enforcement... ")
        for origin, expected_accepted, label in cases:
            accepted, status = probe(host, port, origin)
            if accepted != expected_accepted:
                action = "accepted" if accepted else "rejected"
                raise RuntimeError(
                    "{0}: server {1} unexpectedly (status: {2})".format(label, action, status)
                )
        current.writeln("ok")


TestSuite(__name__, [WSAllowedOriginsTestCase()])
