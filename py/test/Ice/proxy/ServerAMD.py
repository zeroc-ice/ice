#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback, time

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "python", "Ice.py")):
        break
else:
    raise "can't find toplevel directory!"

import Ice

#
# Get Slice directory.
#
slice_dir = os.path.join(os.path.join(toplevel, "..", "slice"))
if not os.path.exists(slice_dir):
    print sys.argv[0] + ': Slice directory not found.'
    sys.exit(1)

Ice.loadSlice('-I' + slice_dir + ' TestAMD.ice')
import Test

class MyDerivedClassI(Test.MyDerivedClass):
    def __init__(self):
        self.ctx = None

    def shutdown_async(self, cb, current=None):
        current.adapter.getCommunicator().shutdown()
        cb.ice_response()

    def getContext_async(self, cb, current):
        return cb.ice_response(self.ctx)

    def echo_async(self, cb, obj, current):
        return cb.ice_response(obj)

    def ice_isA(self, s, current):
        self.ctx = current.ctx
        return Test.MyDerivedClass.ice_isA(self, s, current)

def run(args, communicator):
    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000:udp")
    adapter = communicator.createObjectAdapter("TestAdapter")
    adapter.add(MyDerivedClassI(), communicator.stringToIdentity("test"))
    adapter.activate()
    communicator.waitForShutdown()
    return True

try:
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)
    initData.properties.setProperty("Ice.Warn.Connections", "0");
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
