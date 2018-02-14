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
    AllTests.allTests(communicator, False)
    return True

try:
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)
    initData.properties.setProperty('Ice.Warn.AMICallback', '0')

    #
    # Limit the send buffer size, this test relies on the socket
    # send() blocking after sending a given amount of data.
    #
    initData.properties.setProperty("Ice.TCP.SndSize", "50000");

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
