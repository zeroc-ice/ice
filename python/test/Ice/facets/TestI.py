# Copyright (c) ZeroC, Inc.

from typing import override

from generated.test.Ice.facets import Test

import Ice


class AI(Test.A):
    @override
    def callA(self, current: Ice.Current) -> str:
        return "A"


class BI(Test.B, AI):
    @override
    def callB(self, current: Ice.Current) -> str:
        return "B"


class CI(Test.C, AI):
    @override
    def callC(self, current: Ice.Current) -> str:
        return "C"


class DI(Test.D, BI, CI):
    @override
    def callD(self, current: Ice.Current) -> str:
        return "D"


class EI(Test.E):
    @override
    def callE(self, current: Ice.Current) -> str:
        return "E"


class FI(Test.F, EI):
    @override
    def callF(self, current: Ice.Current) -> str:
        return "F"


class GI(Test.G):
    def __init__(self, communicator: Ice.Communicator):
        self._communicator = communicator

    @override
    def shutdown(self, current: Ice.Current):
        self._communicator.shutdown()

    @override
    def callG(self, current: Ice.Current) -> str:
        return "G"


class HI(Test.H, GI):
    def __init__(self, communicator: Ice.Communicator):
        GI.__init__(self, communicator)

    @override
    def callH(self, current: Ice.Current) -> str:
        return "H"
