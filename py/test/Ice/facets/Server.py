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
import Test

class AI(Test.A):
    def callA(self, current=None):
        return "A"

class BI(Test.B, AI):
    def callB(self, current=None):
        return "B"

class CI(Test.C, AI):
    def callC(self, current=None):
        return "C"

class DI(Test.D, BI, CI):
    def callD(self, current=None):
        return "D"

class EI(Test.E):
    def callE(self, current=None):
        return "E"

class FI(Test.F, EI):
    def callF(self, current=None):
        return "F"

class GI(Test.G):
    def __init__(self, communicator):
        self._communicator = communicator

    def shutdown(self, current=None):
        self._communicator.shutdown()

    def callG(self, current=None):
        return "G"

class HI(Test.H, GI):
    def __init__(self, communicator):
        GI.__init__(self, communicator)

    def callH(self, current=None):
        return "H"

def run(args, communicator):
    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000")
    adapter = communicator.createObjectAdapter("TestAdapter")
    d = DI()
    adapter.add(d, Ice.stringToIdentity("d"))
    adapter.addFacet(d, Ice.stringToIdentity("d"), "facetABCD")
    f = FI()
    adapter.addFacet(f, Ice.stringToIdentity("d"), "facetEF")
    h = HI(communicator)
    adapter.addFacet(h, Ice.stringToIdentity("d"), "facetGH")

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
