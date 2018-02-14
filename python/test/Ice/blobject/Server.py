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

Ice.loadSlice('Test.ice')
import Test

class TestI(Test.Hello):
    def sayHello(self, delay, current=None):
        if delay != 0:
            time.sleep(delay / 1000.0)

    def raiseUE(self, current=None):
        raise Test.UE()

    def add(self, s1, s2, current=None):
        return s1 + s2

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()


def run(args, communicator):
    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010:udp")
    adapter = communicator.createObjectAdapter("TestAdapter")
    adapter.add(TestI(), communicator.stringToIdentity("test"))
    adapter.activate()
    communicator.waitForShutdown()
    return True

try:
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)
    #
    # Its possible to have batch oneway requests dispatched after the
    # adapter is deactivated due to thread scheduling so we supress
    # this warning.
    #
    initData.properties.setProperty("Ice.Warn.Dispatch", "0");
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
