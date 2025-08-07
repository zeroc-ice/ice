# Copyright (c) ZeroC, Inc.

from typing import override

from generated.test.Ice.inheritance import Test
from generated.test.Ice.inheritance.Test import MA, MB

import Ice


class IAI(MA.IA):
    @override
    def iaop(self, p: MA.IAPrx | None, current: Ice.Current) -> MA.IAPrx | None:
        return p


class IB1I(MB.IB1, IAI):
    @override
    def ib1op(self, p: MB.IB1Prx | None, current: Ice.Current) -> MB.IB1Prx | None:
        return p


class IB2I(MB.IB2, IAI):
    @override
    def ib2op(self, p: MB.IB2Prx | None, current: Ice.Current) -> MB.IB2Prx | None:
        return p


class ICI(MA.IC, IB1I, IB2I):
    @override
    def icop(self, p: MA.ICPrx | None, current: Ice.Current) -> MA.ICPrx | None:
        return p
        return p


class InitialI(Test.Initial):
    def __init__(self, adapter: Ice.ObjectAdapter):
        self._ia = MA.IAPrx.uncheckedCast(adapter.addWithUUID(IAI()))
        self._ib1 = MB.IB1Prx.uncheckedCast(adapter.addWithUUID(IB1I()))
        self._ib2 = MB.IB2Prx.uncheckedCast(adapter.addWithUUID(IB2I()))
        self._ic = MA.ICPrx.uncheckedCast(adapter.addWithUUID(ICI()))

    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()

    def iaop(self, current: Ice.Current):
        return self._ia

    def ib1op(self, current: Ice.Current):
        return self._ib1

    def ib2op(self, current: Ice.Current):
        return self._ib2

    def icop(self, current: Ice.Current):
        return self._ic
