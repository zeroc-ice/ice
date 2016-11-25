# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

TestSuite(__file__,
          [ClientTestCase(client=SimpleClient(args=lambda process, current: [current.getBuildDir("testplugin")]))],
          libDirs=["testplugin"])
