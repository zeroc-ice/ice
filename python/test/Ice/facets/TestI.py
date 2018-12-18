# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Test

class AI(Test.A):
    def callA(self, current=None):
        return "A"

class BI(Test.B, AI):
    def callB(self, current=None):
        return "B"

class CI(Test.C, AI):
    def callC(self, current=None):
        return "C"

class DI(Test.D, BI, CI):
    def callD(self, current=None):
        return "D"

class EI(Test.E):
    def callE(self, current=None):
        return "E"

class FI(Test.F, EI):
    def callF(self, current=None):
        return "F"

class GI(Test.G):
    def __init__(self, communicator):
        self._communicator = communicator

    def shutdown(self, current=None):
        self._communicator.shutdown()

    def callG(self, current=None):
        return "G"

class HI(Test.H, GI):
    def __init__(self, communicator):
        GI.__init__(self, communicator)

    def callH(self, current=None):
        return "H"
