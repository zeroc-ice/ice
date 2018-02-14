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
import Test, TestI, AllTests

def run(args, communicator):
    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010")
    adapter = communicator.createObjectAdapter("TestAdapter")
    d = TestI.DI()
    adapter.add(d, communicator.stringToIdentity("d"))
    adapter.addFacet(d, communicator.stringToIdentity("d"), "facetABCD")
    f = TestI.FI()
    adapter.addFacet(f, communicator.stringToIdentity("d"), "facetEF")
    h = TestI.HI(communicator)
    adapter.addFacet(h, communicator.stringToIdentity("d"), "facetGH")

    #adapter.activate() // Don't activate OA to ensure collocation is used.

    AllTests.allTests(communicator)

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
