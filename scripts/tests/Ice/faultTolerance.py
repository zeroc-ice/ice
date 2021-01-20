#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from Util import *

# This is used for the trace file
props = lambda process, current: {
    "Ice.ProgramName": "server{}".format(process.args[0])
}

# Enable some tracing to allow investigating test failures
traceProps = {
    "Ice.Trace.Network": 3,
    "Ice.Trace.Retry": 1,
    "Ice.Trace.Protocol": 1,
}

#
# Start 12 servers
#
servers=range(1, 13)

TestSuite(__name__, [
    ClientServerTestCase(client=Client(args=[i for i in servers]),
                         servers=[Server(args=[i], waitForShutdown=False, props=props, quiet=True) for i in servers],
                         traceProps=traceProps)
])
