# Copyright (c) ZeroC, Inc.

from generated.test.Ice.operations import Test
from TestHelper import TestHelper, test

import Ice


def oneways(helper: TestHelper, p: Test.MyClassPrx) -> None:
    p = Test.MyClassPrx.uncheckedCast(p.ice_oneway())

    #
    # ice_ping
    #
    p.ice_ping()

    #
    # opVoid
    #
    p.opVoid()

    #
    # opIdempotent
    #
    p.opIdempotent()

    #
    # opByte
    #
    try:
        p.opByte(0xFF, 0x0F)
        test(False)
    except Ice.TwowayOnlyException:
        pass
