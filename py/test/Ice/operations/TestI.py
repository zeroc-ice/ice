#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test

class MyDerivedClassI(Test.MyDerivedClass):
    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

    def opVoid(self, current=None):
        pass

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
        # By default sequence<byte> maps to a string.
        p3 = map(ord, p1)
        p3.reverse()
        r = map(ord, p1)
        r.extend(map(ord, p2))
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

    def opIntS(self, s, current=None):
        return [-x for x in s]

    def opByteSOneway(self, s, current=None):
        pass

    def opContext(self, current=None):
        return current.ctx

    def opDerived(self, current=None):
        pass
