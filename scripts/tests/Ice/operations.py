# Copyright (c) ZeroC, Inc.

# Enable some tracing to allow investigating test failures
from Util import ClientAMDServerTestCase, ClientServerTestCase, CollocatedTestCase, Mapping, TestSuite

traceProps = {"Ice.Trace.Retry": 1, "Ice.Trace.Protocol": 1}

testcases = [ClientServerTestCase(traceProps=traceProps)]

if Mapping.getByPath(__name__).hasSource("Ice/operations", "serveramd"):
    testcases += [ClientAMDServerTestCase(traceProps=traceProps)]

if Mapping.getByPath(__name__).hasSource("Ice/operations", "collocated"):
    testcases += [CollocatedTestCase(traceProps=traceProps)]

TestSuite(__name__, testcases)
