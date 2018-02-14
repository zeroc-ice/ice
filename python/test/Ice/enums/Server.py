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

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class TestIntfI(Test.TestIntf):
    def opByte(self, b1, current=None):
        return (b1, b1)

    def opShort(self, s1, current=None):
        return (s1, s1)

    def opInt(self, i1, current=None):
        return (i1, i1)

    def opSimple(self, s1, current=None):
        return (s1, s1)

    def opByteSeq(self, b1, current=None):
        return (b1, b1)

    def opShortSeq(self, s1, current=None):
        return (s1, s1)

    def opIntSeq(self, i1, current=None):
        return (i1, i1)

    def opSimpleSeq(self, s1, current=None):
        return (s1, s1)

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

def run(args, communicator):
    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010")
    adapter = communicator.createObjectAdapter("TestAdapter")
    object = TestIntfI()
    adapter.add(object, Ice.stringToIdentity("test"))
    adapter.activate()
    communicator.waitForShutdown()
    return True

try:
    with Ice.initialize(sys.argv) as communicator:
         status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

sys.exit(not status)
