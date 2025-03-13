# Copyright (c) ZeroC, Inc.

import Ice
import Test
import threading
import time


class TestIntfI(Test.TestIntf):
    def __init__(self):
        self._cond = threading.Condition()
        self._batchCount = 0
        self._pending = None
        self._shutdown = False

    def op(self, current):
        pass

    def opWithResult(self, current):
        return 15

    def opWithUE(self, current):
        raise Test.TestIntfException()

    def opWithPayload(self, bytes, current):
        pass

    def opBatch(self, current):
        with self._cond:
            self._batchCount += 1
            self._cond.notify()

    def opBatchCount(self, current):
        with self._cond:
            return self._batchCount

    def waitForBatch(self, count, current):
        with self._cond:
            while self._batchCount < count:
                self._cond.wait(5)
            result = count == self._batchCount
            self._batchCount = 0
            return result

    def closeConnection(self, current):
        # We can't wait for the connection to close - it would self-deadlock. So we just initiate the closure.

        def connection_close(future):
            try:
                future.result()
            except Exception as ex:
                print("closeConnection failed: ", ex)

        current.con.close().add_done_callback(connection_close)

    def abortConnection(self, current):
        current.con.abort()

    def sleep(self, ms, current):
        time.sleep(ms / 1000.0)

    def startDispatch(self, current):
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

    def finishDispatch(self, current):
        with self._cond:
            if self._shutdown:
                return
            elif (
                self._pending
            ):  # Pending might not be set yet if startDispatch is dispatch out-of-order
                self._pending.set_result(None)
                self._pending = None

    def shutdown(self, current):
        with self._cond:
            self._shutdown = True
            if self._pending:
                self._pending.set_result(None)
                self._pending = None
            current.adapter.getCommunicator().shutdown()

    def supportsFunctionalTests(self, current):
        return False

    def supportsBackPressureTests(self, current):
        return True

    async def pingBiDir(self, reply, current):
        expectSuccess = "ONE" not in current.ctx
        try:
            await reply.ice_fixed(current.con).replyAsync()
            if not expectSuccess:
                raise Test.TestIntfException()
        except Ice.ObjectNotExistException:
            if expectSuccess:
                raise Test.TestIntfException()


class TestIntfII(Test.Outer.Inner.TestIntf):
    def op(self, i, current):
        return (i, i)


class TestIntfControllerI(Test.TestIntfController):
    def __init__(self, adapter):
        self._adapter = adapter

    def holdAdapter(self, current):
        self._adapter.hold()

    def resumeAdapter(self, current):
        self._adapter.activate()
