# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, Ice, Test, _Top, time

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class TestI(_Top.Test):
    def transient(self, current=None):
        communicator = current.adapter.getCommunicator()
        adapter = communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default -p 9999")
        adapter.activate()
        adapter.deactivate()
        adapter.waitForDeactivate()

    def deactivate(self, current=None):
        current.adapter.deactivate()
        time.sleep(1)

class CookieI(_Top.Cookie):
    def message(self):
        return 'blahblah'

class ServantLocatorI(Ice.ServantLocator):
    def __init__(self):
        self._deactivated = False

    def __del__(self):
        test(self._deactivated)

    def locate(self, current):
        print "locate returning TestI"
        test(not self._deactivated)

        test(current.id.category == '')
        test(current.id.name == 'test')

        return (TestI(), CookieI())

    def finished(self, current, servant, cookie):
        print "finished received " + cookie.message()
        test(not self._deactivated)

        test(isinstance(cookie, _Top.Cookie))
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
