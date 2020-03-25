#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

testcases = [
    ClientServerTestCase("client/server test with Compact format"),
    ClientServerTestCase("client/server test with Sliced format", props = {"Ice.Default.SlicedFormat" : True}),
]

# If the mapping has AMD servers, also run with the AMD servers
if Mapping.getByPath(__name__).hasSource("Ice/slicing/objects", "serveramd"):
    testcases += [
        ClientAMDServerTestCase("client/AMD server test with Compact format"),
        ClientAMDServerTestCase("client/ AMD server test with Sliced format",
                                props = {"Ice.Default.SlicedFormat" : True}),
    ]

TestSuite(__name__, testcases, options = { "valgrind" : [False] })
