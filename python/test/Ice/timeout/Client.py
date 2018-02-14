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
    timeout = AllTests.allTests(communicator)
    timeout.shutdown()

    return True

try:
    #
    # In this test, we need at least two threads in the
    # client side thread pool for nested AMI.
    #
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)

    #
    # We need to send messages large enough to cause the transport
    # buffers to fill up.
    #
    initData.properties.setProperty("Ice.MessageSizeMax", "10000");

    #
    # For this test, we want to disable retries.
    #
    initData.properties.setProperty("Ice.RetryIntervals", "-1");

    #
    # This test kills connections, so we don't want warnings.
    #
    initData.properties.setProperty("Ice.Warn.Connections", "0");

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
