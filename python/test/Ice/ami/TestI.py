# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
        self._pending = []

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
        f = Ice.Future()
        with self._cond:
            self._pending.append(f)
        return f

    def finishDispatch(self, current=None):
        with self._cond:
            for f in self._pending:
                f.set_result(None)
            self._pending = []

    def shutdown(self, current=None):
        #
        # Just in case a request arrived late.
        #
        with self._cond:
            for f in self._pending:
                f.set_result(None)
        current.adapter.getCommunicator().shutdown()

    def supportsAMD(self, current=None):
        return True

    def supportsFunctionalTests(self, current=None):
        return False

    def pingBiDir(self, id, current = None):
        Test.PingReplyPrx.uncheckedCast(current.con.createProxy(id)).reply()

class TestIntfControllerI(Test.TestIntfController):
    def __init__(self, adapter):
        self._adapter = adapter

    def holdAdapter(self, current=None):
        self._adapter.hold()

    def resumeAdapter(self, current=None):
        self._adapter.activate()
