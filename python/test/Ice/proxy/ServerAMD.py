#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback, time

import Ice
slice_dir = Ice.getSliceDir()
if not slice_dir:
    print(sys.argv[0] + ': Slice directory not found.')
    sys.exit(1)

Ice.loadSlice("'-I" + slice_dir + "' TestAMD.ice")
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
    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010:udp")
    adapter = communicator.createObjectAdapter("TestAdapter")
    adapter.add(MyDerivedClassI(), communicator.stringToIdentity("test"))
    adapter.activate()
    communicator.waitForShutdown()
    return True

try:
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)
    initData.properties.setProperty("Ice.Warn.Connections", "0")
    initData.properties.setProperty("Ice.Warn.Dispatch", "0")
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
