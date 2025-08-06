# Copyright (c) ZeroC, Inc.

import threading
import time
from typing import override

from generated.test.Ice.ami import Test
from generated.test.Ice.ami.Test.Outer.Inner import TestIntf as Inner_TestIntf

import Ice


class TestIntfI(Test.TestIntf):
    def __init__(self):
        self._cond = threading.Condition()
        self._batchCount = 0
        self._pending = None
        self._shutdown = False

    @override
    def op(self, current: Ice.Current):
        pass

    @override
    def opWithResult(self, current: Ice.Current):
        return 15

    @override
    def opWithUE(self, current: Ice.Current):
        raise Test.TestIntfException()

    @override
    def opWithPayload(self, seq: bytes, current: Ice.Current):
        pass

    @override
    def opBatch(self, current: Ice.Current):
        with self._cond:
            self._batchCount += 1
            self._cond.notify()

    @override
    def opBatchCount(self, current: Ice.Current):
        with self._cond:
            return self._batchCount

    @override
    def waitForBatch(self, count: int, current: Ice.Current):
        with self._cond:
            while self._batchCount < count:
                self._cond.wait(5)
            result = count == self._batchCount
            self._batchCount = 0
            return result

    @override
    def closeConnection(self, current: Ice.Current):
        # We can't wait for the connection to close - it would self-deadlock. So we just initiate the closure.
        def close_connection(future: Ice.Future):
            try:
                future.result()
            except Exception as ex:
                print("closeConnection failed: ", ex)

        assert current.con is not None
        current.con.close().add_done_callback(close_connection)

    @override
    def abortConnection(self, current: Ice.Current):
        assert current.con is not None
        current.con.abort()

    @override
    def sleep(self, ms: int, current: Ice.Current):
        time.sleep(ms / 1000.0)

    @override
    def startDispatch(self, current: Ice.Current):
        with self._cond:
            if self._shutdown:
                # Ignore, this can occur with the forceful connection close test, shutdown can be dispatch
                # before start dispatch.
                v = Ice.Future()
                v.set_result(None)
                return v
            elif self._pending:
                self._pending.set_result(None)
            self._pending = Ice.Future()
            return self._pending

    @override
    def finishDispatch(self, current: Ice.Current):
        with self._cond:
            if self._shutdown:
                return
            elif self._pending:  # Pending might not be set yet if startDispatch is dispatch out-of-order
                self._pending.set_result(None)
                self._pending = None

    @override
    def shutdown(self, current: Ice.Current):
        with self._cond:
            self._shutdown = True
            if self._pending:
                self._pending.set_result(None)
                self._pending = None
            current.adapter.getCommunicator().shutdown()

    @override
    def supportsFunctionalTests(self, current: Ice.Current):
        return False

    @override
    def supportsBackPressureTests(self, current: Ice.Current):
        return True

    @override
    async def pingBiDir(self, reply: Test.PingReplyPrx | None, current: Ice.Current):
        assert reply is not None
        assert current.con is not None
        expectSuccess = "ONE" not in current.ctx
        try:
            await reply.ice_fixed(current.con).replyAsync()
            if not expectSuccess:
                raise Test.TestIntfException()
        except Ice.ObjectNotExistException:
            if expectSuccess:
                raise Test.TestIntfException()


class TestIntfII(Inner_TestIntf):
    @override
    def op(self, i: int, current: Ice.Current) -> tuple[int, int]:
        return (i, i)


class TestIntfControllerI(Test.TestIntfController):
    def __init__(self, adapter: Ice.ObjectAdapter):
        self._adapter = adapter

    def holdAdapter(self, current: Ice.Current):
        self._adapter.hold()

    def resumeAdapter(self, current: Ice.Current):
        self._adapter.activate()
