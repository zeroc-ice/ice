# Copyright (c) ZeroC, Inc.

from collections.abc import Awaitable, Mapping
from typing import override

from generated.test.Ice.proxy import Test

import Ice


class MyDerivedClassI(Test.MyDerivedClass):
    def __init__(self):
        self.ctx = dict[str, str]()

    @override
    def shutdown(self, current: Ice.Current) -> None:
        current.adapter.getCommunicator().shutdown()

    @override
    def getContext(self, current: Ice.Current) -> Mapping[str, str]:
        return self.ctx

    @override
    def echo(self, obj: Ice.ObjectPrx | None, current: Ice.Current) -> Ice.ObjectPrx | None:
        return obj

    @override
    def ice_isA(self, id: str, current: Ice.Current) -> bool | Awaitable[bool]:
        self.ctx = current.ctx
        return Test.MyDerivedClass.ice_isA(self, id, current)


class CI(Test.C):
    def __init__(self):
        self.ctx = dict[str, str]()

    @override
    def opA(self, a: Test.APrx | None, current: Ice.Current) -> Test.APrx | None:
        return a

    @override
    def opB(self, b: Test.BPrx | None, current: Ice.Current) -> Test.BPrx | None:
        return b

    @override
    def opC(self, c: Test.CPrx | None, current: Ice.Current) -> Test.CPrx | None:
        return c

    @override
    def opS(self, s: Test.S, current: Ice.Current) -> Test.S:
        return s
