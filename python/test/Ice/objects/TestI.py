# Copyright (c) ZeroC, Inc.

from generated.test.Ice.objects import Test
from generated.test.Ice.objects.server_private.Test import UnexpectedObjectExceptionTest
from Objects import BI, CI, DI

import Ice


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

    def shutdown(self, current: Ice.Current):
        self._shutdown()

    def _shutdown(self):
        self._adapter.getCommunicator().shutdown()

    def getB1(self, current: Ice.Current):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        return self._b1

    def getB2(self, current: Ice.Current):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        return self._b2

    def getC(self, current: Ice.Current):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        return self._c

    def getD(self, current: Ice.Current):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        self._d.preMarshalInvoked = False
        return self._d

    def setRecursive(self, r, current: Ice.Current):
        pass

    def setCycle(self, r, current: Ice.Current):
        pass

    def acceptsClassCycles(self, current: Ice.Current):
        return True

    def getMB(self, current: Ice.Current):
        return Test.Initial.GetMBMarshaledResult(self._b1, current)

    def getAMDMB(self, current: Ice.Current):
        return Ice.Future.completed(Test.Initial.GetAMDMBMarshaledResult(self._b1, current))

    def getAll(self, current: Ice.Current):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        self._d.preMarshalInvoked = False
        return (self._b1, self._b2, self._c, self._d)

    def getK(self, current: Ice.Current):
        return Test.K(Test.L("l"))

    def opValue(self, v1, current: Ice.Current):
        return v1, v1

    def opValueSeq(self, v1, current: Ice.Current):
        return v1, v1

    def opValueMap(self, v1, current: Ice.Current):
        return v1, v1

    def getD1(self, d1, current: Ice.Current):
        return d1

    def throwEDerived(self, current: Ice.Current):
        raise Test.EDerived(Test.A1("a1"), Test.A1("a2"), Test.A1("a3"), Test.A1("a4"))

    def setG(self, g, current: Ice.Current):
        pass

    def opBaseSeq(self, inSeq, current: Ice.Current):
        return (inSeq, inSeq)

    def getCompact(self, current: Ice.Current):
        return Test.CompactExt()

    def getInnerA(self, current: Ice.Current):
        return Test.Inner.A(self._b1)

    def getInnerSubA(self, current: Ice.Current):
        return Test.Inner.Sub.A(Test.Inner.A(self._b1))

    def throwInnerEx(self, current: Ice.Current):
        raise Test.Inner.Ex("Inner::Ex")

    def throwInnerSubEx(self, current: Ice.Current):
        raise Test.Inner.Sub.Ex("Inner::Sub::Ex")

    def opM(self, m, current: Ice.Current):
        return (m, m)

    def opF1(self, f11, current: Ice.Current):
        return (f11, Test.F1("F12"))

    def opF2(self, f21, current: Ice.Current):
        return (
            f21,
            Test.F2Prx.uncheckedCast(current.adapter.getCommunicator().stringToProxy("F22")),
        )

    def opF3(self, f31, current: Ice.Current):
        return (
            f31,
            Test.F3(
                Test.F1("F12"),
                Test.F2Prx.uncheckedCast(current.adapter.getCommunicator().stringToProxy("F22")),
            ),
        )

    def hasF3(self, current: Ice.Current):
        return True


class UnexpectedObjectExceptionTestI(UnexpectedObjectExceptionTest):
    def op(self, current: Ice.Current):
        return Test.AlsoEmpty()


class F2I(Test.F2):
    def op(self, current: Ice.Current):
        pass
