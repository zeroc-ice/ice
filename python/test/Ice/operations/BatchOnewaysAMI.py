# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, array, sys, threading, time

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class Callback:
    def __init__(self):
        self._called = False
        self._cond = threading.Condition()

    def check(self):
        self._cond.acquire()
        try:
            while not self._called:
                self._cond.wait()
            self._called = False
        finally:
            self._cond.release()

    def called(self):
        self._cond.acquire()
        self._called = True
        self._cond.notify()
        self._cond.release()

def batchOneways(p):

    if sys.version_info[0] == 2:
        bs1 = []
        bs1[0:10 * 1024] = range(0, 10 * 1024) # add 100,000 entries.
        bs1 = ['\x00' for x in bs1] # set them all to \x00
        bs1 = ''.join(bs1) # make into a byte array
    else:
        bs1 = bytes([0 for x in range(0, 10 * 1024)])
    batch = Test.MyClassPrx.uncheckedCast(p.ice_batchOneway())

    batch.end_ice_flushBatchRequests(batch.begin_ice_flushBatchRequests()) # Empty flush
    test(batch.begin_ice_flushBatchRequests().isSent()) # Empty flush
    test(batch.begin_ice_flushBatchRequests().isCompleted()) # Empty flush
    test(batch.begin_ice_flushBatchRequests().sentSynchronously()) # Empty flush

    for i in range(30):
        batch.begin_opByteSOneway(bs1, lambda: 0, lambda ex: test(False) )

    count = 0
    while count < 27: # 3 * 9 requests auto-flushed.
        count += p.opByteSOnewayCallCount()
        time.sleep(0.01)

    if p.ice_getConnection():

        batch1 = Test.MyClassPrx.uncheckedCast(p.ice_batchOneway())
        batch2 = Test.MyClassPrx.uncheckedCast(p.ice_batchOneway())

        batch1.end_ice_ping(batch1.begin_ice_ping())
        batch2.end_ice_ping(batch2.begin_ice_ping())
        batch1.end_ice_flushBatchRequests(batch1.begin_ice_flushBatchRequests())
        batch1.ice_getConnection().close(False)
        batch1.end_ice_ping(batch1.begin_ice_ping())
        batch2.end_ice_ping(batch2.begin_ice_ping())

        batch1.ice_getConnection()
        batch2.ice_getConnection()

        batch1.ice_getConnection().close(False)

        batch1.end_ice_ping(batch1.begin_ice_ping())
        batch2.end_ice_ping(batch2.begin_ice_ping())

    identity = Ice.Identity()
    identity.name = "invalid";
    batch3 = batch.ice_identity(identity)
    batch3.ice_ping()
    batch3.end_ice_flushBatchRequests(batch3.begin_ice_flushBatchRequests())

    # Make sure that a bogus batch request doesn't cause troubles to other ones.
    batch3.ice_ping()
    batch.ice_ping()
    batch.end_ice_flushBatchRequests(batch.begin_ice_flushBatchRequests())
    batch.ice_ping()
