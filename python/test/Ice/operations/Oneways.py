# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, math, Test, array

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def oneways(helper, p):
    communicator = helper.communicator()
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
    # opNonmutating
    #
    p.opNonmutating()

    #
    # opByte
    #
    try:
        p.opByte(0xff, 0x0f)
    except Ice.TwowayOnlyException:
        pass
