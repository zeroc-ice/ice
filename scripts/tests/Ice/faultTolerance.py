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
# Start 12 servers
#
servers=range(1, 13)

TestSuite(__name__, [
    ClientServerTestCase(client=Client(args=[i for i in servers]),
                         servers=[Server(args=[i], waitForShutdown=False, quiet=True) for i in servers])
])
