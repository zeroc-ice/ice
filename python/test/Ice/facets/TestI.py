# Copyright (c) ZeroC, Inc.

import Test


class AI(Test.A):
    def callA(self, current):
        return "A"


class BI(Test.B, AI):
    def callB(self, current):
        return "B"


class CI(Test.C, AI):
    def callC(self, current):
        return "C"


class DI(Test.D, BI, CI):
    def callD(self, current):
        return "D"


class EI(Test.E):
    def callE(self, current):
        return "E"


class FI(Test.F, EI):
    def callF(self, current):
        return "F"


class GI(Test.G):
    def __init__(self, communicator):
        self._communicator = communicator

    def shutdown(self, current):
        self._communicator.shutdown()

    def callG(self, current):
        return "G"


class HI(Test.H, GI):
    def __init__(self, communicator):
        GI.__init__(self, communicator)

    def callH(self, current):
        return "H"
