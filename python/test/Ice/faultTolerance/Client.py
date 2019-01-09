#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************


from TestHelper import TestHelper
import AllTests


class Client(TestHelper):

    def run(self, args):

        properties = self.createTestProperties(args)

        #
        # This test aborts servers, so we don't want warnings.
        #
        properties.setProperty('Ice.Warn.Connections', '0')

        ports = []
        for arg in args:
            if arg[0] == '-':
                continue
            ports.append(int(arg))

        if len(ports) == 0:
            raise RuntimeError("no ports specified")

        with self.initialize(properties=properties) as communicator:
            AllTests.allTests(self, communicator, ports)
