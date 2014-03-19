# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, sys, threading

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class CallbackBase:
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

class CallbackSuccess(CallbackBase):
    def response(self):
        self.called()

    def exception(self, ex):
        test(False)

class CallbackFail(CallbackBase):
    def response(self):
        test(False)

    def exception(self, ex):
        test(isinstance(ex, Ice.ConnectionLostException))
        self.called()

def allTests(communicator):
    sys.stdout.write("testing stringToProxy... ")
    sys.stdout.flush()
    ref = "retry:default -p 12010"
    base1 = communicator.stringToProxy(ref)
    test(base1)
    base2 = communicator.stringToProxy(ref)
    test(base2)
    print("ok")

    sys.stdout.write("testing checked cast... ")
    sys.stdout.flush()
    retry1 = Test.RetryPrx.checkedCast(base1)
    test(retry1)
    test(retry1 == base1)
    retry2 = Test.RetryPrx.checkedCast(base2)
    test(retry2)
    test(retry2 == base2)
    print("ok")

    sys.stdout.write("calling regular operation with first proxy... ")
    sys.stdout.flush()
    retry1.op(False)
    print("ok")

    sys.stdout.write("calling operation to kill connection with second proxy... ")
    sys.stdout.flush()
    try:
        retry2.op(True)
        test(False)
    except Ice.ConnectionLostException:
        print("ok")

    sys.stdout.write("calling regular operation with first proxy again... ")
    sys.stdout.flush()
    retry1.op(False)
    print("ok")

    cb1 = CallbackSuccess()
    cb2 = CallbackFail()

    sys.stdout.write("calling regular AMI operation with first proxy... ")
    sys.stdout.flush()
    retry1.begin_op(False, cb1.response, cb1.exception)
    cb1.check()
    print("ok")

    sys.stdout.write("calling AMI operation to kill connection with second proxy... ")
    sys.stdout.flush()
    retry2.begin_op(True, cb2.response, cb2.exception)
    cb2.check()
    print("ok")

    sys.stdout.write("calling regular AMI operation with first proxy again... ")
    sys.stdout.flush()
    retry1.begin_op(False, cb1.response, cb1.exception)
    cb1.check()
    print("ok")

    return retry1
