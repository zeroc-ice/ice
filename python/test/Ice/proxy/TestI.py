# Copyright (c) ZeroC, Inc.

from generated.test.Ice.proxy import Test


class MyDerivedClassI(Test.MyDerivedClass):
    def __init__(self):
        self.ctx = None

    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()

    def getContext(self, current: Ice.Current):
        return self.ctx

    def echo(self, obj, current: Ice.Current):
        return obj

    def ice_isA(self, s, current: Ice.Current):
        self.ctx = current.ctx
        return Test.MyDerivedClass.ice_isA(self, s, current)


class CI(Test.C):
    def __init__(self):
        self.ctx = None

    def opA(self, a, current: Ice.Current):
        return a

    def opB(self, b, current: Ice.Current):
        return b

    def opC(self, c, current: Ice.Current):
        return c

    def opS(self, s, current: Ice.Current):
        return s
