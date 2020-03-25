#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

testcases = [
    ClientServerTestCase(),
]

# If the mapping has AMD servers, also run with the AMD servers
if Mapping.getByPath(__name__).hasSource("Ice/slicing/exceptions", "serveramd"):
    testcases += [
        ClientAMDServerTestCase(),
    ]

TestSuite(__name__, testcases, options = { "serialize" : [False] })
