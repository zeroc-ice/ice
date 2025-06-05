# Copyright (c) ZeroC, Inc.

# Enable some tracing to allow investigating test failures
from Util import ClientServerTestCase, TestSuite


traceProps = {"Ice.Trace.Network": 2, "Ice.Trace.Retry": 1, "Ice.Trace.Protocol": 1}

testcases = [ClientServerTestCase(traceProps=traceProps)]

TestSuite(__name__, testcases)
