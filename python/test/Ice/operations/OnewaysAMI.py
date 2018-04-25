# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
        with self._cond:
            while not self._called:
                self._cond.wait()
            self._called = False

    def called(self):
        with self._cond:
            self._called = True
            self._cond.notify()

class Callback(CallbackBase):
    def sent(self, sentSynchronously):
        self.called()

    def noException(self, ex):
        test(False)

def onewaysAMI(communicator, proxy):

    p = Test.MyClassPrx.uncheckedCast(proxy.ice_oneway())

    cb = Callback()
    p.begin_ice_ping(None, cb.noException, cb.sent)
    cb.check()

    try:
        p.begin_ice_isA(Test.MyClass.ice_staticId())
        test(False)
    except RuntimeError:
        pass

    try:
        p.begin_ice_id()
        test(False)
    except RuntimeError:
        pass

    try:
        p.begin_ice_ids()
        test(False)
    except RuntimeError:
        pass

    cb = Callback()
    p.begin_opVoid(None, cb.noException, cb.sent)
    cb.check()

    cb = Callback()
    p.begin_opIdempotent(None, cb.noException, cb.sent)
    cb.check()

    cb = Callback()
    p.begin_opNonmutating(None, cb.noException, cb.sent)
    cb.check()

    try:
        p.begin_opByte(0xff, 0x0f)
        test(False)
    except RuntimeError:
        pass
