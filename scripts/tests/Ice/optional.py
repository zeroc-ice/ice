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
]

# If the mapping has AMD servers, also run with the AMD servers
if Mapping.getByPath(__name__).hasSource("Ice/optional", "serveramd"):
    testcases += [
        ClientAMDServerTestCase("client/amd server with compact format"),
        ClientAMDServerTestCase("client/amd server with sliced format", props={ "Ice.Default.SlicedFormat" : True }),
    ]

TestSuite(__name__, testcases)
