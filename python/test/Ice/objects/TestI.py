# Copyright (c) ZeroC, Inc.

from typing import override
from typing import override
from generated.test.Ice.objects import Test
from generated.test.Ice.objects.server_private.Test import UnexpectedObjectExceptionTest
from Objects import BI, CI, DI

import Ice


class InitialI(Test.Initial):
    def __init__(self, adapter: Ice.ObjectAdapter):
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

    @override
    def shutdown(self, current: Ice.Current):
        self._shutdown()

    def _shutdown(self):
        self._adapter.getCommunicator().shutdown()

    @override
    def getB1(self, current: Ice.Current):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        return self._b1

    @override
    def getB2(self, current: Ice.Current):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        return self._b2

    @override
    def getC(self, current: Ice.Current):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        return self._c

    @override
    def getD(self, current: Ice.Current):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        self._d.preMarshalInvoked = False
        return self._d

    @override
    def setRecursive(self, p: Test.Recursive | None, current: Ice.Current):
        pass

    @override
    def setCycle(self, r: Test.Recursive | None, current: Ice.Current):
        pass

    @override
    def acceptsClassCycles(self, current: Ice.Current):
        return True

    @override
    def getMB(self, current: Ice.Current) -> Test.B:
        return Test.Initial.GetMBMarshaledResult(self._b1, current)

    @override
    def getAMDMB(self, current: Ice.Current):
        return Ice.Future.completed(Test.Initial.GetAMDMBMarshaledResult(self._b1, current))

    @override
    def getAll(self, current: Ice.Current):
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        self._d.preMarshalInvoked = False
        return (self._b1, self._b2, self._c, self._d)

    @override
    def getK(self, current: Ice.Current):
        return Test.K(Test.L("l"))

    @override
    def opValue(self, v1, current: Ice.Current):
        return v1, v1

    @override
    def opValueSeq(self, v1, current: Ice.Current):
        return v1, v1

    @override
    def opValueMap(self, v1, current: Ice.Current):
        return v1, v1

    def getD1(self, d1, current: Ice.Current):
        return d1

    @override
    def throwEDerived(self, current: Ice.Current):
        raise Test.EDerived(Test.A1("a1"), Test.A1("a2"), Test.A1("a3"), Test.A1("a4"))

    @override
    def setG(self, g, current: Ice.Current):
        pass

    @override
    def opBaseSeq(self, inSeq, current: Ice.Current):
        return (inSeq, inSeq)

    @override
    def getCompact(self, current: Ice.Current):
        return Test.CompactExt()

    @override
    def getInnerA(self, current: Ice.Current):
        return Test.Inner.A(self._b1)

    @override
    def getInnerSubA(self, current: Ice.Current):
        return Test.Inner.Sub.A(Test.Inner.A(self._b1))

    @override
    def throwInnerEx(self, current: Ice.Current):
        raise Test.Inner.Ex("Inner::Ex")

    @override
    def throwInnerSubEx(self, current: Ice.Current):
        raise Test.Inner.Sub.Ex("Inner::Sub::Ex")

    @override
    def opM(self, m, current: Ice.Current):
        return (m, m)

    @override
    def opF1(self, f11, current: Ice.Current):
        return (f11, Test.F1("F12"))

    @override
    def opF2(self, f21, current: Ice.Current):
        return (
            f21,
            Test.F2Prx.uncheckedCast(current.adapter.getCommunicator().stringToProxy("F22")),
        )

    @override
    def opF3(self, f31, current: Ice.Current):
        return (
            f31,
            Test.F3(
                Test.F1("F12"),
                Test.F2Prx.uncheckedCast(current.adapter.getCommunicator().stringToProxy("F22")),
            ),
        )

    @override
    def hasF3(self, current: Ice.Current):
        return True


class UnexpectedObjectExceptionTestI(UnexpectedObjectExceptionTest):
    def op(self, current: Ice.Current):
        return Test.AlsoEmpty()


class F2I(Test.F2):
    def op(self, current: Ice.Current):
        pass
