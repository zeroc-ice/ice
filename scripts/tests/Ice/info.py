# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from ws_ping_probe import ping_pong

class WSPingTestCase(ClientServerTestCase):
    # Sends WebSocket PING control frames directly to a server listening on a ws endpoint and checks the PONG replies.

    def __init__(self):
        ClientServerTestCase.__init__(self, "ws ping/pong control frames",
                                      server=Server(quiet=True, waitForShutdown=False,
                                                    props={ "Ice.Default.Protocol" : "ws" }))

    def canRun(self, current):
        if current.driver.cross:
            return False
        if not isinstance(self.getMapping(), (CppMapping, CSharpMapping, JavaMapping)):
            return False
        if current.config.android or getattr(current.config, "uwp", False):
            return False
        return current.config.buildPlatform not in ["iphonesimulator", "iphoneos"]

    def runClientSide(self, current):
        current.write("testing WebSocket ping/pong control frames... ")
        host = current.host
        port = current.driver.getTestPort(0)
        if ping_pong(host, port, b"") != b"":
            raise RuntimeError("zero-length ping: server did not return an empty pong")
        # The probe masks its ping payload; the server must unmask it and echo it back unchanged (RFC 6455 5.5.3).
        payload = b"ice ping payload \x00\x01\x02\xfe\xff"
        echoed = ping_pong(host, port, payload)
        if echoed != payload:
            raise RuntimeError("ping payload not echoed verbatim: sent {0!r}, got {1!r}".format(payload, echoed))
        current.writeln("ok")

class InfoTestSuite(TestSuite):

    def __init__(self, *args, **kargs):
        TestSuite.__init__(self, *args, **kargs)
        # Run the WebSocket ping case in addition to the test cases computed from the test directory.
        testcase = WSPingTestCase()
        testcase.init(self.mapping, self)
        self.testcases[testcase.name] = testcase

InfoTestSuite(__name__, options = { "ipv6" : [False], "compress" : [False] }, multihost=False)
