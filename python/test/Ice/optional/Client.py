#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

from TestHelper import TestHelper
TestHelper.loadSlice("--all -I. ClientPrivate.ice")
import AllTests


class Client(TestHelper):

    def run(self, args):
        with self.initialize(args=args) as communicator:
            initial = AllTests.allTests(self, communicator)
            initial.shutdown()
