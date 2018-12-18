# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

import Ice, Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def onewaysFuture(helper, proxy):
    communicator = helper.communicator()
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

    f = p.opNonmutatingAsync()
    f.sent()

    try:
        p.opByteAsync(0xff, 0x0f)
        test(False)
    except Ice.TwowayOnlyException:
        pass
