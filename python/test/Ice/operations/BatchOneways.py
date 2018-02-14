# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, array, sys, time

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class BatchRequestInterceptorI(Ice.BatchRequestInterceptor):

    def __init__(self):
        self._enabled = False
        self._count = 0
        self._size = 0
        self._lastRequestSize = 0

    def enqueue(self, request, count, size):
        test(request.getOperation() == "opByteSOneway" or request.getOperation() == "ice_ping")
        test(request.getProxy().ice_isBatchOneway())

        if count > 0:
            test(self._lastRequestSize + self._size == size)

        self._count = count
        self._size = size

        if self._size + request.getSize() > 25000:
            request.getProxy().begin_ice_flushBatchRequests()
            self._size = 18 # header

        if self._enabled:
            self._lastRequestSize = request.getSize()
            self._count += 1
            request.enqueue()

    def setEnabled(self, v):
        self._enabled = v

    def count(self):
        return self._count

def batchOneways(p):

    if sys.version_info[0] == 2:
        bs1 = []
        bs1[0:10 * 1024] = range(0, 10 * 1024) # add 100,000 entries.
        bs1 = ['\x00' for x in bs1] # set them all to \x00
        bs1 = ''.join(bs1) # make into a byte array
    else:
        bs1 = bytes([0 for x in range(0, 10 * 1024)])

    try:
        p.opByteSOneway(bs1)
    except Ice.MemoryLimitException:
        test(False)

    batch = Test.MyClassPrx.uncheckedCast(p.ice_batchOneway())

    batch.ice_flushBatchRequests() # Empty flush

    p.opByteSOnewayCallCount() # Reset the call count

    for i in range(30):
        batch.opByteSOneway(bs1)

    count = 0
    while count < 27: # 3 * 9 requests auto-flushed.
        count += p.opByteSOnewayCallCount()
        time.sleep(0.01)

    if p.ice_getConnection():
        batch1 = Test.MyClassPrx.uncheckedCast(p.ice_batchOneway())
        batch2 = Test.MyClassPrx.uncheckedCast(p.ice_batchOneway())

        batch1.ice_ping()
        batch2.ice_ping()
        batch1.ice_flushBatchRequests()
        batch1.ice_getConnection().close(False)
        batch1.ice_ping()
        batch2.ice_ping()

        batch1.ice_getConnection()
        batch2.ice_getConnection()

        batch1.ice_ping()
        batch1.ice_getConnection().close(False)

        batch1.ice_ping()
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

    if batch.ice_getConnection():
        initData = Ice.InitializationData()
        initData.properties = p.ice_getCommunicator().getProperties().clone()
        interceptor = BatchRequestInterceptorI()
        initData.batchRequestInterceptor = interceptor

        ic = Ice.initialize(data=initData)

        batch = Test.MyClassPrx.uncheckedCast(ic.stringToProxy(p.ice_toString())).ice_batchOneway()

        test(interceptor.count() == 0)
        batch.ice_ping()
        batch.ice_ping()
        batch.ice_ping()
        test(interceptor.count() == 0)

        interceptor.setEnabled(True)
        batch.ice_ping()
        batch.ice_ping()
        batch.ice_ping()
        test(interceptor.count() == 3)

        batch.ice_flushBatchRequests()
        batch.ice_ping()
        test(interceptor.count() == 1)

        batch.opByteSOneway(bs1)
        test(interceptor.count() == 2)
        batch.opByteSOneway(bs1)
        test(interceptor.count() == 3)

        batch.opByteSOneway(bs1) # This should trigger the flush
        batch.ice_ping()
        test(interceptor.count() == 2)

        ic.destroy()
