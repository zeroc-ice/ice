#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "python", "Ice.py")):
        break
else:
    raise "can't find toplevel directory!"

#
# Get Slice directory.
#
slice_dir = os.path.join(os.path.join(toplevel, "..", "slice"))
if not os.path.exists(slice_dir):
    print sys.argv[0] + ': Slice directory not found.'
    sys.exit(1)

import Ice
Ice.loadSlice('-I' + slice_dir + ' Test.ice')
import AllTests

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def run(args, communicator):
    retry = AllTests.allTests(communicator)
    retry.shutdown()
    return True

try:
    #
    # In this test, we need at least two threads in the
    # client side thread pool for nested AMI.
    #
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)
    initData.properties.setProperty('Ice.RetryIntervals', '-1')
    initData.properties.setProperty('Ice.Warn.Connections', '0')
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
