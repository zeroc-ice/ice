# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

testcases = [
    ClientServerTestCase("client/server with compact format"),
    ClientServerTestCase("client/server with sliced format", props={ "Ice.Default.SlicedFormat" : True }),
    ClientServerTestCase("client/server with 1.0 encoding", props={ "Ice.Default.EncodingVersion" : "1.0" }),
]

if Mapping.getByPath(__name__).hasSource("Ice/objects", "collocated"):
    testcases += [ CollocatedTestCase() ]

TestSuite(__name__, testcases)

