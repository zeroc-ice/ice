# Copyright (c) ZeroC, Inc.

import Ice
import Test


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

def customSliceLoader(typeId):
    if typeId == "::Test::B":
        return BI()
    elif typeId == "::Test::C":
        return CI()
    elif typeId == "::Test::D":
        return DI()
    else:
        return None

class InitialI(Test.Initial):
    def __init__(self, adapter):
        self._adapter = adapter
        self._b1 = BI()
        self._b2 = BI()
        self._c = CI()
        self._d = DI()

        self._b1.theA = self._b2  # Cyclic reference to another B
        self._b1.theB = self._b1  # Self reference.
        self._b1.theC = None  # Null reference.

        self._b2.theA = self._b2  # Self reference, using base.
        self._b2.theB = self._b1  # Cyclic reference to another B
        self._b2.theC = self._c  # Cyclic reference to a C.

        self._c.theB = self._b2  # Cyclic reference to a B.

        self._d.theA = self._b1  # Reference to a B.
        self._d.theB = self._b2  # Reference to a B.
        self._d.theC = None  # Reference to a C.

    def shutdown(self, current):
        self._shutdown()

    def _shutdown(self):
        self._adapter.getCommunicator().shutdown()

    def getB1(self, current):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        return self._b1

    def getB2(self, current):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        return self._b2

    def getC(self, current):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        return self._c

    def getD(self, current):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        self._d.preMarshalInvoked = False
        return self._d

    def setRecursive(self, r, current):
        pass

    def setCycle(self, r, current):
        pass

    def acceptsClassCycles(self, current):
        return True

    def getMB(self, current):
        return Test.Initial.GetMBMarshaledResult(self._b1, current)

    def getAMDMB(self, current):
        return Ice.Future.completed(
            Test.Initial.GetAMDMBMarshaledResult(self._b1, current)
        )

    def getAll(self, current):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        self._d.preMarshalInvoked = False
        return (self._b1, self._b2, self._c, self._d)

    def getK(self, current):
        return Test.K(Test.L("l"))

    def opValue(self, v1, current):
        return v1, v1

    def opValueSeq(self, v1, current):
        return v1, v1

    def opValueMap(self, v1, current):
        return v1, v1

    def getD1(self, d1, current):
        return d1

    def throwEDerived(self, current):
        raise Test.EDerived(Test.A1("a1"), Test.A1("a2"), Test.A1("a3"), Test.A1("a4"))

    def setG(self, g, current):
        pass

    def opBaseSeq(self, inSeq, current):
        return (inSeq, inSeq)

    def getCompact(self, current):
        return Test.CompactExt()

    def getInnerA(self, current):
        return Test.Inner.A(self._b1)

    def getInnerSubA(self, current):
        return Test.Inner.Sub.A(Test.Inner.A(self._b1))

    def throwInnerEx(self, current):
        raise Test.Inner.Ex("Inner::Ex")

    def throwInnerSubEx(self, current):
        raise Test.Inner.Sub.Ex("Inner::Sub::Ex")

    def opM(self, m, current):
        return (m, m)

    def opF1(self, f11, current):
        return (f11, Test.F1("F12"))

    def opF2(self, f21, current):
        return (
            f21,
            Test.F2Prx.uncheckedCast(
                current.adapter.getCommunicator().stringToProxy("F22")
            ),
        )

    def opF3(self, f31, current):
        return (
            f31,
            Test.F3(
                Test.F1("F12"),
                Test.F2Prx.uncheckedCast(
                    current.adapter.getCommunicator().stringToProxy("F22")
                ),
            ),
        )

    def hasF3(self, current):
        return True


class UnexpectedObjectExceptionTestI(Test.UnexpectedObjectExceptionTest):
    def op(self, current):
        return Test.AlsoEmpty()


class F2I(Test.F2):
    def op(self, current):
        pass
