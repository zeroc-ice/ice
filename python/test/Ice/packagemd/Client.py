#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

from TestHelper import TestHelper
TestHelper.loadSlice("--all -I. Test.ice")
import AllTests


class Client(TestHelper):

    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty('Ice.Warn.Dispatch', '0')
        with self.initialize(properties=properties) as communicator:
            initial = AllTests.allTests(self, communicator)
            initial.shutdown()
