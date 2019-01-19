#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from Util import *

#
# Start 12 servers
#
servers=range(1, 13)

TestSuite(__name__, [
    ClientServerTestCase(client=Client(args=[i for i in servers]),
                         servers=[Server(args=[i], waitForShutdown=False, quiet=True) for i in servers])
])
