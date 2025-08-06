# Copyright (c) ZeroC, Inc.

from generated.test.Ice.facets import Test

import Ice


class AI(Test.A):
    def callA(self, current: Ice.Current):
        return "A"


class BI(Test.B, AI):
    def callB(self, current: Ice.Current):
        return "B"


class CI(Test.C, AI):
    def callC(self, current: Ice.Current):
        return "C"


class DI(Test.D, BI, CI):
    def callD(self, current: Ice.Current):
        return "D"


class EI(Test.E):
    def callE(self, current: Ice.Current):
        return "E"


class FI(Test.F, EI):
    def callF(self, current: Ice.Current):
        return "F"


class GI(Test.G):
    def __init__(self, communicator):
        self._communicator = communicator

    def shutdown(self, current: Ice.Current):
        self._communicator.shutdown()

    def callG(self, current: Ice.Current):
        return "G"


class HI(Test.H, GI):
    def __init__(self, communicator):
        GI.__init__(self, communicator)

    def callH(self, current: Ice.Current):
        return "H"
