# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, math, sys, threading

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
    def __init__(self, communicator=None):
        CallbackBase.__init__(self)
        self._communicator = communicator

    def ping(self):
        self.called()

    def isA(self, r):
        test(r)
        self.called()

    def id(self, id):
        test(id == "::Test::MyDerivedClass")
        self.called()

    def ids(self, ids):
        test(len(ids) == 3)
        self.called()

    def opVoid(self):
        self.called()

    def opByte(self, r, b):
        test(b == 0xf0)
        test(r == 0xff)
        self.called()

    def opBool(self, r, b):
        test(b)
        test(not r)
        self.called()

    def opShortIntLong(self, r, s, i, l):
        test(s == 10)
        test(i == 11)
        test(l == 12)
        test(r == 12)
        self.called()

    def opFloatDouble(self, r, f, d):
        test(f - 3.14 < 0.001)
        test(d == 1.1E10)
        test(r == 1.1E10)
        self.called()

    def opString(self, r, s):
        test(s == "world hello")
        test(r == "hello world")
        self.called()

    def opMyEnum(self, r, e):
        test(e == Test.MyEnum.enum2)
        test(r == Test.MyEnum.enum3)
        self.called()

    def opMyClass(self, r, c1, c2):
        test(c1.ice_getIdentity() == Ice.stringToIdentity("test"))
        test(c2.ice_getIdentity() == Ice.stringToIdentity("noSuchIdentity"))
        test(r.ice_getIdentity() == Ice.stringToIdentity("test"))
        # We can't do the callbacks below in serialize mode
        if self._communicator.getProperties().getPropertyAsInt("Ice.Client.ThreadPool.Serialize") == 0:
            r.opVoid()
            c1.opVoid()
            try:
                c2.opVoid()
                test(False)
            except Ice.ObjectNotExistException:
                pass
        self.called()

    def opStruct(self, rso, so):
        test(rso.p == None)
        test(rso.e == Test.MyEnum.enum2)
        test(rso.s.s == "def")
        test(so.e == Test.MyEnum.enum3)
        test(so.s.s == "a new string")
        # We can't do the callbacks below in serialize mode.
        if self._communicator.getProperties().getPropertyAsInt("Ice.ThreadPool.Client.Serialize") == 0:
            so.p.opVoid()
        self.called()

    def opByteS(self, rso, bso):
        test(len(bso) == 4)
        test(len(rso) == 8)
        if sys.version_info[0] == 2:
            test(bso[0] == '\x22')
            test(bso[1] == '\x12')
            test(bso[2] == '\x11')
            test(bso[3] == '\x01')
            test(rso[0] == '\x01')
            test(rso[1] == '\x11')
            test(rso[2] == '\x12')
            test(rso[3] == '\x22')
            test(rso[4] == '\xf1')
            test(rso[5] == '\xf2')
            test(rso[6] == '\xf3')
            test(rso[7] == '\xf4')
        else:
            test(bso[0] == 0x22)
            test(bso[1] == 0x12)
            test(bso[2] == 0x11)
            test(bso[3] == 0x01)
            test(rso[0] == 0x01)
            test(rso[1] == 0x11)
            test(rso[2] == 0x12)
            test(rso[3] == 0x22)
            test(rso[4] == 0xf1)
            test(rso[5] == 0xf2)
            test(rso[6] == 0xf3)
            test(rso[7] == 0xf4)
        self.called()

    def opBoolS(self, rso, bso):
        test(len(bso) == 4)
        test(bso[0])
        test(bso[1])
        test(not bso[2])
        test(not bso[3])
        test(len(rso) == 3)
        test(not rso[0])
        test(rso[1])
        test(rso[2])
        self.called()

    def opShortIntLongS(self, rso, sso, iso, lso):
        test(len(sso) == 3)
        test(sso[0] == 1)
        test(sso[1] == 2)
        test(sso[2] == 3)
        test(len(iso) == 4)
        test(iso[0] == 8)
        test(iso[1] == 7)
        test(iso[2] == 6)
        test(iso[3] == 5)
        test(len(lso) == 6)
        test(lso[0] == 10)
        test(lso[1] == 30)
        test(lso[2] == 20)
        test(lso[3] == 10)
        test(lso[4] == 30)
        test(lso[5] == 20)
        test(len(rso) == 3)
        test(rso[0] == 10)
        test(rso[1] == 30)
        test(rso[2] == 20)
        self.called()

    def opFloatDoubleS(self, rso, fso, dso):
        test(len(fso) == 2)
        test(fso[0] - 3.14 < 0.001)
        test(fso[1] - 1.11 < 0.001)
        test(len(dso) == 3)
        test(dso[0] == 1.3E10)
        test(dso[1] == 1.2E10)
        test(dso[2] == 1.1E10)
        test(len(rso) == 5)
        test(rso[0] == 1.1E10)
        test(rso[1] == 1.2E10)
        test(rso[2] == 1.3E10)
        test(rso[3] - 3.14 < 0.001)
        test(rso[4] - 1.11 < 0.001)
        self.called()

    def opStringS(self, rso, sso):
        test(len(sso) == 4)
        test(sso[0] == "abc")
        test(sso[1] == "de")
        test(sso[2] == "fghi")
        test(sso[3] == "xyz")
        test(len(rso) == 3)
        test(rso[0] == "fghi")
        test(rso[1] == "de")
        test(rso[2] == "abc")
        self.called()

    def opByteSS(self, rso, bso):
        test(len(bso) == 2)
        test(len(bso[0]) == 1)
        test(len(bso[1]) == 3)
        test(len(rso) == 4)
        test(len(rso[0]) == 3)
        test(len(rso[1]) == 1)
        test(len(rso[2]) == 1)
        test(len(rso[3]) == 2)
        if sys.version_info[0] == 2:
            test(bso[0][0] == '\xff')
            test(bso[1][0] == '\x01')
            test(bso[1][1] == '\x11')
            test(bso[1][2] == '\x12')
            test(rso[0][0] == '\x01')
            test(rso[0][1] == '\x11')
            test(rso[0][2] == '\x12')
            test(rso[1][0] == '\xff')
            test(rso[2][0] == '\x0e')
            test(rso[3][0] == '\xf2')
            test(rso[3][1] == '\xf1')
        else:
            test(bso[0][0] == 0xff)
            test(bso[1][0] == 0x01)
            test(bso[1][1] == 0x11)
            test(bso[1][2] == 0x12)
            test(rso[0][0] == 0x01)
            test(rso[0][1] == 0x11)
            test(rso[0][2] == 0x12)
            test(rso[1][0] == 0xff)
            test(rso[2][0] == 0x0e)
            test(rso[3][0] == 0xf2)
            test(rso[3][1] == 0xf1)
        self.called()

    def opBoolSS(self, rso, bso):
        test(len(bso) == 4);
        test(len(bso[0]) == 1);
        test(bso[0][0]);
        test(len(bso[1]) == 1);
        test(not bso[1][0]);
        test(len(bso[2]) == 2);
        test(bso[2][0]);
        test(bso[2][1]);
        test(len(bso[3]) == 3);
        test(not bso[3][0]);
        test(not bso[3][1]);
        test(bso[3][2]);
        test(len(rso) == 3);
        test(len(rso[0]) == 2);
        test(rso[0][0]);
        test(rso[0][1]);
        test(len(rso[1]) == 1);
        test(not rso[1][0]);
        test(len(rso[2]) == 1);
        test(rso[2][0]);
        self.called();

    def opShortIntLongSS(self, rso, sso, iso, lso):
        test(len(rso) == 1);
        test(len(rso[0]) == 2);
        test(rso[0][0] == 496);
        test(rso[0][1] == 1729);
        test(len(sso) == 3);
        test(len(sso[0]) == 3);
        test(sso[0][0] == 1);
        test(sso[0][1] == 2);
        test(sso[0][2] == 5);
        test(len(sso[1]) == 1);
        test(sso[1][0] == 13);
        test(len(sso[2]) == 0);
        test(len(iso) == 2);
        test(len(iso[0]) == 1);
        test(iso[0][0] == 42);
        test(len(iso[1]) == 2);
        test(iso[1][0] == 24);
        test(iso[1][1] == 98);
        test(len(lso) == 2);
        test(len(lso[0]) == 2);
        test(lso[0][0] == 496);
        test(lso[0][1] == 1729);
        test(len(lso[1]) == 2);
        test(lso[1][0] == 496);
        test(lso[1][1] == 1729);
        self.called();

    def opFloatDoubleSS(self, rso, fso, dso):
        test(len(fso) == 3)
        test(len(fso[0]) == 1)
        test(fso[0][0] - 3.14 < 0.001)
        test(len(fso[1]) == 1)
        test(fso[1][0] - 1.11 < 0.001)
        test(len(fso[2]) == 0)
        test(len(dso) == 1)
        test(len(dso[0]) == 3)
        test(dso[0][0] == 1.1E10)
        test(dso[0][1] == 1.2E10)
        test(dso[0][2] == 1.3E10)
        test(len(rso) == 2)
        test(len(rso[0]) == 3)
        test(rso[0][0] == 1.1E10)
        test(rso[0][1] == 1.2E10)
        test(rso[0][2] == 1.3E10)
        test(len(rso[1]) == 3)
        test(rso[1][0] == 1.1E10)
        test(rso[1][1] == 1.2E10)
        test(rso[1][2] == 1.3E10)
        self.called()

    def opStringSS(self, rso, sso):
        test(len(sso) == 5)
        test(len(sso[0]) == 1)
        test(sso[0][0] == "abc")
        test(len(sso[1]) == 2)
        test(sso[1][0] == "de")
        test(sso[1][1] == "fghi")
        test(len(sso[2]) == 0)
        test(len(sso[3]) == 0)
        test(len(sso[4]) == 1)
        test(sso[4][0] == "xyz")
        test(len(rso) == 3)
        test(len(rso[0]) == 1)
        test(rso[0][0] == "xyz")
        test(len(rso[1]) == 0)
        test(len(rso[2]) == 0)
        self.called()

    def opByteBoolD(self, ro, do):
        di1 = {10: True, 100: False}
        test(do == di1)
        test(len(ro) == 4)
        test(ro[10])
        test(not ro[11])
        test(not ro[100])
        test(ro[101])
        self.called()

    def opShortIntD(self, ro, do):
        di1 = {110: -1, 1100: 123123}
        test(do == di1)
        test(len(ro) == 4)
        test(ro[110] == -1)
        test(ro[111] == -100)
        test(ro[1100] == 123123)
        test(ro[1101] == 0)
        self.called()

    def opLongFloatD(self, ro, do):
        di1 = {999999110: -1.1, 999999111: 123123.2}
        for k in do:
            test(math.fabs(do[k] - di1[k]) < 0.01)
        test(len(ro) == 4)
        test(ro[999999110] - -1.1 < 0.01)
        test(ro[999999120] - -100.4 < 0.01)
        test(ro[999999111] - 123123.2 < 0.01)
        test(ro[999999130] - 0.5 < 0.01)
        self.called()

    def opStringStringD(self, ro, do):
        di1 = {'foo': 'abc -1.1', 'bar': 'abc 123123.2'}
        test(do == di1)
        test(len(ro) == 4)
        test(ro["foo"] == "abc -1.1")
        test(ro["FOO"] == "abc -100.4")
        test(ro["bar"] == "abc 123123.2")
        test(ro["BAR"] == "abc 0.5")
        self.called()

    def opStringMyEnumD(self, ro, do):
        di1 = {'abc': Test.MyEnum.enum1, '': Test.MyEnum.enum2}
        test(do == di1)
        test(len(ro) == 4)
        test(ro["abc"] == Test.MyEnum.enum1)
        test(ro["qwerty"] == Test.MyEnum.enum3)
        test(ro[""] == Test.MyEnum.enum2)
        test(ro["Hello!!"] == Test.MyEnum.enum2)
        self.called()

    def opMyEnumStringD(self, ro, do):
        di1 = {Test.MyEnum.enum1: 'abc'}
        test(do == di1)
        test(len(ro) == 3)
        test(ro[Test.MyEnum.enum1] == "abc")
        test(ro[Test.MyEnum.enum2] == "Hello!!")
        test(ro[Test.MyEnum.enum3] == "qwerty")
        self.called()

    def opMyStructMyEnumD(self, ro, do):
        s11 = Test.MyStruct()
        s11.i = 1
        s11.j = 1
        s12 = Test.MyStruct()
        s12.i = 1
        s12.j = 2
        s22 = Test.MyStruct()
        s22.i = 2
        s22.j = 2
        s23 = Test.MyStruct()
        s23.i = 2
        s23.j = 3
        di1 = {s11: Test.MyEnum.enum1, s12: Test.MyEnum.enum2}
        test(do == di1)
        test(len(ro) == 4)
        test(ro[s11] == Test.MyEnum.enum1)
        test(ro[s12] == Test.MyEnum.enum2)
        test(ro[s22] == Test.MyEnum.enum3)
        test(ro[s23] == Test.MyEnum.enum2)
        self.called()

    def opByteBoolDS(self, ro, do):
        test(len(ro) == 2)
        test(len(ro[0]) == 3)
        test(ro[0][10])
        test(not ro[0][11])
        test(ro[0][101])
        test(len(ro[1]) == 2)
        test(ro[1][10])
        test(not ro[1][100])
        test(len(do) == 3)
        test(len(do[0]) == 2)
        test(not do[0][100])
        test(not do[0][101])
        test(len(do[1]) == 2)
        test(do[1][10])
        test(not do[1][100])
        test(len(do[2]) == 3)
        test(do[2][10])
        test(not do[2][11])
        test(do[2][101])
        self.called()

    def opShortIntDS(self, ro, do):
        test(len(ro) == 2)
        test(len(ro[0]) == 3)
        test(ro[0][110] == -1)
        test(ro[0][111] == -100)
        test(ro[0][1101] == 0)
        test(len(ro[1]) == 2)
        test(ro[1][110] == -1)
        test(ro[1][1100] == 123123)
        test(len(do) == 3)
        test(len(do[0]) == 1)
        test(do[0][100] == -1001)
        test(len(do[1]) == 2)
        test(do[1][110] == -1)
        test(do[1][1100] == 123123)
        test(len(do[2]) == 3)
        test(do[2][110] == -1)
        test(do[2][111] == -100)
        test(do[2][1101] == 0)
        self.called()

    def opLongFloatDS(self, ro, do):
        test(len(ro) == 2)
        test(len(ro[0]) == 3)
        test(ro[0][999999110] - -1.1 < 0.01)
        test(ro[0][999999120] - -100.4 < 0.01)
        test(ro[0][999999130] - 0.5 < 0.01)
        test(len(ro[1]) == 2)
        test(ro[1][999999110] - -1.1 < 0.01)
        test(ro[1][999999111] - 123123.2 < 0.01)
        test(len(do) == 3)
        test(len(do[0]) == 1)
        test(do[0][999999140] - 3.14 < 0.01)
        test(len(do[1]) == 2)
        test(do[1][999999110] - -1.1 < 0.01)
        test(do[1][999999111] - 123123.2 < 0.01)
        test(len(do[2]) == 3)
        test(do[2][999999110] - -1.1 < 0.01)
        test(do[2][999999120] - -100.4 < 0.01)
        test(do[2][999999130] - 0.5 < 0.01)
        self.called()

    def opStringStringDS(self, ro, do):
        test(len(ro) == 2)
        test(len(ro[0]) == 3)
        test(ro[0]["foo"] == "abc -1.1")
        test(ro[0]["FOO"] == "abc -100.4")
        test(ro[0]["BAR"] == "abc 0.5")
        test(len(ro[1]) == 2)
        test(ro[1]["foo"] == "abc -1.1")
        test(ro[1]["bar"] == "abc 123123.2")
        test(len(do) == 3)
        test(len(do[0]) == 1)
        test(do[0]["f00"] == "ABC -3.14")
        test(len(do[1]) == 2)
        test(do[1]["foo"] == "abc -1.1")
        test(do[1]["bar"] == "abc 123123.2")
        test(len(do[2]) == 3)
        test(do[2]["foo"] == "abc -1.1")
        test(do[2]["FOO"] == "abc -100.4")
        test(do[2]["BAR"] == "abc 0.5")
        self.called()

    def opStringMyEnumDS(self, ro, do):
        test(len(ro) == 2)
        test(len(ro[0]) == 3)
        test(ro[0]["abc"] == Test.MyEnum.enum1)
        test(ro[0]["qwerty"] == Test.MyEnum.enum3)
        test(ro[0]["Hello!!"] == Test.MyEnum.enum2)
        test(len(ro[1]) == 2)
        test(ro[1]["abc"] == Test.MyEnum.enum1)
        test(ro[1][""] == Test.MyEnum.enum2)
        test(len(do) == 3)
        test(len(do[0]) == 1)
        test(do[0]["Goodbye"] == Test.MyEnum.enum1)
        test(len(do[1]) == 2)
        test(do[1]["abc"] == Test.MyEnum.enum1)
        test(do[1][""] == Test.MyEnum.enum2)
        test(len(do[2]) == 3)
        test(do[2]["abc"] == Test.MyEnum.enum1)
        test(do[2]["qwerty"] == Test.MyEnum.enum3)
        test(do[2]["Hello!!"] == Test.MyEnum.enum2)
        self.called()

    def opMyEnumStringDS(self, ro, do):
        test(len(ro) == 2)
        test(len(ro[0]) == 2)
        test(ro[0][Test.MyEnum.enum2] == "Hello!!")
        test(ro[0][Test.MyEnum.enum3] == "qwerty")
        test(len(ro[1]) == 1)
        test(ro[1][Test.MyEnum.enum1] == "abc")
        test(len(do) == 3)
        test(len(do[0]) == 1)
        test(do[0][Test.MyEnum.enum1] == "Goodbye")
        test(len(do[1]) == 1)
        test(do[1][Test.MyEnum.enum1] == "abc")
        test(len(do[2]) == 2)
        test(do[2][Test.MyEnum.enum2] == "Hello!!")
        test(do[2][Test.MyEnum.enum3] == "qwerty")
        self.called()

    def opMyStructMyEnumDS(self, ro, do):
        s11 = Test.MyStruct(1, 1)
        s12 = Test.MyStruct(1, 2)
        s22 = Test.MyStruct(2, 2)
        s23 = Test.MyStruct(2, 3)
        test(len(ro) == 2)
        test(len(ro[0]) == 3)
        test(ro[0][s11] == Test.MyEnum.enum1)
        test(ro[0][s22] == Test.MyEnum.enum3)
        test(ro[0][s23] == Test.MyEnum.enum2)
        test(len(ro[1]) == 2)
        test(ro[1][s11] == Test.MyEnum.enum1)
        test(ro[1][s12] == Test.MyEnum.enum2)
        test(len(do) == 3)
        test(len(do[0]) == 1)
        test(do[0][s23] == Test.MyEnum.enum3)
        test(len(do[1]) == 2)
        test(do[1][s11] == Test.MyEnum.enum1)
        test(do[1][s12] == Test.MyEnum.enum2)
        test(len(do[2]) == 3)
        test(do[2][s11] == Test.MyEnum.enum1)
        test(do[2][s22] == Test.MyEnum.enum3)
        test(do[2][s23] == Test.MyEnum.enum2)
        self.called()

    def opByteByteSD(self, ro, do):
        if sys.version_info[0] == 2:
            test(len(do) == 1)
            test(len(do[0xf1]) == 2)
            test(do[0xf1][0] == '\xf2')
            test(do[0xf1][1] == '\xf3')
            test(len(ro) == 3)
            test(len(ro[0x01]) == 2)
            test(ro[0x01][0] == '\x01')
            test(ro[0x01][1] == '\x11')
            test(len(ro[0x22]) == 1)
            test(ro[0x22][0] == '\x12')
            test(len(ro[0xf1]) == 2)
            test(ro[0xf1][0] == '\xf2')
            test(ro[0xf1][1] == '\xf3')
        else:
            test(len(do) == 1)
            test(len(do[0xf1]) == 2)
            test(do[0xf1][0] == 0xf2)
            test(do[0xf1][1] == 0xf3)
            test(len(ro) == 3)
            test(len(ro[0x01]) == 2)
            test(ro[0x01][0] == 0x01)
            test(ro[0x01][1] == 0x11)
            test(len(ro[0x22]) == 1)
            test(ro[0x22][0] == 0x12)
            test(len(ro[0xf1]) == 2)
            test(ro[0xf1][0] == 0xf2)
            test(ro[0xf1][1] == 0xf3)
        self.called()

    def opBoolBoolSD(self, ro, do):
        test(len(do) == 1)
        test(len(do[False]) == 2)
        test(do[False][0])
        test(not do[False][1])
        test(len(ro) == 2)
        test(len(ro[False]) == 2)
        test(ro[False][0])
        test(not ro[False][1])
        test(len(ro[True]) == 3)
        test(not ro[True][0])
        test(ro[True][1])
        test(ro[True][2])
        self.called()

    def opShortShortSD(self, ro, do):
        test(len(do) == 1)
        test(len(do[4]) == 2)
        test(do[4][0] == 6)
        test(do[4][1] == 7)
        test(len(ro) == 3)
        test(len(ro[1]) == 3)
        test(ro[1][0] == 1)
        test(ro[1][1] == 2)
        test(ro[1][2] == 3)
        test(len(ro[2]) == 2)
        test(ro[2][0] == 4)
        test(ro[2][1] == 5)
        test(len(ro[4]) == 2)
        test(ro[4][0] == 6)
        test(ro[4][1] == 7)
        self.called()

    def opIntIntSD(self, ro, do):
        test(len(do) == 1)
        test(len(do[400]) == 2)
        test(do[400][0] == 600)
        test(do[400][1] == 700)
        test(len(ro) == 3)
        test(len(ro[100]) == 3)
        test(ro[100][0] == 100)
        test(ro[100][1] == 200)
        test(ro[100][2] == 300)
        test(len(ro[200]) == 2)
        test(ro[200][0] == 400)
        test(ro[200][1] == 500)
        test(len(ro[400]) == 2)
        test(ro[400][0] == 600)
        test(ro[400][1] == 700)
        self.called()

    def opLongLongSD(self, ro, do):
        test(len(do) == 1)
        test(len(do[999999992]) == 2)
        test(do[999999992][0] == 999999110)
        test(do[999999992][1] == 999999120)
        test(len(ro) == 3)
        test(len(ro[999999990]) == 3)
        test(ro[999999990][0] == 999999110)
        test(ro[999999990][1] == 999999111)
        test(ro[999999990][2] == 999999110)
        test(len(ro[999999991]) == 2)
        test(ro[999999991][0] == 999999120)
        test(ro[999999991][1] == 999999130)
        test(len(ro[999999992]) == 2)
        test(ro[999999992][0] == 999999110)
        test(ro[999999992][1] == 999999120)
        self.called()

    def opStringFloatSD(self, ro, do):
        test(len(do) == 1)
        test(len(do["aBc"]) == 2)
        test(do["aBc"][0] - -3.14 < 0.10)
        test(do["aBc"][1] - 3.14 < 0.10)
        test(len(ro) == 3)
        test(len(ro["abc"]) == 3)
        test(ro["abc"][0] - -1.1 < 0.10)
        test(ro["abc"][1] - 123123.2 < 0.10)
        test(ro["abc"][2] - 100.0 < 0.10)
        test(len(ro["ABC"]) == 2)
        test(ro["ABC"][0] - 42.24 < 0.10)
        test(ro["ABC"][1] - -1.61 < 0.10)
        test(len(ro["aBc"]) == 2)
        test(ro["aBc"][0] - -3.14 < 0.10)
        test(ro["aBc"][1] - 3.14 < 0.10)
        self.called()

    def opStringDoubleSD(self, ro, do):
        test(len(do) == 1)
        test(len(do[""]) == 2)
        test(do[""][0] == 1.6E10)
        test(do[""][1] == 1.7E10)
        test(len(ro) == 3)
        test(len(ro["Hello!!"]) == 3)
        test(ro["Hello!!"][0] == 1.1E10)
        test(ro["Hello!!"][1] == 1.2E10)
        test(ro["Hello!!"][2] == 1.3E10)
        test(len(ro["Goodbye"]) == 2)
        test(ro["Goodbye"][0] == 1.4E10)
        test(ro["Goodbye"][1] == 1.5E10)
        test(len(ro[""]) == 2)
        test(ro[""][0] == 1.6E10)
        test(ro[""][1] == 1.7E10)
        self.called()

    def opStringStringSD(self, ro, do):
        test(len(do) == 1)
        test(len(do["ghi"]) == 2)
        test(do["ghi"][0] == "and")
        test(do["ghi"][1] == "xor")
        test(len(ro) == 3)
        test(len(ro["abc"]) == 3)
        test(ro["abc"][0] == "abc")
        test(ro["abc"][1] == "de")
        test(ro["abc"][2] == "fghi")
        test(len(ro["def"]) == 2)
        test(ro["def"][0] == "xyz")
        test(ro["def"][1] == "or")
        test(len(ro["ghi"]) == 2)
        test(ro["ghi"][0] == "and")
        test(ro["ghi"][1] == "xor")
        self.called()

    def opMyEnumMyEnumSD(self, ro, do):
        test(len(do) == 1)
        test(len(do[Test.MyEnum.enum1]) == 2)
        test(do[Test.MyEnum.enum1][0] == Test.MyEnum.enum3)
        test(do[Test.MyEnum.enum1][1] == Test.MyEnum.enum3)
        test(len(ro) == 3)
        test(len(ro[Test.MyEnum.enum3]) == 3)
        test(ro[Test.MyEnum.enum3][0] == Test.MyEnum.enum1)
        test(ro[Test.MyEnum.enum3][1] == Test.MyEnum.enum1)
        test(ro[Test.MyEnum.enum3][2] == Test.MyEnum.enum2)
        test(len(ro[Test.MyEnum.enum2]) == 2)
        test(ro[Test.MyEnum.enum2][0] == Test.MyEnum.enum1)
        test(ro[Test.MyEnum.enum2][1] == Test.MyEnum.enum2)
        test(len(ro[Test.MyEnum.enum1]) == 2)
        test(ro[Test.MyEnum.enum1][0] == Test.MyEnum.enum3)
        test(ro[Test.MyEnum.enum1][1] == Test.MyEnum.enum3)
        self.called()

    def opIntS(self, r):
        for j in range(0, len(r)):
            test(r[j] == -j)
        self.called()

    def opIdempotent(self):
        self.called()

    def opNonmutating(self):
        self.called()

    def opDerived(self):
        self.called()

    def exCB(self, ex):
        test(False)

