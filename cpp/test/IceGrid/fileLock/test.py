# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

class IceGridAdminTestCase(IceGridTestCase):

    def runClientSide(self, current):

        sys.stdout.write("testing IceGrid file lock... ")
        registry = IceGridRegistryMaster(portnum=25, readyCount=0, quiet=True);
        registry.start(current)
        registry.expect(current, ".*IceUtil::FileLockException.*")
        registry.stop(current, False)
        print("ok")

TestSuite(__file__, [ IceGridAdminTestCase(application=None) ], runOnMainThread=True, multihost=False)
