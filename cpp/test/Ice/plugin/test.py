#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

TestSuite(__file__,
          [ClientTestCase(client=SimpleClient(args=lambda process, current: [current.getBuildDir("testplugin")]))],
          libDirs=["testplugin"])
