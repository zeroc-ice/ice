# Copyright (c) ZeroC, Inc.

import Test


class MyDerivedClassI(Test.MyDerivedClass):
    def __init__(self):
        self.ctx = None

    def shutdown(self, current):
        current.adapter.getCommunicator().shutdown()

    def getContext(self, current):
        return self.ctx

    def echo(self, obj, current):
        return obj

    def ice_isA(self, s, current):
        self.ctx = current.ctx
        return Test.MyDerivedClass.ice_isA(self, s, current)
