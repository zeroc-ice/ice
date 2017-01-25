#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback

import Ice
slice_dir = Ice.getSliceDir()
if not slice_dir:
    print(sys.argv[0] + ': Slice directory not found.')
    sys.exit(1)

Ice.loadSlice('"-I' + slice_dir + '" Test.ice')
import Test, TestI, AllTests

def run(args, communicator):
    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010")
    communicator.getProperties().setProperty("ControllerAdapter.Endpoints", "default -p 12011")
    communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1")

    adapter = communicator.createObjectAdapter("TestAdapter")
    adapter2 = communicator.createObjectAdapter("ControllerAdapter")

    testController = TestI.TestIntfControllerI(adapter)

    adapter.add(TestI.TestIntfI(), Ice.stringToIdentity("test"))
    #adapter.activate() # Collocated test doesn't need to active the OA

    adapter2.add(testController, Ice.stringToIdentity("testController"))
    #adapter2.activate() # Collocated test doesn't need to active the OA

    AllTests.allTests(communicator, True)

    return True

try:
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)

    initData.properties.setProperty("Ice.Warn.AMICallback", "0");

    #
    # This test kills connections, so we don't want warnings.
    #
    initData.properties.setProperty("Ice.Warn.Connections", "0");

    communicator = Ice.initialize(sys.argv, initData)
    status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

if communicator:
    communicator.destroy()

sys.exit(not status)
