#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

TestSuite(__name__, [ClientTestCase(client=Client(props={"Ice.Warn.Dispatch" : 0}))], libDirs=["interceptortest"])
