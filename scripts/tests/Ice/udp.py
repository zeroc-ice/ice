# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

global currentMapping # The mapping for which this test suite is being loaded

if isinstance(currentMapping, AndroidMappingMixin):
    testcase = ClientServerTestCase(server=Server(ready="McastTestAdapter"))
else:
    testcase = ClientServerTestCase(client=Client(args=[5]),
                                    servers=[Server(args=[i], ready="McastTestAdapter") for i in range(0, 5)])

TestSuite(__name__, [ testcase ], multihost=False, options=options)
