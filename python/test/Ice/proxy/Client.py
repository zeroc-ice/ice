#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

from TestHelper import TestHelper
TestHelper.loadSlice("Test.ice")
import AllTests


class Client(TestHelper):

    def run(self, args):
        with self.initialize(args=args) as communicator:
            myClass = AllTests.allTests(self, communicator, False)
            myClass.shutdown()
