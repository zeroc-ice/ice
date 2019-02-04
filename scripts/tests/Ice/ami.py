# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

# Enable some tracing to allow investigating test failures
traceProps = {
    "Ice.Trace.Network" : 2,
    "Ice.Trace.Retry" : 1,
    "Ice.Trace.Protocol" : 1
}

testcases = [ ClientServerTestCase(traceProps=traceProps) ]
if Mapping.getByPath(__name__).hasSource("Ice/ami", "collocated"):
    testcases += [ CollocatedTestCase() ]

TestSuite(__name__, testcases,
          options = { "compress" : [False], "serialize" : [False] })
