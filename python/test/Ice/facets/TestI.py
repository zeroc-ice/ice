# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Test

class AI(Test._ADisp):
    def callA(self, current=None):
        return "A"

class BI(Test._BDisp, AI):
    def callB(self, current=None):
        return "B"

class CI(Test._CDisp, AI):
    def callC(self, current=None):
        return "C"

class DI(Test._DDisp, BI, CI):
    def callD(self, current=None):
        return "D"

class EI(Test._EDisp):
    def callE(self, current=None):
        return "E"

class FI(Test._FDisp, EI):
    def callF(self, current=None):
        return "F"

class GI(Test._GDisp):
    def __init__(self, communicator):
        self._communicator = communicator

    def shutdown(self, current=None):
        self._communicator.shutdown()

    def callG(self, current=None):
        return "G"

class HI(Test._HDisp, GI):
    def __init__(self, communicator):
        GI.__init__(self, communicator)

    def callH(self, current=None):
        return "H"
