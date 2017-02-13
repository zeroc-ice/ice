#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
import Test, TestI

def run(args, communicator):
    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010")
    communicator.getProperties().setProperty("ControllerAdapter.Endpoints", "default -p 12011")
    communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1")

    adapter = communicator.createObjectAdapter("TestAdapter")
    adapter2 = communicator.createObjectAdapter("ControllerAdapter")

    testController = TestI.TestIntfControllerI(adapter)

    adapter.add(TestI.TestIntfI(), Ice.stringToIdentity("test"))
    adapter.activate()

    adapter2.add(testController, Ice.stringToIdentity("testController"))
    adapter2.activate()

    communicator.waitForShutdown()
    return True

try:
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)

    #
    # This test kills connections, so we don't want warnings.
    #
    initData.properties.setProperty("Ice.Warn.Connections", "0")

    #
    # Limit the recv buffer size, this test relies on the socket
    # send() blocking after sending a given amount of data.
    #
    initData.properties.setProperty("Ice.TCP.RcvSize", "50000")

    with Ice.initialize(sys.argv, initData) as communicator:
        status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

sys.exit(not status)
