#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys

import Ice
Ice.loadSlice('Test.ice')
import Test, AllTests

class TestClient(Ice.Application):
    def run(self, args):
        obj = AllTests.allTests(self.communicator(), False)
        obj.shutdown()
        return 0

app = TestClient()
sys.exit(app.main(sys.argv))
