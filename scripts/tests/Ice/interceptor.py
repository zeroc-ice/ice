# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

TestSuite(__name__, [ClientTestCase(client=Client(props={"Ice.Warn.Dispatch" : 0}))], libDirs=["interceptortest"])
