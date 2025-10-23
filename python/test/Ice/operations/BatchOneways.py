# Copyright (c) ZeroC, Inc.

import time
from typing import cast

from generated.test.Ice.operations import Test
from TestHelper import test

import Ice


class BatchRequestInterceptor:
    def __init__(self):
        self._enabled = False
        self._count = 0
        self._size = 0
        self._lastRequestSize = 0

    def enqueue(self, request: Ice.BatchRequest, count: int, size: int):
        test(request.getOperation() == "opByteSOneway" or request.getOperation() == "ice_ping")
        test(request.getProxy().ice_isBatchOneway())

        if count > 0:
            test(self._lastRequestSize + self._size == size)

        self._count = count
        self._size = size

        if self._size + request.getSize() > 25000:
            f = request.getProxy().ice_flushBatchRequestsAsync()
            assert isinstance(f, Ice.Future)
            f.result()
            self._size = 18  # header

        if self._enabled:
            self._lastRequestSize = request.getSize()
            self._count += 1
            request.enqueue()

    def setEnabled(self, v: bool):
        self._enabled = v

    def count(self):
        return self._count


def batchOneways(p: Test.MyClassPrx) -> None:
    bs1 = bytes([0 for _ in range(0, 10 * 1024)])

    p.opByteSOneway(bs1)
    batch = Test.MyClassPrx.uncheckedCast(p.ice_batchOneway())

    batch.ice_flushBatchRequests()  # Empty flush
    batchConnection = batch.ice_getConnection()
    if batchConnection:
        batchConnection.flushBatchRequests(Ice.CompressBatch.BasedOnProxy)
    batch.ice_getCommunicator().flushBatchRequests(Ice.CompressBatch.BasedOnProxy)

    p.opByteSOnewayCallCount()  # Reset the call count

    for _ in range(30):
        batch.opByteSOneway(bs1)

    count = 0
    while count < 27:  # 3 * 9 requests auto-flushed.
        count += p.opByteSOnewayCallCount()
        time.sleep(0.01)

    if p.ice_getConnection():
        batch1 = Test.MyClassPrx.uncheckedCast(p.ice_batchOneway())
        batch2 = Test.MyClassPrx.uncheckedCast(p.ice_batchOneway())

        batch1.ice_ping()
        batch2.ice_ping()
        batch1.ice_flushBatchRequests()
        cast(Ice.Connection, batch1.ice_getConnection()).close().result()
        batch1.ice_ping()
        batch2.ice_ping()

        batch1.ice_getConnection()
        batch2.ice_getConnection()

        batch1.ice_ping()
        cast(Ice.Connection, batch1.ice_getConnection()).close().result()

        batch1.ice_ping()
        batch2.ice_ping()

    identity = Ice.Identity()
    identity.name = "invalid"
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
        interceptor = BatchRequestInterceptor()
        initData.batchRequestInterceptor = interceptor.enqueue

        ic = Ice.Communicator(initData=initData)

        batch = Test.MyClassPrx(ic, p.ice_toString()).ice_batchOneway()

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

        batch.opByteSOneway(bs1)  # This should trigger the flush
        batch.ice_ping()
        test(interceptor.count() == 2)

        ic.destroy()
