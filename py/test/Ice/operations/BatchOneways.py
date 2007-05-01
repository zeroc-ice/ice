# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, array

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def batchOneways(p):

    bs1 = []
    bs1[0:10 * 1024] = range(0, 10 * 1024) # add 100,000 entries.
    bs1 = ['\x00' for x in bs1] # set them all to \x00
    bs1 = ''.join(bs1) # make into a byte array

    bs2 = []
    bs2[0:99 * 1024] = range(0, 99 * 1024) # add 100,000 entries.
    bs2 = ['\x00' for x in bs2] # set them all to \x00
    bs2 = ''.join(bs2) # make into a byte array

    bs3 = []
    bs3[0:100 * 1024] = range(0, 100 * 1024) # add 100,000 entries.
    bs3 = ['\x00' for x in bs3] # set them all to \x00
    bs3 = ''.join(bs3) # make into a byte array

    try:
        p.opByteSOneway(bs1)
    except Ice.MemoryLimitException:
        test(False)

    try:
        p.opByteSOneway(bs2)
    except Ice.MemoryLimitException:
        test(False)

    try:
        p.opByteSOneway(bs3)
        test(False)
    except Ice.MemoryLimitException:
        pass

    batch = Test.MyClassPrx.uncheckedCast(p.ice_batchOneway())

    for i in range(30):
        try:
            batch.opByteSOneway(bs1)
        except Ice.MemoryLimitException:
            test(False)

    batch.ice_getConnection().flushBatchRequests()
