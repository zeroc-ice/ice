# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test

class CAI(Test.MA.CADisp):
    def caop(self, p, current=None):
        return p

class CBI(Test.MB.CBDisp, CAI):
     def cbop(self, p, current=None):
        return p

class CCI(Test.MA.CCDisp, CBI):
    def ccop(self, p, current=None):
        return p

class IAI(Test.MA.IA):
    def iaop(self, p, current=None):
        return p

class IB1I(Test.MB.IB1, IAI):
    def ib1op(self, p, current=None):
        return p

class IB2I(Test.MB.IB2, IAI):
    def ib2op(self, p, current=None):
        return p

class ICI(Test.MA.IC, IB1I, IB2I):
    def icop(self, p, current=None):
        return p

class CDI(Test.MA.CDDisp, CCI, IB1I, IB2I):
    def cdop(self, p, current=None):
        return p

class InitialI(Test.Initial):
    def __init__(self, adapter):
        self._ca = Test.MA.CAPrx.uncheckedCast(adapter.addWithUUID(CAI()))
        self._cb = Test.MB.CBPrx.uncheckedCast(adapter.addWithUUID(CBI()))
        self._cc = Test.MA.CCPrx.uncheckedCast(adapter.addWithUUID(CCI()))
        self._cd = Test.MA.CDPrx.uncheckedCast(adapter.addWithUUID(CDI()))
        self._ia = Test.MA.IAPrx.uncheckedCast(adapter.addWithUUID(IAI()))
        self._ib1 = Test.MB.IB1Prx.uncheckedCast(adapter.addWithUUID(IB1I()))
        self._ib2 = Test.MB.IB2Prx.uncheckedCast(adapter.addWithUUID(IB2I()))
        self._ic = Test.MA.ICPrx.uncheckedCast(adapter.addWithUUID(ICI()))

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

    def caop(self, current=None):
        return self._ca

    def cbop(self, current=None):
        return self._cb

    def ccop(self, current=None):
        return self._cc

    def cdop(self, current=None):
        return self._cd

    def iaop(self, current=None):
        return self._ia

    def ib1op(self, current=None):
        return self._ib1

    def ib2op(self, current=None):
        return self._ib2

    def icop(self, current=None):
        return self._ic
