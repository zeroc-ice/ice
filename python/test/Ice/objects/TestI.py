# Copyright (c) ZeroC, Inc.

from collections.abc import Awaitable, Sequence
from typing import Mapping, override

from generated.test.Ice.objects import Test
from generated.test.Ice.objects.server_private.Test import UnexpectedObjectExceptionTest
from generated.test.Ice.objects.Test import Inner
from generated.test.Ice.objects.Test.Inner import Sub
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
    def acceptsClassCycles(self, current: Ice.Current) -> bool:
        return True

    @override
    def getMB(self, current: Ice.Current) -> Awaitable[Test.B]:
        return Ice.Future.completed(self._b1)

    @override
    def getAMDMB(self, current: Ice.Current) -> Awaitable[Test.B]:
        return Ice.Future.completed(self._b1)

    @override
    def getAll(self, current: Ice.Current) -> tuple[Test.B, Test.B, Test.C, Test.D]:
        self._b1.preMarshalInvoked = False
        self._b2.preMarshalInvoked = False
        self._c.preMarshalInvoked = False
        self._d.preMarshalInvoked = False
        return (self._b1, self._b2, self._c, self._d)

    @override
    def getK(self, current: Ice.Current) -> Test.K:
        return Test.K(Test.L("l"))

    @override
    def opValue(self, v1: Ice.Value | None, current: Ice.Current) -> tuple[Ice.Value | None, Ice.Value | None]:
        return v1, v1

    @override
    def opValueSeq(
        self, v1: list[Ice.Value | None], current: Ice.Current
    ) -> tuple[Sequence[Ice.Value | None], Sequence[Ice.Value | None]]:
        return v1, v1

    @override
    def opValueMap(
        self, v1: dict[str, Ice.Value | None], current: Ice.Current
    ) -> tuple[Mapping[str, Ice.Value | None], Mapping[str, Ice.Value | None]]:
        return v1, v1

    @override
    def getD1(self, d1: Test.D1 | None, current: Ice.Current) -> Test.D1 | None:
        return d1

    @override
    def throwEDerived(self, current: Ice.Current):
        raise Test.EDerived(Test.A1("a1"), Test.A1("a2"), Test.A1("a3"), Test.A1("a4"))

    @override
    def setG(self, theG: Test.G | None, current: Ice.Current):
        pass

    @override
    def opBaseSeq(
        self, inSeq: list[Test.Base | None], current: Ice.Current
    ) -> tuple[Sequence[Test.Base | None], Sequence[Test.Base | None]]:
        return (inSeq, inSeq)

    @override
    def getCompact(self, current: Ice.Current) -> Test.Compact | None:
        return Test.CompactExt()

    @override
    def getInnerA(self, current: Ice.Current) -> Inner.A:
        return Inner.A(self._b1)

    @override
    def getInnerSubA(self, current: Ice.Current) -> Sub.A:
        return Sub.A(Inner.A(self._b1))

    @override
    def throwInnerEx(self, current: Ice.Current):
        raise Inner.Ex("Inner::Ex")

    @override
    def throwInnerSubEx(self, current: Ice.Current):
        raise Sub.Ex("Inner::Sub::Ex")

    @override
    def opM(self, v1: Test.M | None, current: Ice.Current) -> tuple[Test.M | None, Test.M | None]:
        return (v1, v1)

    @override
    def opF1(self, f11: Test.F1 | None, current: Ice.Current) -> tuple[Test.F1 | None, Test.F1 | None]:
        return (f11, Test.F1("F12"))

    @override
    def opF2(self, f21: Test.F2Prx | None, current: Ice.Current) -> tuple[Test.F2Prx | None, Test.F2Prx | None]:
        return (
            f21,
            Test.F2Prx.uncheckedCast(current.adapter.getCommunicator().stringToProxy("F22")),
        )

    @override
    def opF3(self, f31: Test.F3 | None, current: Ice.Current) -> tuple[Test.F3 | None, Test.F3 | None]:
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
