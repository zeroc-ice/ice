#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

sys.path.insert(0, os.path.join(toplevel, "python"))
sys.path.insert(0, os.path.join(toplevel, "lib"))

import Ice
Ice.loadSlice('Test.ice')
Ice.loadSlice('ServerPrivate.ice')
import Test, TestI

class MyObjectFactory(Ice.ObjectFactory):
    def create(self, type):
        if type == '::Test::I':
            return TestI.II()
        elif type == '::Test::J':
            return TestI.JI()
        elif type == '::Test::H':
            return TestI.HI()
        assert(False) # Should never be reached

    def destroy(self):
        # Nothing to do
        pass

def run(args, communicator):
    factory = MyObjectFactory()
    communicator.addObjectFactory(factory, '::Test::I')
    communicator.addObjectFactory(factory, '::Test::J')
    communicator.addObjectFactory(factory, '::Test::H')

    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000")
    adapter = communicator.createObjectAdapter("TestAdapter")
    initial = TestI.InitialI(adapter)
    adapter.add(initial, communicator.stringToIdentity("initial"))
    uoet = TestI.UnexpectedObjectExceptionTestI()
    adapter.add(uoet, communicator.stringToIdentity("uoet"))
    adapter.activate()
    communicator.waitForShutdown()
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
