# Copyright (c) ZeroC, Inc.

from Util import ClientServerTestCase, TestSuite

# Enable some tracing to allow investigating test failures
traceProps = {"Ice.Trace.Network": 2, "Ice.Trace.Protocol": 1}

testcases = [ClientServerTestCase(traceProps=traceProps)]

TestSuite(__name__, testcases, libDirs=["testtransport"], options={"mx": [False]})
