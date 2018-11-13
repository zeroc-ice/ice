# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, threading, time

class TestIntfI(Test.TestIntf):
    def __init__(self):
        self._cond = threading.Condition()
        self._batchCount = 0
        self._pending = None
        self._shutdown = False

    def op(self, current=None):
        pass

    def opWithResult(self, current=None):
        return 15

    def opWithUE(self, current=None):
        raise Test.TestIntfException()

    def opWithPayload(self, bytes, current=None):
        pass

    def opBatch(self, current=None):
        with self._cond:
            self._batchCount += 1
            self._cond.notify()

    def opBatchCount(self, current=None):
        with self._cond:
            return self._batchCount

    def waitForBatch(self, count, current=None):
        with self._cond:
            while self._batchCount < count:
                self._cond.wait(5)
            result = count == self._batchCount
            self._batchCount = 0
            return result

    def close(self, mode, current=None):
        current.con.close(Ice.ConnectionClose.valueOf(mode.value))

    def sleep(self, ms, current=None):
        time.sleep(ms / 1000.0)

    def startDispatch(self, current=None):
        with self._cond:
            if self._shutdown:
                # Ignore, this can occur with the forcefull connection close test, shutdown can be dispatch
                # before start dispatch.
                v = Ice.Future()
                v.set_result(None)
                return v
            elif self._pending:
                self._pending.set_result(None)
            self._pending = Ice.Future()
            return self._pending

    def finishDispatch(self, current=None):
        with self._cond:
            if self._shutdown:
                return
            elif self._pending: # Pending might not be set yet if startDispatch is dispatch out-of-order
                self._pending.set_result(None)
                self._pending = None

    def shutdown(self, current=None):
        with self._cond:
            self._shutdown = True
            if self._pending:
                self._pending.set_result(None)
                self._pending = None
            current.adapter.getCommunicator().shutdown()

    def supportsAMD(self, current=None):
        return True

    def supportsFunctionalTests(self, current=None):
        return False

    def pingBiDir(self, reply, current=None):
        # TODO: verify correct thread with add_done_callback_async
        reply.ice_fixed(current.con).replyAsync().result()

class TestIntfII(Test.Outer.Inner.TestIntf):
    def op(self, i, current):
        return (i, i)

class TestIntfControllerI(Test.TestIntfController):
    def __init__(self, adapter):
        self._adapter = adapter

    def holdAdapter(self, current=None):
        self._adapter.hold()

    def resumeAdapter(self, current=None):
        self._adapter.activate()
