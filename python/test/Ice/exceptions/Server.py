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
import Test, TestI

def run(args, communicator):
    adapter = communicator.createObjectAdapter("TestAdapter")
    adapter2 = communicator.createObjectAdapter("TestAdapter2")
    adapter3 = communicator.createObjectAdapter("TestAdapter3")
    object = TestI.ThrowerI()
    adapter.add(object, Ice.stringToIdentity("thrower"))
    adapter2.add(object, Ice.stringToIdentity("thrower"))
    adapter3.add(object, Ice.stringToIdentity("thrower"))
    adapter.activate()
    adapter2.activate()
    adapter3.activate()
    communicator.waitForShutdown()
    return True

try:
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)
    initData.properties.setProperty("Ice.Warn.Dispatch", "0")
    initData.properties.setProperty("Ice.Warn.Connections", "0");
    initData.properties.setProperty("TestAdapter.Endpoints", "default -p 12010:udp")
    initData.properties.setProperty("Ice.MessageSizeMax", "10")
    initData.properties.setProperty("TestAdapter2.Endpoints", "default -p 12011")
    initData.properties.setProperty("TestAdapter2.MessageSizeMax", "0")
    initData.properties.setProperty("TestAdapter3.Endpoints", "default -p 12012")
    initData.properties.setProperty("TestAdapter3.MessageSizeMax", "1")
    with Ice.initialize(sys.argv, initData) as communicator:
        status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

sys.exit(not status)
