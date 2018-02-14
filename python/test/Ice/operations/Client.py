#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback
import Ice

import Ice
slice_dir = Ice.getSliceDir()
if not slice_dir:
    print(sys.argv[0] + ': Slice directory not found.')
    sys.exit(1)

Ice.loadSlice("'-I" + slice_dir + "' Test.ice")
import AllTests

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def run(args, communicator):
    myClass = AllTests.allTests(communicator)

    sys.stdout.write("testing server shutdown... ")
    sys.stdout.flush()
    myClass.shutdown()
    try:
        myClass.opVoid()
        test(False)
    except Ice.LocalException:
        print("ok")

    return True

try:
    #
    # In this test, we need at least two threads in the
    # client side thread pool for nested AMI.
    #
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)
    initData.properties.setProperty('Ice.ThreadPool.Client.Size', '2')
    initData.properties.setProperty('Ice.ThreadPool.Client.SizeWarn', '0')
    initData.properties.setProperty("Ice.BatchAutoFlushSize", "100")

    communicator = Ice.initialize(sys.argv, initData)
    status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

if communicator:
    try:
        communicator.destroy()
    except:
        traceback.print_exc()
        status = False

sys.exit(not status)
