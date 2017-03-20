# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def onewaysFuture(communicator, proxy):

    p = Test.MyClassPrx.uncheckedCast(proxy.ice_oneway())

    f = p.ice_pingAsync()
    f.sent()

    try:
        p.ice_isAAsync(Test.MyClass.ice_staticId())
        test(False)
    except RuntimeError:
        pass

    try:
        p.ice_idAsync()
        test(False)
    except RuntimeError:
        pass

    try:
        p.ice_idsAsync()
        test(False)
    except RuntimeError:
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
    except RuntimeError:
        pass
