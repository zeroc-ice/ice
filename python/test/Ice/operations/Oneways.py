# Copyright (c) ZeroC, Inc.

import Ice
import Test


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


def oneways(helper, p):
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
    except Ice.TwowayOnlyException:
        pass
