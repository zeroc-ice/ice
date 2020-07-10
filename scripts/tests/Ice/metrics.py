# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

# Enable some tracing to allow investigating test failures
traceProps = {
    "Ice.Trace.Network" : 3,
    "Ice.Trace.Retry" : 1,
    "Ice.Trace.Protocol" : 1
}
testcases = [
    ClientServerTestCase(traceProps=traceProps),
]
if Mapping.getByPath(__name__).hasSource("Ice/metrics", "serveramd"):
    testcases += [ ClientAMDServerTestCase(traceProps=traceProps) ]
if Mapping.getByPath(__name__).hasSource("Ice/metrics", "collocated"):
    testcases += [ CollocatedTestCase(traceProps=traceProps) ]

TestSuite(__name__, testcases,
          options = { "ipv6" : [False], "compress": [False], "transport" : ["tcp", "ssl"] },
          multihost=False)
