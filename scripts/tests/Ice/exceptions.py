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

# If the mapping has AMD servers, also run with the AMD servers
if Mapping.getByPath(__name__).hasSource("Ice/exceptions", "serveramd"):
    testcases += [
        ClientAMDServerTestCase("client/amd server with compact format"),
        ClientAMDServerTestCase("client/amd server with sliced format", props={ "Ice.Default.SlicedFormat" : True }),
        ClientAMDServerTestCase("client/amd server with 1.0 encoding", props={ "Ice.Default.EncodingVersion" : "1.0"}),
    ]

# If the mapping has bidir clients, also run with the bidir clients.
if Mapping.getByPath(__name__).getClientMapping().hasSource("Ice/exceptions", "clientBidir"):
    testcases += [
        ClientEchoServerTestCase("client/echo server with compact format"),
        ClientEchoServerTestCase("client/echo server with sliced format", props={"Ice.Default.SlicedFormat" : True}),
        ClientEchoServerTestCase("client/echo server with 1.0 encoding", props={"Ice.Default.EncodingVersion" : "1.0"}),
    ]

if Mapping.getByPath(__name__).hasSource("Ice/exceptions", "collocated"):
    testcases += [ CollocatedTestCase() ]

TestSuite(__name__, testcases)
