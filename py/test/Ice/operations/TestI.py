# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, sys

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class MyDerivedClassI(Test.MyDerivedClass):
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
        current.adapter.getCommunicator().shutdown()

    def opVoid(self, current=None):
        test(current.mode == Ice.OperationMode.Normal)

    def opByte(self, p1, p2, current=None):
        return (p1, p1 ^ p2)

    def opBool(self, p1, p2, current=None):
        return (p2, p1)

    def opShortIntLong(self, p1, p2, p3, current=None):
        return (p3, p1, p2, p3)

    def opFloatDouble(self, p1, p2, current=None):
        return (p2, p1, p2)

    def opString(self, p1, p2, current=None):
        return (p1 + " " + p2, p2 + " " + p1)

    def opMyEnum(self, p1, current=None):
        return (Test.MyEnum.enum3, p1)

    def opMyClass(self, p1, current=None):
        return (Test.MyClassPrx.uncheckedCast(current.adapter.createProxy(current.id)), p1,
                Test.MyClassPrx.uncheckedCast(current.adapter.createProxy(current.adapter.getCommunicator().stringToIdentity("noSuchIdentity"))))

    def opStruct(self, p1, p2, current=None):
        p1.s.s = "a new string"
        return (p2, p1)

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
        return (r, p3)

    def opBoolS(self, p1, p2, current=None):
        p3 = p1[0:]
        p3.extend(p2)
        r = p1[0:]
        r.reverse();
        return (r, p3)

    def opShortIntLongS(self, p1, p2, p3, current=None):
        p4 = p1[0:]
        p5 = p2[0:]
        p5.reverse()
        p6 = p3[0:]
        p6.extend(p3)
        return (p3, p4, p5, p6)

    def opFloatDoubleS(self, p1, p2, current=None):
        p3 = p1[0:]
        p4 = p2[0:]
        p4.reverse()
        r = p2[0:]
        r.extend(p1)
        return (r, p3, p4)

    def opStringS(self, p1, p2, current=None):
        p3 = p1[0:]
        p3.extend(p2)
        r = p1[0:]
        r.reverse()
        return (r, p3)

    def opByteSS(self, p1, p2, current=None):
        p3 = p1[0:]
        p3.reverse()
        r = p1[0:]
        r.extend(p2)
        return (r, p3)

    def opBoolSS(self, p1, p2, current=None):
        p3 = p1[0:]
        p3.extend(p2)
        r = p1[0:]
        r.reverse()
        return (r, p3)

    def opShortIntLongSS(self, p1, p2, p3, current=None):
        p4 = p1[0:]
        p5 = p2[0:]
        p5.reverse()
        p6 = p3[0:]
        p6.extend(p3)
        return (p3, p4, p5, p6)

    def opFloatDoubleSS(self, p1, p2, current=None):
        p3 = p1[0:]
        p4 = p2[0:]
        p4.reverse()
        r = p2[0:]
        r.extend(p2)
        return (r, p3, p4)

    def opStringSS(self, p1, p2, current=None):
        p3 = p1[0:]
        p3.extend(p2)
        r = p2[0:]
        r.reverse()
        return (r, p3)

    def opStringSSS(self, p1, p2, current=None):
        p3 = p1[0:]
        p3.extend(p2)
        r = p2[0:]
        r.reverse()
        return (r, p3)

    def opByteBoolD(self, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    def opShortIntD(self, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    def opLongFloatD(self, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    def opStringStringD(self, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    def opStringMyEnumD(self, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    def opMyEnumStringD(self, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    def opMyStructMyEnumD(self, p1, p2, current=None):
        p3 = p1.copy()
        r = p1.copy()
        r.update(p2)
        return (r, p3)

    def opIntS(self, s, current=None):
        return [-x for x in s]

    def opByteSOneway(self, s, current=None):
        pass

    def opContext(self, current=None):
        return current.ctx

    def opDoubleMarshaling(self, p1, p2, current=None):
        d = 1278312346.0 / 13.0;
        test(p1 == d)
        for i in p2:
            test(i == d)

    def opIdempotent(self, current=None):
        test(current.mode == Ice.OperationMode.Idempotent)

    def opNonmutating(self, current=None):
        test(current.mode == Ice.OperationMode.Nonmutating)

    def opDerived(self, current=None):
        pass
