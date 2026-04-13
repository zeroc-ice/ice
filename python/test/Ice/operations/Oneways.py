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
    # Calling a ["oneway"] operation on a oneway proxy succeeds.
    #
    p.opOneway()

    #
    # Calling a ["oneway"] operation on a twoway proxy throws OnewayOnlyException.
    #
    try:
        Test.MyClassPrx.uncheckedCast(p.ice_twoway()).opOneway()
        test(False)
    except Ice.OnewayOnlyException:
        pass

    #
    # opByte
    #
    try:
        p.opByte(0xFF, 0x0F)
        test(False)
    except Ice.TwowayOnlyException:
        pass
