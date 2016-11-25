# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

TestSuite(__name__, [
    ClientServerTestCase(client=Client(args=[5]), servers=[Server(args=[i], ready="McastTestAdapter") for i in servers])
], multihost=False)
