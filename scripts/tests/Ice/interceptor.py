# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

TestSuite(__name__, [ClientTestCase(client=Client(props={"Ice.Warn.Dispatch" : 0}))], libDirs=["interceptortest"])
