# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, threading

class TestIntfI(Test._TestIntfDisp):
    def __init__(self):
        self._cond = threading.Condition()
        self._batchCount = 0

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

    def close(self, force, current=None):
        current.con.close(force)

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

    def supportsFunctionalTests(self, current=None):
        return False

class TestIntfControllerI(Test._TestIntfControllerDisp):
    def __init__(self, adapter):
        self._adapter = adapter

    def holdAdapter(self, current=None):
        self._adapter.hold()

    def resumeAdapter(self, current=None):
        self._adapter.activate()
