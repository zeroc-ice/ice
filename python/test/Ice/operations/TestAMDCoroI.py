#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback, threading, time, concurrent.futures
import Ice
slice_dir = Ice.getSliceDir()
if not slice_dir:
    print(sys.argv[0] + ': Slice directory not found.')
    sys.exit(1)

Ice.loadSlice("'-I" + slice_dir + "' Test.ice")
import Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class FutureThread(threading.Thread):
    def __init__(self, f, r):
        threading.Thread.__init__(self)
        self.future = f
        self.result = r

    def run(self):
        time.sleep(0.05)
        self.future.set_result(self.result)

class MyDerivedClassI(Test.MyDerivedClass):
    def __init__(self):
        self.threads = []
        self.threadLock = threading.Lock()
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

    def shutdown(self, current=None):
        with self.threadLock:
            for thread in self.threads:
                thread.join()
            self.threads = []

        current.adapter.getCommunicator().shutdown()

    def supportsCompress(self, current=None):
        return True

    def opVoid(self, current=None):
        test(current.mode == Ice.OperationMode.Normal)

        f = Ice.Future()

        with self.threadLock:
            thread = FutureThread(f, None)
            self.threads.append(thread)
            thread.start()

        return f

    def opByte(self, p1, p2, current=None):
        # Test the ability to use another Future type
        f = concurrent.futures.Future()
        with self.threadLock:
            thread = FutureThread(f, (p1, p1 ^ p2))
            self.threads.append(thread)
            thread.start()
        return f

    def opBool(self, p1, p2, current=None):
        return Ice.Future.completed((p2, p1))

    # Test the ability to define a servant method as a coroutine
    async def opShortIntLong(self, p1, p2, p3, current=None):
        f = Ice.Future()

        with self.threadLock:
            thread = FutureThread(f, (p3, p1, p2, p3))
            self.threads.append(thread)
            thread.start()

        return await f

    def opFloatDouble(self, p1, p2, current=None):
        return Ice.Future.completed((p2, p1, p2))

    def opString(self, p1, p2, current=None):
        return Ice.Future.completed((p1 + " " + p2, p2 + " " + p1))

    def opMyEnum(self, p1, current=None):
        return Ice.Future.completed((Test.MyEnum.enum3, p1))

    def opMyClass(self, p1, current=None):
        p2 = p1
        p3 = Test.MyClassPrx.uncheckedCast(current.adapter.createProxy(Ice.stringToIdentity("noSuchIdentity")))
        return Ice.Future.completed((Test.MyClassPrx.uncheckedCast(current.adapter.createProxy(current.id)), p2, p3))

    def opStruct(self, p1, p2, current=None):
        p1.s.s = "a new string"
        return Ice.Future.completed((p2, p1))

    def opByteS(self, p1, p2, current=None):
        if sys.version_info[0] == 2:
            # By default sequence<byte> maps to a string.
            p3 = map(ord, p1)
            p3.reverse()
            r = map(ord, p1)
            r.extend(map(ord, p2))
        else:
            p3 = bytes(reversed(p1))
            r = p1 + p2
        return Ice.Future.completed((r, p3))

    def opBoolS(self, p1, p2, current=None):
        p3 = p1[0:]
        p3.extend(p2)
        r = p1[0:]
        r.reverse();
        return Ice.Future.completed((r, p3))

    def opShortIntLongS(self, p1, p2, p3, current=None):
        p4 = p1[0:]
        p5 = p2[0:]
        p5.reverse()
        p6 = p3[0:]
        p6.extend(p3)
        return Ice.Future.completed((p3, p4, p5, p6))

    def opFloatDoubleS(self, p1, p2, current=None):
        p3 = p1[0:]
        p4 = p2[0:]
        p4.reverse()
        r = p2[0:]
        r.extend(p1)
        return Ice.Future.completed((r, p3, p4))

    def opStringS(self, p1, p2, current=None):
        p3 = p1[0:]
        p3.extend(p2)
        r = p1[0:]
        r.reverse()
        return Ice.Future.completed((r, p3))

    def opByteSS(self, p1, p2, current=None):
        p3 = p1[0:]
        p3.reverse()
        r = p1[0:]
        r.extend(p2)
        return Ice.Future.completed((r, p3))

    def opBoolSS(self, p1, p2, current=None):
        p3 = p1[0:]
        p3.extend(p2)
        r = p1[0:]
        r.reverse()
        return Ice.Future.completed((r, p3))

    def opShortIntLongSS(self, p1, p2, p3, current=None):
        p4 = p1[0:]
        p5 = p2[0:]
        p5.reverse()
        p6 = p3[0:]
        p6.extend(p3)
        return Ice.Future.completed((p3, p4, p5, p6))

    def opFloatDoubleSS(self, p1, p2, current=None):
        p3 = p1[0:]
        p4 = p2[0:]
        p4.reverse()
        r = p2[0:]
        r.extend(p2)
        return Ice.Future.completed((r, p3, p4))

    def opStringSS(self, p1, p2, current=None):
        p3 = p1[0:]
        p3.extend(p2)
        r = p2[0:]
        r.reverse()
        return Ice.Future.completed((r, p3))

    def opStringSSS(self, p1, p2, current=None):
        p3 = p1[0:]
        p3.extend(p2)
        r = p2[0:]
        r.reverse()
        return Ice.Future.completed((r, p3))

    def opByteBoolD(self, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    def opShortIntD(self, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    def opLongFloatD(self, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    def opStringStringD(self, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    def opStringMyEnumD(self, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    def opMyEnumStringD(self, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    def opMyStructMyEnumD(self, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    def opByteBoolDS(self, p1, p2, current=None):
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        return Ice.Future.completed((r, p3))

    def opShortIntDS(self, p1, p2, current=None):
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        return Ice.Future.completed((r, p3))

    def opLongFloatDS(self, p1, p2, current=None):
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        return Ice.Future.completed((r, p3))

    def opStringStringDS(self, p1, p2, current=None):
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        return Ice.Future.completed((r, p3))

    def opStringMyEnumDS(self, p1, p2, current=None):
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        return Ice.Future.completed((r, p3))

    def opMyEnumStringDS(self, p1, p2, current=None):
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        return Ice.Future.completed((r, p3))

    def opMyStructMyEnumDS(self, p1, p2, current=None):
        p3 = p2[0:]
        p3.extend(p1)
        r = p1[::-1]
        return Ice.Future.completed((r, p3))

    def opByteByteSD(self, p1, p2, current=None):
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    def opBoolBoolSD(self, p1, p2, current=None):
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    def opShortShortSD(self, p1, p2, current=None):
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    def opIntIntSD(self, p1, p2, current=None):
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    def opLongLongSD(self, p1, p2, current=None):
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    def opStringFloatSD(self, p1, p2, current=None):
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    def opStringDoubleSD(self, p1, p2, current=None):
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    def opStringStringSD(self, p1, p2, current=None):
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    def opMyEnumMyEnumSD(self, p1, p2, current=None):
        p3 = p2.copy()
        r = p1.copy()
        r.update(p2)
        return Ice.Future.completed((r, p3))

    def opIntS(self, s, current=None):
        return Ice.Future.completed([-x for x in s])

    def opByteSOneway(self, s, current=None):
        with self.lock:
            self.opByteSOnewayCount += 1
        return Ice.Future.completed(None)

    def opByteSOnewayCallCount(self, current=None):
        with self.lock:
            count = self.opByteSOnewayCount
            self.opByteSOnewayCount = 0
        return Ice.Future.completed(count)

    def opDoubleMarshaling(self, p1, p2, current=None):
        d = 1278312346.0 / 13.0;
        test(p1 == d)
        for i in p2:
            test(i == d)
        return Ice.Future.completed(None)

    def opContext(self, current=None):
        return Ice.Future.completed(current.ctx)

    def opIdempotent(self, current=None):
        test(current.mode == Ice.OperationMode.Idempotent)
        return Ice.Future.completed(None)

    def opNonmutating(self, current=None):
        test(current.mode == Ice.OperationMode.Nonmutating)
        return Ice.Future.completed(None)

    def opDerived(self, current=None):
        return Ice.Future.completed(None)

    def opByte1(self, value, current=None):
        return Ice.Future.completed(value)

    def opShort1(self, value, current=None):
        return Ice.Future.completed(value)

    def opInt1(self, value, current=None):
        return Ice.Future.completed(value)

    def opLong1(self, value, current=None):
        return Ice.Future.completed(value)

    def opFloat1(self, value, current=None):
        return Ice.Future.completed(value)

    def opDouble1(self, value, current=None):
        return Ice.Future.completed(value)

    def opString1(self, value, current=None):
        return Ice.Future.completed(value)

    def opStringS1(self, value, current=None):
        return Ice.Future.completed(value)

    def opByteBoolD1(self, value, current=None):
        return Ice.Future.completed(value)

    def opStringS2(self, value, current=None):
        return Ice.Future.completed(value)

    def opByteBoolD2(self, value, current=None):
        return Ice.Future.completed(value)

    def opMyClass1(self, value, current=None):
        return Ice.Future.completed(value)

    def opMyStruct1(self, value, current=None):
        return Ice.Future.completed(value)

    def opStringLiterals(self, current=None):
        return Ice.Future.completed([
                Test.s0, Test.s1, Test.s2, Test.s3, Test.s4, Test.s5, Test.s6, Test.s7, Test.s8, Test.s9, Test.s10,
                Test.sw0, Test.sw1, Test.sw2, Test.sw3, Test.sw4, Test.sw5, Test.sw6, Test.sw7, Test.sw8, Test.sw9,
                Test.sw10,
                Test.ss0, Test.ss1, Test.ss2, Test.ss3, Test.ss4, Test.ss5,
                Test.su0, Test.su1, Test.su2])

    def opWStringLiterals(self, current=None):
        return self.opStringLiterals(current)

    def opMStruct1(self, current):
        return Ice.Future.completed(Test.MyClass.OpMStruct1MarshaledResult(Test.Structure(), current))

    def opMStruct2(self, p1, current):
        return Ice.Future.completed(Test.MyClass.OpMStruct2MarshaledResult((p1, p1), current))

    def opMSeq1(self, current):
        return Ice.Future.completed(Test.MyClass.OpMSeq1MarshaledResult([], current))

    def opMSeq2(self, p1, current):
        return Ice.Future.completed(Test.MyClass.OpMSeq2MarshaledResult((p1, p1), current))

    def opMDict1(self, current):
        return Ice.Future.completed(Test.MyClass.OpMDict1MarshaledResult({}, current))

    def opMDict2(self, p1, current):
        return Ice.Future.completed(Test.MyClass.OpMDict2MarshaledResult((p1, p1), current))
