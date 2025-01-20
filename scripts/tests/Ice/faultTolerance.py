# Copyright (c) ZeroC, Inc.


from Util import Client, ClientServerTestCase, Server, TestSuite


# This is used for the trace file
def props(process, current):
    return {"Ice.ProgramName": "server{}".format(process.args[0])}


# Enable some tracing to allow investigating test failures
traceProps = {
    "Ice.Trace.Network": 3,
    "Ice.Trace.Retry": 1,
    "Ice.Trace.Protocol": 1,
}

#
# Start 12 servers
#
servers = range(1, 13)

TestSuite(
    __name__,
    [
        ClientServerTestCase(
            client=Client(args=[i for i in servers]),
            servers=[
                Server(args=[i], waitForShutdown=False, props=props, quiet=True)
                for i in servers
            ],
            traceProps=traceProps,
        )
    ],
)
