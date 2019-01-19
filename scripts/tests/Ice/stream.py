#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

TestSuite(__name__, [
    ClientTestCase("client with default encoding"),
    ClientTestCase("client with 1.0 encoding", props={ "Ice.Default.EncodingVersion" : "1.0" })
])
