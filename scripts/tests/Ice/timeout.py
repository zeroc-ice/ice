# -*- coding: utf-8 -*-
# Copyright (c) ZeroC, Inc.

# Enable some tracing to allow investigating test failures
from Util import ClientServerTestCase, Server, TestSuite


traceProps = {"Ice.Trace.Network": 2, "Ice.Trace.Retry": 1, "Ice.Trace.Protocol": 1}

TestSuite(
    __name__,
    [ClientServerTestCase(server=Server(readyCount=2), traceProps=traceProps)],
    options={"compress": [False]},
)
