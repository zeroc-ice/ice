#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

testcases = [
    ClientServerTestCase("client/server with compact format"),
    ClientServerTestCase("client/server with sliced format", props={ "Ice.Default.SlicedFormat" : True }),
]

# If the mapping has AMD servers, also run with the AMD servers
if Mapping.getByPath(__name__).hasSource("Ice/tagged", "serveramd"):
    testcases += [
        ClientAMDServerTestCase("client/amd server with compact format"),
        ClientAMDServerTestCase("client/amd server with sliced format", props={ "Ice.Default.SlicedFormat" : True }),
    ]

TestSuite(__name__, testcases)
