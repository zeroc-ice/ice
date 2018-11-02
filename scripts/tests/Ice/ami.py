# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

testcases = [ ClientServerTestCase(traceProps=traceProps) ]
if Mapping.getByPath(__name__).hasSource("Ice/ami", "collocated"):
    testcases += [ CollocatedTestCase() ]

TestSuite(__name__, testcases,
          options = { "compress" : [False], "serialize" : [False] })
