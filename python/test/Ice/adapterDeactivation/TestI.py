# Copyright (c) ZeroC, Inc.

import time
import Ice
import Test


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


class TestI(Test.TestIntf):
    def transient(self, current):
        communicator = current.adapter.getCommunicator()
        adapter = communicator.createObjectAdapterWithEndpoints(
            "TransientTestAdapter", "default"
        )
        adapter.activate()
        adapter.destroy()

    def deactivate(self, current):
        current.adapter.deactivate()
        time.sleep(0.1)


class RouterI(Ice.Router):
    def getClientProxy(self, c):
        return (None, False)

    def getServerProxy(self, c):
        return Ice.ObjectPrx(
            c.adapter.getCommunicator(),
            "dummy:tcp -h localhost -p 23456 -t 30000"
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
