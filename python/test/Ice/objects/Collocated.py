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
Ice.loadSlice('ClientPrivate.ice')
import Test, TestI, AllTests

def run(args, communicator):
    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010")
    adapter = communicator.createObjectAdapter("TestAdapter")
    initial = TestI.InitialI(adapter)
    adapter.add(initial, communicator.stringToIdentity("initial"))
    uoet = TestI.UnexpectedObjectExceptionTestI()
    adapter.add(uoet, communicator.stringToIdentity("uoet"))
    #adapter.activate() // Don't activate OA to ensure collocation is used.

    AllTests.allTests(communicator)

    # We must call shutdown even in the collocated case for cyclic dependency cleanup
    initial.shutdown()

    return True

try:
    communicator = Ice.initialize(sys.argv)
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
