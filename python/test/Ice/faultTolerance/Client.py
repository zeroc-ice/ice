#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice(["Test.ice"])

import AllTests


class Client(TestHelper):
    def run(self, args: list[str]):
        properties = self.createTestProperties(args)

        #
        # This test aborts servers, so we don't want warnings.
        #
        properties.setProperty("Ice.Warn.Connections", "0")

        ports = []
        for arg in args:
            if arg[0] == "-":
                continue
            ports.append(int(arg))

        if len(ports) == 0:
            raise RuntimeError("no ports specified")

        with self.initialize(properties=properties) as communicator:
            AllTests.allTests(self, communicator, ports)
