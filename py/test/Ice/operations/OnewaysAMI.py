# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, math, threading

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

class AMI_MyClass_opVoidI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self):
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opIdempotentI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self):
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opNonmutatingI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self):
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opVoidExI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        test(isinstance(ex, Ice.NoEndpointException))
        self.called()

class AMI_MyClass_opByteExI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, r, b):
        test(False)

    def ice_exception(self, ex):
        test(isinstance(ex, Ice.TwowayOnlyException))
        self.called()

def onewaysAMI(communicator, p):

    p = Test.MyClassPrx.uncheckedCast(p.ice_oneway())

    cb = AMI_MyClass_opVoidI()
    p.opVoid_async(cb)
    # Let's check if we can reuse the same callback object for another call.
    p.opVoid_async(cb)

    cb = AMI_MyClass_opIdempotentI()
    p.opIdempotent_async(cb)

    cb = AMI_MyClass_opNonmutatingI()
    p.opNonmutating_async(cb)

    # Check that a call to a void operation raises NoEndpointException
    # in the ice_exception() callback instead of at the point of call.
    indirect = Test.MyClassPrx.uncheckedCast(p.ice_adapterId("dummy"))
    cb = AMI_MyClass_opVoidExI()
    try:
        indirect.opVoid_async(cb)
    except Ice.Exception:
        test(False)
    cb.check()

    cb = AMI_MyClass_opByteExI()
    try:
        p.opByte_async(cb, 0, 0)
    except Ice.Exception:
        test(False)
    cb.check()
