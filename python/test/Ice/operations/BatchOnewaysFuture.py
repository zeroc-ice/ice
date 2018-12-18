# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
        with self._cond:
            while not self._called:
                self._cond.wait()
            self._called = False

    def called(self):
        with self._cond:
            self._called = True
            self._cond.notify()

def batchOneways(p):

    if sys.version_info[0] == 2:
        bs1 = []
        bs1[0:10 * 1024] = range(0, 10 * 1024) # add 100,000 entries.
        bs1 = ['\x00' for x in bs1] # set them all to \x00
        bs1 = ''.join(bs1) # make into a byte array
    else:
        bs1 = bytes([0 for x in range(0, 10 * 1024)])
    batch = Test.MyClassPrx.uncheckedCast(p.ice_batchOneway())

    f = batch.ice_flushBatchRequestsAsync() # Empty flush
    f.result()

    test(batch.ice_flushBatchRequestsAsync().is_sent()) # Empty flush
    test(batch.ice_flushBatchRequestsAsync().done()) # Empty flush
    test(batch.ice_flushBatchRequestsAsync().is_sent_synchronously()) # Empty flush

    for i in range(30):
        batch.opByteSOnewayAsync(bs1)

    count = 0
    while count < 27: # 3 * 9 requests auto-flushed.
        count += p.opByteSOnewayCallCount()
        time.sleep(0.01)

    if p.ice_getConnection():

        batch1 = Test.MyClassPrx.uncheckedCast(p.ice_batchOneway())
        batch2 = Test.MyClassPrx.uncheckedCast(p.ice_batchOneway())

        batch1.ice_pingAsync()
        batch2.ice_pingAsync()
        batch1.ice_flushBatchRequestsAsync().result()
        batch1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait)
        batch1.ice_pingAsync()
        batch2.ice_pingAsync()

        batch1.ice_getConnection()
        batch2.ice_getConnection()

        batch1.ice_pingAsync()
        batch1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait)

        test(batch1.ice_pingAsync().done() and not batch1.ice_pingAsync().exception())
        test(batch2.ice_pingAsync().done() and not batch1.ice_pingAsync().exception())

    identity = Ice.Identity()
    identity.name = "invalid";
    batch3 = batch.ice_identity(identity)
    batch3.ice_ping()
    batch3.ice_flushBatchRequestsAsync().result()

    # Make sure that a bogus batch request doesn't cause troubles to other ones.
    batch3.ice_ping()
    batch.ice_ping()
    batch.ice_flushBatchRequestsAsync().result()
    batch.ice_ping()
