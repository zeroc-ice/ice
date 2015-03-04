# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, array, sys

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

def batchOneways(p):

    if sys.version_info[0] == 2:
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
    else:
        bs1 = bytes([0 for x in range(0, 10 * 1024)])
        bs2 = bytes([0 for x in range(0, 99 * 1024)])
        bs3 = bytes([0 for x in range(0, 100 * 1024)])

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

    batch2 = Test.MyClassPrx.uncheckedCast(p.ice_batchOneway())

    batch.ice_ping()
    batch2.ice_ping()
    batch.ice_flushBatchRequests()
    batch.ice_getConnection().close(False)
    batch.ice_ping()
    batch2.ice_ping()

    batch.ice_getConnection()
    batch2.ice_getConnection()

    batch.ice_ping()
    batch.ice_getConnection().close(False)
    try:
        batch.ice_ping()
        test(False)
    except Ice.CloseConnectionException:
        pass

    try:
        batch2.ice_ping()
        test(False)
    except Ice.CloseConnectionException:
        pass

    batch.ice_ping()
    batch2.ice_ping()

    identity = Ice.Identity()
    identity.name = "invalid";
    batch3 = batch.ice_identity(identity)
    batch3.ice_ping()
    batch3.ice_flushBatchRequests()
    
    # Make sure that a bogus batch request doesn't cause troubles to other ones.
    batch3.ice_ping()
    batch.ice_ping()
    batch.ice_flushBatchRequests()
    batch.ice_ping()
