#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback, time

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

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class TestI(Test.TestIntf):
    def transient(self, current=None):
        communicator = current.adapter.getCommunicator()
        adapter = communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default -p 9999")
        adapter.activate()
        adapter.deactivate()
        adapter.waitForDeactivate()

    def deactivate(self, current=None):
        current.adapter.deactivate()
        time.sleep(1)

class CookieI(Test.Cookie):
    def message(self):
        return 'blahblah'

class ServantLocatorI(Ice.ServantLocator):
    def __init__(self):
        self._deactivated = False

    def __del__(self):
        test(self._deactivated)

    def locate(self, current):
        test(not self._deactivated)

        test(current.id.category == '')
        test(current.id.name == 'test')

        return (TestI(), CookieI())

    def finished(self, current, servant, cookie):
        test(not self._deactivated)

        test(isinstance(cookie, Test.Cookie))
        test(cookie.message() == 'blahblah')

    def deactivate(self, category):
        test(not self._deactivated)

        self._deactivated = True

class TestServer(Ice.Application):
    def run(self, args):
        self.communicator().getProperties().setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000")
        adapter = self.communicator().createObjectAdapter("TestAdapter")
        locator = ServantLocatorI()

        adapter.addServantLocator(locator, "")
        adapter.activate()
        adapter.waitForDeactivate()
        return 0

app = TestServer()
sys.exit(app.main(sys.argv))
