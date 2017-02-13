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
    communicator.getProperties().setProperty("TestAdapter.ACM.Timeout", "0")
    adapter = communicator.createObjectAdapter("TestAdapter")
    id = Ice.stringToIdentity("communicator")
    adapter.add(TestI.RemoteCommunicatorI(), id)
    adapter.activate()

    # Disable ready print for further adapters.
    communicator.getProperties().setProperty("Ice.PrintAdapterReady", "0")

    communicator.waitForShutdown()
    return True

try:
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)
    initData.properties.setProperty("Ice.Warn.Connections", "0");
    initData.properties.setProperty("Ice.ACM.Timeout", "1");
    with Ice.initialize(sys.argv, initData) as communicator:
        status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

sys.exit(not status)
