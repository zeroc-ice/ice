#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

TestSuite(__name__, [
    ClientServerTestCase(),
], options = { "valgrind" : [False] })
