#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback, threading, time

#
# We want to test coroutines but older versions of Python cannot load a source file that uses the
# async/await keywords, so we have two versions of MyDerivedClassI.
#
if sys.version_info >= (3,5):
    from TestAMDCoroI import MyDerivedClassI
else:
    from TestAMDI import MyDerivedClassI

import Ice
slice_dir = Ice.getSliceDir()
if not slice_dir:
    print(sys.argv[0] + ': Slice directory not found.')
    sys.exit(1)

Ice.loadSlice("'-I" + slice_dir + "' Test.ice")
import Test

def run(args, communicator):
    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010")
    adapter = communicator.createObjectAdapter("TestAdapter")
    adapter.add(MyDerivedClassI(), Ice.stringToIdentity("test"))
    adapter.activate()
    communicator.waitForShutdown()
    return True

try:
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)
    initData.properties.setProperty("Ice.Warn.Dispatch", "0");
    with Ice.initialize(sys.argv, initData) as communicator:
        status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

sys.exit(not status)
