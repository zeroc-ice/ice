# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, threading

class TestIntfI(Test.TestIntf):
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
        self._cond.acquire()
        try:
            self._batchCount += 1
            self._cond.notify()
        finally:
            self._cond.release()

    def opBatchCount(self, current=None):
        self._cond.acquire()
        try:
            return self._batchCount
        finally:
            self._cond.release()

    def waitForBatch(self, count, current=None):
        self._cond.acquire()
        try:
            while self._batchCount < count:
                self._cond.wait(5)
            result = count == self._batchCount
            self._batchCount = 0
            return result
        finally:
            self._cond.release()

    def close(self, force, current=None):
        current.con.close(force)

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

    def supportsFunctionalTests(self, current=None):
        return False

class TestIntfControllerI(Test.TestIntfController):
    def __init__(self, adapter):
        self._adapter = adapter

    def holdAdapter(self, current=None):
        self._adapter.hold()

    def resumeAdapter(self, current=None):
        self._adapter.activate()
