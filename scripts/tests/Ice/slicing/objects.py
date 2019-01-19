#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

TestSuite(__name__, [
    ClientServerTestCase(),
    ClientServerTestCase("client/server with 1.0 encoding", props={ "Ice.Default.EncodingVersion" : "1.0" }),
], options = { "valgrind" : [False] })
