# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test

class BI(Test.B):
    def __init__(self):
        self.preMarshalInvoked = False
        self.postUnmarshalInvoked = False

    def ice_preMarshal(self):
        self.preMarshalInvoked = True

    def ice_postUnmarshal(self):
        self.postUnmarshalInvoked = True

class CI(Test.C):
    def __init__(self):
        self.preMarshalInvoked = False
        self.postUnmarshalInvoked = False

    def ice_preMarshal(self):
        self.preMarshalInvoked = True

    def ice_postUnmarshal(self):
        self.postUnmarshalInvoked = True

class DI(Test.D):
    def __init__(self):
        self.preMarshalInvoked = False
        self.postUnmarshalInvoked = False

    def ice_preMarshal(self):
        self.preMarshalInvoked = True

    def ice_postUnmarshal(self):
        self.postUnmarshalInvoked = True

class EI(Test.E):
    def __init__(self):
        Test.E.__init__(self, 1, "hello")

    def checkValues(self, current=None):
        return self._i == 1 and self._s == "hello"

class FI(Test.F):
    def __init__(self, e=None):
        Test.F.__init__(self, e, e)

    def checkValues(self, current=None):
        return self._e1 != None and self._e1 == self.e2

class II(Ice.InterfaceByValue):
    def __init__(self):
        Ice.InterfaceByValue.__init__(self, "::Test::I")

class JI(Ice.InterfaceByValue):
    def __init__(self):
        Ice.InterfaceByValue.__init__(self, "::Test::J")

class HI(Test.H):
    pass

class InitialI(Test.Initial):
    def __init__(self, adapter):
        self._adapter = adapter
        self._b1 = BI()
        self._b2 = BI()
        self._c = CI()
        self._d = DI()
        self._e = EI()
        self._f = FI(self._e)

        self._b1.theA = self._b2 # Cyclic reference to another B
        self._b1.theB = self._b1 # Self reference.
        self._b1.theC = None     # Null reference.

        self._b2.theA = self._b2 # Self reference, using base.
        self._b2.theB = self._b1 # Cyclic reference to another B
        self._b2.theC = self._c  # Cyclic reference to a C.

        self._c.theB = self._b2  # Cyclic reference to a B.

        self._d.theA = self._b1  # Reference to a B.
        self._d.theB = self._b2  # Reference to a B.
        self._d.theC = None      # Reference to a C.

    def shutdown(self, current=None):
        self._adapter.getCommunicator().shutdown()

    def getB1(self, current=None):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        return self._b1

    def getB2(self, current=None):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        return self._b2

    def getC(self, current=None):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        return self._c

    def getD(self, current=None):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        self._d.preMarshalInvoked = False
        return self._d

    def getE(self, current=None):
        return self._e

    def getF(self, current=None):
        return self._f

    def setRecursive(self, r, current):
        pass

    def supportsClassGraphDepthMax(self, current):
        return True

    def getMB(self, current):
        return Test.Initial.GetMBMarshaledResult(self._b1, current)

    def getAMDMB(self, current):
        return Ice.Future.completed(Test.Initial.GetAMDMBMarshaledResult(self._b1, current))

    def getAll(self, current=None):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        self._d.preMarshalInvoked = False
        return (self._b1, self._b2, self._c, self._d)

    def getI(self, current=None):
        return II()

    def getJ(self, current=None):
        return JI()

    def getH(self, current=None):
        return HI()

    def getK(self, current=None):
        return Test.K(Test.L("l"))

    def opValue(self, v1, current=None):
        return v1, v1

    def opValueSeq(self, v1, current=None):
        return v1, v1

    def opValueMap(self, v1, current=None):
        return v1, v1

    def getD1(self, d1, current=None):
        return d1

    def throwEDerived(self, current=None):
        raise Test.EDerived(Test.A1("a1"), Test.A1("a2"), Test.A1("a3"), Test.A1("a4"))

    def setG(self, g, current=None):
        pass

    def setI(self, i, current=None):
        pass

    def opBaseSeq(self, inSeq, current=None):
        return (inSeq, inSeq)

    def getCompact(self, current=None):
        return Test.CompactExt()

    def getInnerA(self, current=None):
        return Test.Inner.A(self._b1)

    def getInnerSubA(self, current=None):
        return Test.Inner.Sub.A(Test.Inner.A(self._b1))

    def throwInnerEx(self, current=None):
        raise Test.Inner.Ex("Inner::Ex")

    def throwInnerSubEx(self, current=None):
        raise Test.Inner.Sub.Ex("Inner::Sub::Ex")

    def opM(self, m, current=None):
        return (m, m)

class UnexpectedObjectExceptionTestI(Test.UnexpectedObjectExceptionTest):
    def op(self, current=None):
        return Test.AlsoEmpty()
