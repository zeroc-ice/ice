#!/usr/bin/env python3
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from TestHelper import TestHelper
import AllTests


class Client(TestHelper):
    def run(self, args):
        with self.initialize(args=args) as communicator:
            initial = AllTests.allTests(self, communicator)
            initial.shutdown()
