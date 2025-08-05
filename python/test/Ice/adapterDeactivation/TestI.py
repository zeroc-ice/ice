# Copyright (c) ZeroC, Inc.

import time
from collections.abc import Sequence
from typing import Any, override

from generated.test.Ice.adapterDeactivation import Test

import Ice


def test(b: Any) -> None:
    if not b:
        raise RuntimeError("test assertion failed")


class TestI(Test.TestIntf):
    def transient(self, current: Ice.Current):
        communicator = current.adapter.getCommunicator()
        adapter = communicator.createObjectAdapterWithEndpoints("TransientTestAdapter", "default")
        adapter.activate()
        adapter.destroy()

    def deactivate(self, current: Ice.Current):
        current.adapter.deactivate()
        time.sleep(0.1)


class RouterI(Ice.Router):
    @override
    def getClientProxy(self, current: Ice.Current) -> tuple[Ice.ObjectPrx | None, bool]:
        return (None, False)

    @override
    def getServerProxy(self, current: Ice.Current) -> Ice.ObjectPrx:
        return Ice.ObjectPrx(current.adapter.getCommunicator(), "dummy:tcp -h localhost -p 23456 -t 30000")

    @override
    def addProxies(
        self, proxies: Sequence[Ice.ObjectPrx | None], current: Ice.Current
    ) -> Sequence[Ice.ObjectPrx | None]:
        return []


class Cookie:
    def message(self) -> str:
        return "blahblah"


class ServantLocatorI(Ice.ServantLocator):
    def __init__(self):
        self._deactivated = False
        self._router = RouterI()

    def __del__(self):
        test(self._deactivated)

    @override
    def locate(self, current: Ice.Current) -> tuple[Ice.Object, Any]:
        test(not self._deactivated)

        if current.id.name == "router":
            return (self._router, None)

        test(current.id.category == "")
        test(current.id.name == "test")

        return (TestI(), Cookie())

    @override
    def finished(self, current: Ice.Current, servant: Ice.Object, cookie: Any):
        test(not self._deactivated)

        if current.id.name == "router":
            return

        assert isinstance(cookie, Cookie)
        assert cookie.message() == "blahblah"

    @override
    def deactivate(self, category: str) -> None:
        test(not self._deactivated)

        self._deactivated = True
