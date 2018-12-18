# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

# Enable some tracing to allow investigating test failures
traceProps = {
    "Ice.Trace.Network" : 2,
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
          options = { "ipv6" : [False], "compress": [False], "protocol" : ["tcp", "ssl"] },
          multihost=False)
