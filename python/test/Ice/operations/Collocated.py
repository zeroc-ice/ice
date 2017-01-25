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
    adapter = communicator.createObjectAdapter("TestAdapter")
    prx = adapter.add(TestI.MyDerivedClassI(), Ice.stringToIdentity("test"))
    #adapter.activate() // Don't activate OA to ensure collocation is used.

    if prx.ice_getConnection():
        raise RuntimeError("collocation doesn't work")

    cl = AllTests.allTests(communicator)

    return True

try:
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)

    initData.properties.setProperty("Ice.BatchAutoFlushSize", "100")

    communicator = Ice.initialize(sys.argv, initData)
    status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

if communicator:
    communicator.destroy()

sys.exit(not status)
