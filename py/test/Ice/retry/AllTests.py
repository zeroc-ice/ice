#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, threading

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
                self._cond.wait(5.0)
            if self._called:
                self._called = False
                return True
            else:
                return False
        finally:
            self._cond.release()

    def called(self):
        self._cond.acquire()
        self._called = True
        self._cond.notify()
        self._cond.release()

class AMIRegular(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self):
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMIException(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        test(isinstance(ex, Ice.ConnectionLostException))
        self.called()

def allTests(communicator):
    print "testing stringToProxy...",
    ref = "retry:default -p 12010 -t 10000"
    base1 = communicator.stringToProxy(ref)
    test(base1)
    base2 = communicator.stringToProxy(ref)
    test(base2)
    print "ok"

    print "testing checked cast...",
    retry1 = Test.RetryPrx.checkedCast(base1)
    test(retry1)
    test(retry1 == base1)
    retry2 = Test.RetryPrx.checkedCast(base2)
    test(retry2)
    test(retry2 == base2)
    print "ok"

    print "calling regular operation with first proxy...",
    retry1.op(False)
    print "ok"

    print "calling operation to kill connection with second proxy...",
    try:
        retry2.op(True)
        test(False)
    except Ice.ConnectionLostException:
        print "ok"

    print "calling regular operation with first proxy again...",
    retry1.op(False)
    print "ok"

    cb1 = AMIRegular()
    cb2 = AMIException()

    print "calling regular AMI operation with first proxy...",
    retry1.op_async(cb1, False)
    test(cb1.check())
    print "ok"

    print "calling AMI operation to kill connection with second proxy...",
    retry2.op_async(cb2, True)
    test(cb2.check())
    print "ok"

    print "calling regular AMI operation with first proxy again...",
    retry1.op_async(cb1, False)
    test(cb1.check())
    print "ok"

    return retry1
    return retry1
