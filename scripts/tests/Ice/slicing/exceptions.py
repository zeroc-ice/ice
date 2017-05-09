# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

testcases = [
    ClientServerTestCase(),
    ClientServerTestCase("client/server with 1.0 encoding", props={ "Ice.Default.EncodingVersion" : "1.0" }),
]

# If the mapping has AMD servers, also run with the AMD servers
if Mapping.getByPath(__name__).hasSource("Ice/exceptions", "serveramd"):
    testcases += [
        ClientAMDServerTestCase(),
        ClientAMDServerTestCase("client/amd server with 1.0 encoding", props={ "Ice.Default.EncodingVersion" : "1.0" }),
    ]

TestSuite(__name__, testcases)
