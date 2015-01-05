# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice Protobuf is licensed to you under the terms
# described in the ICE_PROTOBUF_LICENSE file included in this
# distribution.
#
# **********************************************************************

import Ice, Test, threading
from Test_pb2 import Message

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class CallbackBase:
    def __init__(self):
        self._called = False
        self._cond = threading.Condition()

    def called(self):
        self._cond.acquire()
        self._called = True
        self._cond.notify()
        self._cond.release()

    def check(self):
        self._cond.acquire()
        while not self._called:
            self._cond.wait()
        self._called = False

class AMI_MyClass_opMessageI(CallbackBase):
    def ice_response(self, r, o):
        test(o.i == 99);
        test(r.i == 99);
        self.called()

    def ice_exception(self, ex):
        test(False)

def allTests(communicator, collocated):
    sref = "test:default -p 12010"
    obj = communicator.stringToProxy(sref)
    test(obj != None)

    cl = Test.MyClassPrx.checkedCast(obj)
    test(cl != None)

    print "testing twoway operations... ",
    i = Message()
    i.i = 99

    r, o = cl.opMessage(i);
    test(o.i == 99);
    test(r.i == 99);

    r, o = cl.opMessageAMD(i);
    test(o.i == 99);
    test(r.i == 99);
    print "ok"

    print "testing twoway AMI operations... ",
    cb = AMI_MyClass_opMessageI()
    cl.opMessage_async(cb, i)
    cb.check()

    cb = AMI_MyClass_opMessageI()
    cl.opMessageAMD_async(cb, i)
    cb.check()
    print "ok"

    return cl
