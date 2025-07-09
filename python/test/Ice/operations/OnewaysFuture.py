# Copyright (c) ZeroC, Inc.

import Test

import Ice


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


def onewaysFuture(helper, proxy):
    p = Test.MyClassPrx.uncheckedCast(proxy.ice_oneway())

    f = p.ice_pingAsync()
    f.sent()

    try:
        p.ice_isAAsync(Test.MyClass.ice_staticId())
        test(False)
    except Ice.TwowayOnlyException:
        pass

    try:
        p.ice_idAsync()
        test(False)
    except Ice.TwowayOnlyException:
        pass

    try:
        p.ice_idsAsync()
        test(False)
    except Ice.TwowayOnlyException:
        pass

    f = p.opVoidAsync()
    f.sent()

    f = p.opIdempotentAsync()
    f.sent()

    try:
        p.opByteAsync(0xFF, 0x0F)
        test(False)
    except Ice.TwowayOnlyException:
        pass
