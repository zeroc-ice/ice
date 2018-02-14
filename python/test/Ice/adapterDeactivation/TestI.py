# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback, time
import Ice, Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class TestI(Test.TestIntf):
    def transient(self, current=None):
        communicator = current.adapter.getCommunicator()
        adapter = communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default")
        adapter.activate()
        adapter.destroy()

    def deactivate(self, current=None):
        current.adapter.deactivate()
        time.sleep(0.1)

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
