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
# Start 5 servers
#
servers=range(0, 5)

#
# With UWP, we can't run the UDP tests with the C++ servers (used when SSL is enabled).
#
options=lambda current: { "protocol": ["tcp", "ws"] } if current.config.uwp else {}

TestSuite(__name__, [
    ClientServerTestCase(client=Client(args=[5]), servers=[Server(args=[i], ready="McastTestAdapter") for i in servers])
], multihost=False, options=options)
