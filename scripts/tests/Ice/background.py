#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

# SSL doesn't work with this test when holding on reading on the server side
TestSuite(__name__, libDirs=["testtransport"], options={ "mx": [False], "transport": ["tcp", "ws"] })
