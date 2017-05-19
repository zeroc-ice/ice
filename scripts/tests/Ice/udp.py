# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

from Util import *

#
# With UWP, we can't run the UDP tests with the C++ servers (used when SSL is enabled).
#
options=lambda current: { "protocol": ["tcp", "ws"] } if current.config.uwp else {}

class IceUdpTestSuite(TestSuite):

    def setup(self, current):
        TestSuite.setup(self, current)

        n = 1 if (isinstance(self.getMapping(), AndroidMapping) or
                  isinstance(self.getMapping(), AndroidCompatMapping)) else 5
        #
        # Start n servers
        #
        servers=range(0, n)

        self.testcases = {} # Clear default test cases
        self.addTestCase(ClientServerTestCase(client=Client(args=[n]),
                                              servers=[Server(args=[i], ready="McastTestAdapter") for i in servers]))

IceUdpTestSuite(__name__, multihost=False, options=options)
