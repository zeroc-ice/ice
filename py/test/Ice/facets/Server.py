# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, Ice, Test, _Top

class AI(_Top.A):
    def callA(self, current=None):
        return "A"

class BI(_Top.B, AI):
    def callB(self, current=None):
        return "B"

class CI(_Top.C, AI):
    def callC(self, current=None):
        return "C"

class DI(_Top.D, BI, CI):
    def callD(self, current=None):
        return "D"

class EI(_Top.E):
    def callE(self, current=None):
        return "E"

class FI(_Top.F, EI):
    def callF(self, current=None):
        return "F"

class GI(_Top.G):
    def __init__(self, communicator):
        self._communicator = communicator

    def shutdown(self, current=None):
        self._communicator.shutdown()

    def callG(self, current=None):
        return "G"

class HI(_Top.H, GI):
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
except Ice.Exception, ex:
    print ex
    status = False

if communicator:
    try:
        communicator.destroy()
    except Ice.Exception, ex:
        print ex
        status = False

sys.exit(not status)
