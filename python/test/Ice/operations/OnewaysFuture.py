# Copyright (c) ZeroC, Inc.

from generated.test.Ice.operations import Test
from TestHelper import TestHelper, test

import Ice


def onewaysFuture(helper: TestHelper, proxy: Test.MyClassPrx) -> None:
    p = Test.MyClassPrx.uncheckedCast(proxy.ice_oneway())

    f = p.ice_pingAsync()
    assert isinstance(f, Ice.InvocationFuture)
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
    assert isinstance(f, Ice.InvocationFuture)
    f.sent()

    f = p.opIdempotentAsync()
    assert isinstance(f, Ice.InvocationFuture)
    f.sent()

    try:
        p.opByteAsync(0xFF, 0x0F)
        test(False)
    except Ice.TwowayOnlyException:
        pass
