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


class I1(Test.I):

    def opS(self, s1, current=None):
        return (s1, s1)

    def opSSeq(self, sseq1, current=None):
        return (sseq1, sseq1)

    def opSMap(self, smap1, current=None):
        return (smap1, smap1)

    def opC(self, c1, current=None):
        return (c1, c1)

    def opCSeq(self, cseq1, current=None):
        return (cseq1, cseq1)

    def opCMap(self, cmap1, current=None):
        return (cmap1, cmap1)

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()


class I2(Test.Inner.Inner2.I):

    def opS(self, s1, current=None):
        return (s1, s1)

    def opSSeq(self, sseq1, current=None):
        return (sseq1, sseq1)

    def opSMap(self, smap1, current=None):
        return (smap1, smap1)

    def opC(self, c1, current=None):
        return (c1, c1)

    def opCSeq(self, cseq1, current=None):
        return (cseq1, cseq1)

    def opCMap(self, cmap1, current=None):
        return (cmap1, cmap1)

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()


class I3(Test.Inner.I):

    def opS(self, s1, current=None):
        return (s1, s1)

    def opSSeq(self, sseq1, current=None):
        return (sseq1, sseq1)

    def opSMap(self, smap1, current=None):
        return (smap1, smap1)

    def opC(self, c1, current=None):
        return (c1, c1)

    def opCSeq(self, cseq1, current=None):
        return (cseq1, cseq1)

    def opCMap(self, cmap1, current=None):
        return (cmap1, cmap1)

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()


def run(args, communicator):
    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010")
    adapter = communicator.createObjectAdapter("TestAdapter")
    adapter.add(I1(), Ice.stringToIdentity("i1"))
    adapter.add(I2(), Ice.stringToIdentity("i2"))
    adapter.add(I3(), Ice.stringToIdentity("i3"))
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