def twowaysAMI(communicator, p):
    cb = Callback()
    p.begin_ice_ping(cb.ping, cb.exCB)
    cb.check()

    cb = Callback()
    p.begin_ice_isA(Test.MyClass.ice_staticId(), cb.isA, cb.exCB)
    cb.check()

    cb = Callback()
    p.begin_ice_id(cb.id, cb.exCB)
    cb.check()

    cb = Callback()
    p.begin_ice_ids(cb.ids, cb.exCB)
    cb.check()

    r = p.begin_opVoid()
    p.end_opVoid(r)

    cb = Callback()
    p.begin_opVoid(cb.opVoid, cb.exCB)
    cb.check()

    r = p.begin_opByte(0xff, 0x0f)
    (ret, p3) = p.end_opByte(r)
    test(p3 == 0xf0)
    test(ret == 0xff)

    cb = Callback()
    p.begin_opByte(0xff, 0x0f, cb.opByte, cb.exCB)
    cb.check()

    cb = Callback()
    p.begin_opBool(True, False, cb.opBool, cb.exCB)
    cb.check()

    cb = Callback()
    p.begin_opShortIntLong(10, 11, 12, cb.opShortIntLong, cb.exCB)
    cb.check()

    cb = Callback()
    p.begin_opFloatDouble(3.14, 1.1E10, cb.opFloatDouble, cb.exCB)
    cb.check()

    cb = Callback()
    p.begin_opString("hello", "world", cb.opString, cb.exCB)
    cb.check()

    cb = Callback()
    p.begin_opMyEnum(Test.MyEnum.enum2, cb.opMyEnum, cb.exCB)
    cb.check()

    cb = Callback(communicator)
    p.begin_opMyClass(p, cb.opMyClass, cb.exCB)
    cb.check()

    si1 = Test.Structure()
    si1.p = p
    si1.e = Test.MyEnum.enum3
    si1.s = Test.AnotherStruct()
    si1.s.s = "abc"
    si2 = Test.Structure()
    si2.p = None
    si2.e = Test.MyEnum.enum2
    si2.s = Test.AnotherStruct()
    si2.s.s = "def"

    cb = Callback(communicator)
    p.begin_opStruct(si1, si2, cb.opStruct, cb.exCB)
    cb.check()

    bsi1 = (0x01, 0x11, 0x12, 0x22)
    bsi2 = (0xf1, 0xf2, 0xf3, 0xf4)

    cb = Callback()
    p.begin_opByteS(bsi1, bsi2, cb.opByteS, cb.exCB)
    cb.check()

    bsi1 = (True, True, False)
    bsi2 = (False,)

    cb = Callback()
    p.begin_opBoolS(bsi1, bsi2, cb.opBoolS, cb.exCB)
    cb.check()

    ssi = (1, 2, 3)
    isi = (5, 6, 7, 8)
    lsi = (10, 30, 20)

    cb = Callback()
    p.begin_opShortIntLongS(ssi, isi, lsi, cb.opShortIntLongS, cb.exCB)
    cb.check()

    fsi = (3.14, 1.11)
    dsi = (1.1E10, 1.2E10, 1.3E10)

    cb = Callback()
    p.begin_opFloatDoubleS(fsi, dsi, cb.opFloatDoubleS, cb.exCB)
    cb.check()

    ssi1 = ('abc', 'de', 'fghi')
    ssi2 = ('xyz',)

    cb = Callback()
    p.begin_opStringS(ssi1, ssi2, cb.opStringS, cb.exCB)
    cb.check()

    bsi1 = ((0x01, 0x11, 0x12), (0xff,))
    bsi2 = ((0x0e,), (0xf2, 0xf1))

    cb = Callback()
    p.begin_opByteSS(bsi1, bsi2, cb.opByteSS, cb.exCB)
    cb.check()

    bsi1 = ((True,), (False,), (True, True),)
    bsi2 = ((False, False, True),)

    cb = Callback()
    p.begin_opBoolSS(bsi1, bsi2, cb.opBoolSS, cb.exCB)
    cb.check();

    ssi = ((1,2,5), (13,), ())
    isi = ((24, 98), (42,))
    lsi = ((496, 1729),)

    cb = Callback()
    p.begin_opShortIntLongSS(ssi, isi, lsi, cb.opShortIntLongSS, cb.exCB)
    cb.check()

    fsi = ((3.14,), (1.11,), ())
    dsi = ((1.1E10, 1.2E10, 1.3E10),)

    cb = Callback()
    p.begin_opFloatDoubleSS(fsi, dsi, cb.opFloatDoubleSS, cb.exCB)
    cb.check()

    ssi1 = (('abc',), ('de', 'fghi'))
    ssi2 = ((), (), ('xyz',))

    cb = Callback()
    p.begin_opStringSS(ssi1, ssi2, cb.opStringSS, cb.exCB)
    cb.check()

    di1 = {10: True, 100: False}
    di2 = {10: True, 11: False, 101: True}

    cb = Callback()
    p.begin_opByteBoolD(di1, di2, cb.opByteBoolD, cb.exCB)
    cb.check()

    di1 = {110: -1, 1100: 123123}
    di2 = {110: -1, 111: -100, 1101: 0}

    cb = Callback()
    p.begin_opShortIntD(di1, di2, cb.opShortIntD, cb.exCB)
    cb.check()

    di1 = {999999110: -1.1, 999999111: 123123.2}
    di2 = {999999110: -1.1, 999999120: -100.4, 999999130: 0.5}

    cb = Callback()
    p.begin_opLongFloatD(di1, di2, cb.opLongFloatD, cb.exCB)
    cb.check()

    di1 = {'foo': 'abc -1.1', 'bar': 'abc 123123.2'}
    di2 = {'foo': 'abc -1.1', 'FOO': 'abc -100.4', 'BAR': 'abc 0.5'}

    cb = Callback()
    p.begin_opStringStringD(di1, di2, cb.opStringStringD, cb.exCB)
    cb.check()

    di1 = {'abc': Test.MyEnum.enum1, '': Test.MyEnum.enum2}
    di2 = {'abc': Test.MyEnum.enum1, 'qwerty': Test.MyEnum.enum3, 'Hello!!': Test.MyEnum.enum2}

    cb = Callback()
    p.begin_opStringMyEnumD(di1, di2, cb.opStringMyEnumD, cb.exCB)
    cb.check()

    di1 = {Test.MyEnum.enum1: 'abc'}
    di2 = {Test.MyEnum.enum2: 'Hello!!', Test.MyEnum.enum3: 'qwerty'}

    cb = Callback()
    p.begin_opMyEnumStringD(di1, di2, cb.opMyEnumStringD, cb.exCB)
    cb.check()

    s11 = Test.MyStruct()
    s11.i = 1
    s11.j = 1
    s12 = Test.MyStruct()
    s12.i = 1
    s12.j = 2
    s22 = Test.MyStruct()
    s22.i = 2
    s22.j = 2
    s23 = Test.MyStruct()
    s23.i = 2
    s23.j = 3
    di1 = {s11: Test.MyEnum.enum1, s12: Test.MyEnum.enum2}
    di2 = {s11: Test.MyEnum.enum1, s22: Test.MyEnum.enum3, s23: Test.MyEnum.enum2}

    cb = Callback()
    p.begin_opMyStructMyEnumD(di1, di2, cb.opMyStructMyEnumD, cb.exCB)
    cb.check()

    dsi1 = ({ 10: True, 100: False }, { 10: True, 11: False, 101: True })
    dsi2 = ({ 100: False, 101: False },)

    cb = Callback()
    p.begin_opByteBoolDS(dsi1, dsi2, cb.opByteBoolDS, cb.exCB)
    cb.check()

    dsi1 = ({ 110: -1, 1100: 123123 }, { 110: -1, 111: -100, 1101: 0 })
    dsi2 = ({ 100: -1001 },)

    cb = Callback()
    p.begin_opShortIntDS(dsi1, dsi2, cb.opShortIntDS, cb.exCB)
    cb.called()

    dsi1 = ({ 999999110: -1.1, 999999111: 123123.2 }, { 999999110: -1.1, 999999120: -100.4, 999999130: 0.5 })
    dsi2 = ({ 999999140: 3.14 },)

    cb = Callback()
    p.begin_opLongFloatDS(dsi1, dsi2, cb.opLongFloatDS, cb.exCB)
    cb.called()

    dsi1 = ({ "foo": "abc -1.1", "bar": "abc 123123.2" }, { "foo": "abc -1.1", "FOO": "abc -100.4", "BAR": "abc 0.5" })
    dsi2 = ({ "f00": "ABC -3.14" },)

    cb = Callback()
    p.begin_opStringStringDS(dsi1, dsi2, cb.opStringStringDS, cb.exCB)
    cb.called()

    dsi1 = (
            { "abc": Test.MyEnum.enum1, "": Test.MyEnum.enum2 },
            { "abc": Test.MyEnum.enum1, "qwerty": Test.MyEnum.enum3, "Hello!!": Test.MyEnum.enum2 }
           )

    dsi2 = ({ "Goodbye": Test.MyEnum.enum1 },)

    cb = Callback()
    p.begin_opStringMyEnumDS(dsi1, dsi2, cb.opStringMyEnumDS, cb.exCB)
    cb.called()

    dsi1 = ({ Test.MyEnum.enum1: 'abc' }, { Test.MyEnum.enum2: 'Hello!!', Test.MyEnum.enum3: 'qwerty'})
    dsi2 = ({ Test.MyEnum.enum1: 'Goodbye' },)

    cb = Callback()
    p.begin_opMyEnumStringDS(dsi1, dsi2, cb.opMyEnumStringDS, cb.exCB)
    cb.called()

    s11 = Test.MyStruct(1, 1)
    s12 = Test.MyStruct(1, 2)

    s22 = Test.MyStruct(2, 2)
    s23 = Test.MyStruct(2, 3)

    dsi1 = (
            { s11: Test.MyEnum.enum1, s12: Test.MyEnum.enum2 },
            { s11: Test.MyEnum.enum1, s22: Test.MyEnum.enum3, s23: Test.MyEnum.enum2 }
           )
    dsi2 = ({ s23: Test.MyEnum.enum3 },)

    cb = Callback()
    p.begin_opMyStructMyEnumDS(dsi1, dsi2, cb.opMyStructMyEnumDS, cb.exCB)
    cb.called()

    sdi1 = { 0x01: (0x01, 0x11), 0x22: (0x12,) }
    sdi2 = { 0xf1: (0xf2, 0xf3) }

    cb = Callback()
    p.begin_opByteByteSD(sdi1, sdi2, cb.opByteByteSD, cb.exCB)
    cb.called()

    sdi1 = { False: (True, False), True: (False, True, True) }
    sdi2 = { False: (True, False) }

    cb = Callback()
    p.begin_opBoolBoolSD(sdi1, sdi2, cb.opBoolBoolSD, cb.exCB)
    cb.called()

    sdi1 = { 1: (1, 2, 3), 2: (4, 5) }
    sdi2 = { 4: (6, 7) }

    cb = Callback()
    p.begin_opShortShortSD(sdi1, sdi2, cb.opShortShortSD, cb.exCB)
    cb.called()

    sdi1 = { 100: (100, 200, 300), 200: (400, 500) }
    sdi2 = { 400: (600, 700) }

    cb = Callback()
    p.begin_opIntIntSD(sdi1, sdi2, cb.opIntIntSD, cb.exCB)
    cb.called()

    sdi1 = { 999999990: (999999110, 999999111, 999999110), 999999991: (999999120, 999999130) }
    sdi2 = { 999999992: (999999110, 999999120) }

    cb = Callback()
    p.begin_opLongLongSD(sdi1, sdi2, cb.opLongLongSD, cb.exCB)
    cb.called()

    sdi1 = { "abc": (-1.1, 123123.2, 100.0), "ABC": (42.24, -1.61) }
    sdi2 = { "aBc": (-3.14, 3.14) }

    cb = Callback()
    p.begin_opStringFloatSD(sdi1, sdi2, cb.opStringFloatSD, cb.exCB)
    cb.called()

    sdi1 = { "Hello!!": (1.1E10, 1.2E10, 1.3E10), "Goodbye": (1.4E10, 1.5E10) }
    sdi2 = { "": (1.6E10, 1.7E10) }

    cb = Callback()
    p.begin_opStringDoubleSD(sdi1, sdi2, cb.opStringDoubleSD, cb.exCB);
    cb.called()

    sdi1 = { "abc": ("abc", "de", "fghi") , "def": ("xyz", "or") }
    sdi2 = { "ghi": ("and", "xor") }

    cb = Callback()
    p.begin_opStringStringSD(sdi1, sdi2, cb.opStringStringSD, cb.exCB)
    cb.called()

    sdi1 = {
            Test.MyEnum.enum3: (Test.MyEnum.enum1, Test.MyEnum.enum1, Test.MyEnum.enum2),
            Test.MyEnum.enum2: (Test.MyEnum.enum1, Test.MyEnum.enum2)
           }
    sdi2 = { Test.MyEnum.enum1: (Test.MyEnum.enum3, Test.MyEnum.enum3) }

    cb = Callback()
    p.begin_opMyEnumMyEnumSD(sdi1, sdi2, cb.opMyEnumMyEnumSD, cb.exCB)
    cb.called()

    lengths = ( 0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000 )
    for l in lengths:
        s = []
        for i in range(l):
            s.append(i)
        cb = Callback(l)
        p.begin_opIntS(s, cb.opIntS, cb.exCB)
        cb.check()

    ctx = {'one': 'ONE', 'two': 'TWO', 'three': 'THREE'}

    test(len(p.ice_getContext()) == 0)
    r = p.begin_opContext()
    c = p.end_opContext(r)
    test(c != ctx)

    test(len(p.ice_getContext()) == 0)
    r = p.begin_opContext(_ctx=ctx)
    c = p.end_opContext(r)
    test(c == ctx)

    p2 = Test.MyClassPrx.checkedCast(p.ice_context(ctx))
    test(p2.ice_getContext() == ctx)
    r = p2.begin_opContext()
    c = p2.end_opContext(r)
    test(c == ctx)

    r = p2.begin_opContext(_ctx=ctx)
    c = p2.end_opContext(r)
    test(c == ctx)

    #
    # Test implicit context propagation
    #
    if p.ice_getConnection():
        impls = ( 'Shared', 'PerThread' )
        for i in impls:
            initData = Ice.InitializationData()
            initData.properties = communicator.getProperties().clone()
            initData.properties.setProperty('Ice.ImplicitContext', i)
            ic = Ice.initialize(data=initData)

            ctx = {'one': 'ONE', 'two': 'TWO', 'three': 'THREE'}

            p3 = Test.MyClassPrx.uncheckedCast(ic.stringToProxy("test:default -p 12010"))

            ic.getImplicitContext().setContext(ctx)
            test(ic.getImplicitContext().getContext() == ctx)
            r = p3.begin_opContext()
            c = p3.end_opContext(r)
            test(c == ctx)

            ic.getImplicitContext().put('zero', 'ZERO')

            ctx = ic.getImplicitContext().getContext()
            r = p3.begin_opContext()
            c = p3.end_opContext(r)
            test(c == ctx)

            prxContext = {'one': 'UN', 'four': 'QUATRE'}

            combined = {}
            combined.update(ctx)
            combined.update(prxContext)
            test(combined['one'] == 'UN')

            p3 = Test.MyClassPrx.uncheckedCast(p3.ice_context(prxContext))
            ic.getImplicitContext().setContext({})
            r = p3.begin_opContext()
            c = p3.end_opContext(r)
            test(c == prxContext)

            ic.getImplicitContext().setContext(ctx)
            r = p3.begin_opContext()
            c = p3.end_opContext(r)
            test(c == combined)

            ic.destroy()

    cb = Callback()
    p.begin_opIdempotent(cb.opIdempotent, cb.exCB)
    cb.check()

    cb = Callback()
    p.begin_opNonmutating(cb.opNonmutating, cb.exCB)
    cb.check()

    derived = Test.MyDerivedClassPrx.checkedCast(p)
    test(derived)
    cb = Callback()
    derived.begin_opDerived(cb.opDerived, cb.exCB)
    cb.check()
    
    
    r = p.begin_opByte1(0xFF)
    test(p.end_opByte1(r) == 0xFF)
    
    r = p.begin_opShort1(0x7FFF)
    test(p.end_opShort1(r) == 0x7FFF)
    
    r = p.begin_opInt1(0x7FFFFFFF)
    test(p.end_opInt1(r) == 0x7FFFFFFF)
    
    r = p.begin_opLong1(0x7FFFFFFFFFFFFFFF)
    test(p.end_opLong1(r) == 0x7FFFFFFFFFFFFFFF)
    
    r = p.begin_opFloat1(1.0)
    test(p.end_opFloat1(r) == 1.0)
    
    r = p.begin_opDouble1(1.0)
    test(p.end_opDouble1(r) == 1.0)
    
    r = p.begin_opString1("opString1")
    test(p.end_opString1(r) == "opString1")
    
    r = p.begin_opStringS1(None)
    test(len(p.end_opStringS1(r)) == 0)
    
    r = p.begin_opByteBoolD1(None)
    test(len(p.end_opByteBoolD1(r)) == 0)
    
    r = p.begin_opStringS2(None)
    test(len(p.end_opStringS2(r)) == 0)
    
    r = p.begin_opByteBoolD2(None)
    test(len(p.end_opByteBoolD2(r)) == 0)
