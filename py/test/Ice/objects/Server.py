#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
import Test, TestI

class InitialI(Test.Initial):
    def __init__(self, adapter):
        self._adapter = adapter
        self._b1 = TestI.BI()
        self._b2 = TestI.BI()
        self._c = TestI.CI()
        self._d = TestI.DI()

        self._b1.theA = self._b2 # Cyclic reference to another B
        self._b1.theB = self._b1 # Self reference.
        self._b1.theC = None     # Null reference.

        self._b2.theA = self._b2 # Self reference, using base.
        self._b2.theB = self._b1 # Cyclic reference to another B
        self._b2.theC = self._c  # Cyclic reference to a C.

        self._c.theB = self._b2  # Cyclic reference to a B.

        self._d.theA = self._b1  # Reference to a B.
        self._d.theB = self._b2  # Reference to a B.
        self._d.theC = None      # Reference to a C.

    def shutdown(self, current=None):
        self._adapter.getCommunicator().shutdown()

    def getB1(self, current=None):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        return self._b1

    def getB2(self, current=None):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        return self._b2

    def getC(self, current=None):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        return self._c

    def getD(self, current=None):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        self._d.preMarshalInvoked = False
        return self._d

    def getAll(self, current=None):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        self._d.preMarshalInvoked = False
        return (self._b1, self._b2, self._c, self._d)

def run(args, communicator):
    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000")
    adapter = communicator.createObjectAdapter("TestAdapter")
    initial = InitialI(adapter)
    adapter.add(initial, Ice.stringToIdentity("initial"))
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
