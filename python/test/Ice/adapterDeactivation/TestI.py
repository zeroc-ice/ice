#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import time
import Ice
import Test


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


class TestI(Test.TestIntf):
    def transient(self, current=None):
        communicator = current.adapter.getCommunicator()
        adapter = communicator.createObjectAdapterWithEndpoints(
            "TransientTestAdapter", "default"
        )
        adapter.activate()
        adapter.destroy()

    def deactivate(self, current=None):
        current.adapter.deactivate()
        time.sleep(0.1)


class RouterI(Ice.Router):
    def __init__(self):
        self._nextPort = 23456

    def getClientProxy(self, c):
        return (None, False)

    def getServerProxy(self, c):
        port = self._nextPort
        self._nextPort += 1
        return Ice.ObjectPrx(
            c.adapter.getCommunicator(),
            "dummy:tcp -h localhost -p {0} -t 30000".format(port)
        )

    def addProxies(self, proxies, c):
        return []


class Cookie:
    def message(self):
        return "blahblah"


class ServantLocatorI(Ice.ServantLocator):
    def __init__(self):
        self._deactivated = False
        self._router = RouterI()

    def __del__(self):
        test(self._deactivated)

    def locate(self, current):
        test(not self._deactivated)

        if current.id.name == "router":
            return (self._router, None)

        test(current.id.category == "")
        test(current.id.name == "test")

        return (TestI(), Cookie())

    def finished(self, current, servant, cookie):
        test(not self._deactivated)

        if current.id.name == "router":
            return

        test(isinstance(cookie, Cookie))
        test(cookie.message() == "blahblah")

    def deactivate(self, category):
        test(not self._deactivated)

        self._deactivated = True
