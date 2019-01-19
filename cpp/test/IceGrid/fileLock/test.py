# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

class IceGridAdminTestCase(IceGridTestCase):

    def runClientSide(self, current):

        sys.stdout.write("testing IceGrid file lock... ")
        registry = IceGridRegistryMaster(portnum=25, readyCount=0, quiet=True);
        registry.start(current)
        registry.expect(current, ".*IceUtil::FileLockException.*")
        registry.stop(current, False)
        print("ok")

TestSuite(__file__, [ IceGridAdminTestCase(application=None) ], runOnMainThread=True, multihost=False)
