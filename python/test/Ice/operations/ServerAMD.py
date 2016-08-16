#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback, threading

import Ice
slice_dir = Ice.getSliceDir()
if not slice_dir:
    print(sys.argv[0] + ': Slice directory not found.')
    sys.exit(1)

Ice.loadSlice("'-I" + slice_dir + "' TestAMD.ice")
import Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class Thread_opVoid(threading.Thread):
    def __init__(self, cb):
        threading.Thread.__init__(self)
        self.cb = cb

    def run(self):
        self.cb.ice_response()

class MyDerivedClassI(Test.MyDerivedClass):
    def __init__(self):
        self.opVoidThread = None
        self.opVoidThreadLock = threading.Lock()
        self.lock = threading.Lock()
        self.opByteSOnewayCount = 0

    def ice_isA(self, id, current=None):
        test(current.mode == Ice.OperationMode.Nonmutating)
        return Test.MyDerivedClass.ice_isA(self, id, current)

    def ice_ping(self, current=None):
        test(current.mode == Ice.OperationMode.Nonmutating)
        Test.MyDerivedClass.ice_ping(self, current)

    def ice_ids(self, current=None):
        test(current.mode == Ice.OperationMode.Nonmutating)
        return Test.MyDerivedClass.ice_ids(self, current)

    def ice_id(self, current=None):
        test(current.mode == Ice.OperationMode.Nonmutating)
        return Test.MyDerivedClass.ice_id(self, current)

    def shutdown_async(self, cb, current=None):
        self.opVoidThreadLock.acquire()
        if self.opVoidThread:
            self.opVoidThread.join()
            self.opVoidThread = None
        self.opVoidThreadLock.release()

        current.adapter.getCommunicator().shutdown()
        cb.ice_response()

    def opVoid_async(self, cb, current=None):
        test(current.mode == Ice.OperationMode.Normal)

        self.opVoidThreadLock.acquire()
        if self.opVoidThread:
            self.opVoidThread.join()
            self.opVoidThread = None

        self.opVoidThread = Thread_opVoid(cb)
        self.opVoidThread.start()
        self.opVoidThreadLock.release()

    def opByte_async(self, cb, p1, p2, current=None):
        cb.ice_response(p1, p1 ^ p2)

    def opBool_async(self, cb, p1, p2, current=None):
        cb.ice_response(p2, p1)

    def opShortIntLong_async(self, cb, p1, p2, p3, current=None):
        cb.ice_response(p3, p1, p2, p3)

    def opFloatDouble_async(self, cb, p1, p2, current=None):
        cb.ice_response(p2, p1, p2)

    def opString_async(self, cb, p1, p2, current=None):
        cb.ice_response(p1 + " " + p2, p2 + " " + p1)

    def opMyEnum_async(self, cb, p1, current=None):
        cb.ice_response(Test.MyEnum.enum3, p1)

    def opMyClass_async(self, cb, p1, current=None):
        p2 = p1
        p3 = Test.MyClassPrx.uncheckedCast(current.adapter.createProxy(Ice.stringToIdentity("noSuchIdentity")))
        cb.ice_response(Test.MyClassPrx.uncheckedCast(current.adapter.createProxy(current.id)), p2, p3)

    def opStruct_async(self, cb, p1, p2, current=None):
        p1.s.s = "a new string"
        cb.ice_response(p2, p1)

    def opByteS_async(self, cb, p1, p2, current=None):
        if sys.version_info[0] == 2:
            # By default sequence<byte> maps to a string.
            p3 = map(ord, p1)
            p3.reverse()
            r = map(ord, p1)
            r.extend(map(ord, p2))
        else:
            p3 = bytes(reversed(p1))
            r = p1 + p2
        cb.ice_response(r, p3)

    def opBoolS_async(self, cb, p1, p2, current=None):
        p3 = p1[0:]
        p3.extend(p2)
        r = p1[0:]
        r.reverse();
        cb.ice_response(r, p3)

    def opShortIntLongS_async(self, cb, p1, p2, p3, current=None):
        p4 = p1[0:]
        p5 = p2[0:]
        p5.reverse()
        p6 = p3[0:]
        p6.extend(p3)
        cb.ice_response(p3, p4, p5, p6)

    def opFloatDoubleS_async(self, cb, p1, p2, current=None):
        p3 = p1[0:]
        p4 = p2[0:]
        p4.reverse()
        r = p2[0:]
        r.extend(p1)
        cb.ice_response(r, p3, p4)

    def opStringS_async(self, cb, p1, p2, current=None):
        p3 = p1[0:]
        p3.extend(p2)
        r = p1[0:]
        r.reverse()
        cb.ice_response(r, p3)

    def opByteSS_async(self, cb, p1, p2, current=None):
        p3 = p1[0:]
        p3.reverse()
        r = p1[0:]
        r.extend(p2)
        cb.ice_response(r, p3)

    def opBoolSS_async(self, cb, p1, p2, current=None):
        p3 = p1[0:]
        p3.extend(p2)
        r = p1[0:]
        r.reverse()
        cb.ice_response(r, p3)

    def opShortIntLongSS_async(self, cb, p1, p2, p3, current=None):
        p4 = p1[0:]
        p5 = p2[0:]
        p5.reverse()
        p6 = p3[0:]
        p6.extend(p3)
        cb.ice_response(p3, p4, p5, p6)

    def opFloatDoubleSS_async(self, cb, p1, p2, current=None):
        p3 = p1[0:]
        p4 = p2[0:]
        p4.reverse()
        r = p2[0:]
        r.extend(p2)
        cb.ice_response(r, p3, p4)

    def opStringSS_async(self, cb, p1, p2, current=None):
        p3 = p1[0:]
        p3.extend(p2)
        r = p2[0:]
        r.reverse()
        cb.ice_response(r, p3)

    def opStringSSS_async(self, cb, p1, p2, current=None):
        p3 = p1[0:]
        p3.extend(p2)
        r = p2[0:]
        r.reverse()
        cb.ice_response(r, p3)

    def opByteBoolD_async(self, cb, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        cb.ice_response(r, p3)

    def opShortIntD_async(self, cb, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        cb.ice_response(r, p3)

    def opLongFloatD_async(self, cb, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        cb.ice_response(r, p3)

    def opStringStringD_async(self, cb, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        cb.ice_response(r, p3)

    def opStringMyEnumD_async(self, cb, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        cb.ice_response(r, p3)

    def opMyEnumStringD_async(self, cb, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        cb.ice_response(r, p3)

    def opMyStructMyEnumD_async(self, cb, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        cb.ice_response(r, p3)

    def opByteBoolDS_async(self, cb, p1, p2, current=None):
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        cb.ice_response(r, p3)

    def opShortIntDS_async(self, cb, p1, p2, current=None):
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        cb.ice_response(r, p3)

    def opLongFloatDS_async(self, cb, p1, p2, current=None):
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        cb.ice_response(r, p3)

    def opStringStringDS_async(self, cb, p1, p2, current=None):
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        cb.ice_response(r, p3)

    def opStringMyEnumDS_async(self, cb, p1, p2, current=None):
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        cb.ice_response(r, p3)

    def opMyEnumStringDS_async(self, cb, p1, p2, current=None):
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        cb.ice_response(r, p3)

    def opMyStructMyEnumDS_async(self, cb, p1, p2, current=None):
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        cb.ice_response(r, p3)

    def opByteByteSD_async(self, cb, p1, p2, current=None):
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        cb.ice_response(r, p3)

    def opBoolBoolSD_async(self, cb, p1, p2, current=None):
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        cb.ice_response(r, p3)

    def opShortShortSD_async(self, cb, p1, p2, current=None):
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        cb.ice_response(r, p3)

    def opIntIntSD_async(self, cb, p1, p2, current=None):
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        cb.ice_response(r, p3)

    def opLongLongSD_async(self, cb, p1, p2, current=None):
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        cb.ice_response(r, p3)

    def opStringFloatSD_async(self, cb, p1, p2, current=None):
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        cb.ice_response(r, p3)

    def opStringDoubleSD_async(self, cb, p1, p2, current=None):
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        cb.ice_response(r, p3)

    def opStringStringSD_async(self, cb, p1, p2, current=None):
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        cb.ice_response(r, p3)

    def opMyEnumMyEnumSD_async(self, cb, p1, p2, current=None):
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        cb.ice_response(r, p3)

    def opIntS_async(self, cb, s, current=None):
        cb.ice_response([-x for x in s])

    def opByteSOneway_async(self, cb, s, current=None):
        self.lock.acquire()
        self.opByteSOnewayCount += 1
        self.lock.release()
        cb.ice_response()

    def opByteSOnewayCallCount_async(self, cb, current=None):
        self.lock.acquire()
        count = self.opByteSOnewayCount
        self.opByteSOnewayCount = 0
        self.lock.release()
        cb.ice_response(count)

    def opDoubleMarshaling_async(self, cb, p1, p2, current=None):
        d = 1278312346.0 / 13.0;
        test(p1 == d)
        for i in p2:
            test(i == d)
        cb.ice_response()

    def opContext_async(self, cb, current=None):
        cb.ice_response(current.ctx)

    def opIdempotent_async(self, cb, current=None):
        test(current.mode == Ice.OperationMode.Idempotent)
        cb.ice_response()

    def opNonmutating_async(self, cb, current=None):
        test(current.mode == Ice.OperationMode.Nonmutating)
        cb.ice_response()

    def opDerived_async(self, cb, current=None):
        cb.ice_response()

    def opByte1_async(self, cb, value, current=None):
        cb.ice_response(value)

    def opShort1_async(self, cb, value, current=None):
        cb.ice_response(value)

    def opInt1_async(self, cb, value, current=None):
        cb.ice_response(value)

    def opLong1_async(self, cb, value, current=None):
        cb.ice_response(value)

    def opFloat1_async(self, cb, value, current=None):
        cb.ice_response(value)

    def opDouble1_async(self, cb, value, current=None):
        cb.ice_response(value)

    def opString1_async(self, cb, value, current=None):
        cb.ice_response(value)

    def opStringS1_async(self, cb, value, current=None):
        cb.ice_response(value)

    def opByteBoolD1_async(self, cb, value, current=None):
        cb.ice_response(value)

    def opStringS2_async(self, cb, value, current=None):
        cb.ice_response(value)

    def opByteBoolD2_async(self, cb, value, current=None):
        cb.ice_response(value)

    def opMyClass1_async(self, cb, value, current=None):
        return cb.ice_response(value)

    def opMyStruct1_async(self, cb, value, current=None):
        return cb.ice_response(value)

    def opStringLiterals_async(self, cb, current=None):
        return cb.ice_response([
                Test.s0, Test.s1, Test.s2, Test.s3, Test.s4, Test.s5, Test.s6, Test.s7, Test.s8, Test.s9, Test.s10,
                Test.sw0, Test.sw1, Test.sw2, Test.sw3, Test.sw4, Test.sw5, Test.sw6, Test.sw7, Test.sw8, Test.sw9, Test.sw10,
                Test.ss0, Test.ss1, Test.ss2, Test.ss3, Test.ss4, Test.ss5,
                Test.su0, Test.su1, Test.su2])

    def opWStringLiterals_async(self, cb, current=None):
        return self.opStringLiterals_async(cb, current)

    def opMStruct1_async(self, cb, current):
        cb.ice_response(Test.Structure())

    def opMStruct2_async(self, cb, p1, current):
        cb.ice_response(p1, p1)

    def opMSeq1_async(self, cb, current):
        cb.ice_response([])

    def opMSeq2_async(self, cb, p1, current):
        cb.ice_response(p1, p1)

    def opMDict1_async(self, cb, current):
        cb.ice_response({})

    def opMDict2_async(self, cb, p1, current):
        cb.ice_response(p1, p1)

def run(args, communicator):
    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010:udp")
    adapter = communicator.createObjectAdapter("TestAdapter")
    adapter.add(MyDerivedClassI(), Ice.stringToIdentity("test"))
    adapter.activate()
    communicator.waitForShutdown()
    return True

try:
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)
    initData.properties.setProperty("Ice.Warn.Dispatch", "0");
    communicator = Ice.initialize(sys.argv, initData)
    status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

if communicator:
    try:
        communicator.destroy()
    except:
        traceback.print_exc()
        status = False

sys.exit(not status)
