# Copyright (c) ZeroC, Inc.

import math
import threading
from typing import cast

from generated.test.Ice.operations import Test
from TestHelper import TestHelper, test

import Ice


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
    def __init__(self, communicator: Ice.Communicator | None = None):
        CallbackBase.__init__(self)
        self._communicator = communicator

    def opByte(self, f: Ice.Future):
        try:
            (r, b) = f.result()
            test(b == 0xF0)
            test(r == 0xFF)
            self.called()
        except Exception:
            test(False)

    def opBool(self, f: Ice.Future):
        try:
            (r, b) = f.result()
            test(b)
            test(not r)
            self.called()
        except Exception:
            test(False)

    def opShortIntLong(self, f: Ice.Future):
        try:
            (r, s, i, l) = f.result()  # noqa: E741
            test(s == 10)
            test(i == 11)
            test(l == 12)
            test(r == 12)
            self.called()
        except Exception:
            test(False)

    def opFloatDouble(self, fut: Ice.Future):
        try:
            (r, f, d) = fut.result()
            test(f - 3.14 < 0.001)
            test(d == 1.1e10)
            test(r == 1.1e10)
            self.called()
        except Exception:
            test(False)

    def opString(self, f: Ice.Future):
        try:
            (r, s) = f.result()
            test(s == "world hello")
            test(r == "hello world")
            self.called()
        except Exception:
            test(False)

    def opMyEnum(self, f: Ice.Future):
        try:
            (r, e) = f.result()
            test(e == Test.MyEnum.enum2)
            test(r == Test.MyEnum.enum3)
            self.called()
        except Exception:
            test(False)

    def opMyClass(self, f: Ice.Future):
        try:
            (r, c1, c2) = f.result()
            test(c1.ice_getIdentity() == Ice.stringToIdentity("test"))
            test(c2.ice_getIdentity() == Ice.stringToIdentity("noSuchIdentity"))
            test(r.ice_getIdentity() == Ice.stringToIdentity("test"))
            # We can't do the callbacks below in serialize mode
            assert self._communicator is not None
            if self._communicator.getProperties().getPropertyAsInt("Ice.Client.ThreadPool.Serialize") == 0:
                r.opVoid()
                c1.opVoid()
                try:
                    c2.opVoid()
                    test(False)
                except Ice.ObjectNotExistException:
                    pass
            self.called()
        except Exception:
            test(False)

    def opStruct(self, f: Ice.Future):
        try:
            (rso, so) = f.result()
            test(rso.p is None)
            test(rso.e == Test.MyEnum.enum2)
            test(rso.s.s == "def")
            test(so.e == Test.MyEnum.enum3)
            test(so.s.s == "a new string")
            # We can't do the callbacks below in serialize mode.
            assert self._communicator is not None
            if self._communicator.getProperties().getPropertyAsInt("Ice.ThreadPool.Client.Serialize") == 0:
                so.p.opVoid()
            self.called()
        except Exception:
            test(False)

    def opByteS(self, f: Ice.Future):
        try:
            (rso, bso) = f.result()
            test(len(bso) == 4)
            test(len(rso) == 8)
            test(bso[0] == 0x22)
            test(bso[1] == 0x12)
            test(bso[2] == 0x11)
            test(bso[3] == 0x01)
            test(rso[0] == 0x01)
            test(rso[1] == 0x11)
            test(rso[2] == 0x12)
            test(rso[3] == 0x22)
            test(rso[4] == 0xF1)
            test(rso[5] == 0xF2)
            test(rso[6] == 0xF3)
            test(rso[7] == 0xF4)
            self.called()
        except Exception:
            test(False)

    def opBoolS(self, f: Ice.Future):
        try:
            (rso, bso) = f.result()
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
        except Exception:
            test(False)

    def opShortIntLongS(self, f: Ice.Future):
        try:
            (rso, sso, iso, lso) = f.result()
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
        except Exception:
            test(False)

    def opFloatDoubleS(self, f: Ice.Future):
        try:
            (rso, fso, dso) = f.result()
            test(len(fso) == 2)
            test(fso[0] - 3.14 < 0.001)
            test(fso[1] - 1.11 < 0.001)
            test(len(dso) == 3)
            test(dso[0] == 1.3e10)
            test(dso[1] == 1.2e10)
            test(dso[2] == 1.1e10)
            test(len(rso) == 5)
            test(rso[0] == 1.1e10)
            test(rso[1] == 1.2e10)
            test(rso[2] == 1.3e10)
            test(rso[3] - 3.14 < 0.001)
            test(rso[4] - 1.11 < 0.001)
            self.called()
        except Exception:
            test(False)

    def opStringS(self, f: Ice.Future):
        try:
            (rso, sso) = f.result()
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
        except Exception:
            test(False)

    def opByteSS(self, f: Ice.Future):
        try:
            (rso, bso) = f.result()
            test(len(bso) == 2)
            test(len(bso[0]) == 1)
            test(len(bso[1]) == 3)
            test(len(rso) == 4)
            test(len(rso[0]) == 3)
            test(len(rso[1]) == 1)
            test(len(rso[2]) == 1)
            test(len(rso[3]) == 2)
            test(bso[0][0] == 0xFF)
            test(bso[1][0] == 0x01)
            test(bso[1][1] == 0x11)
            test(bso[1][2] == 0x12)
            test(rso[0][0] == 0x01)
            test(rso[0][1] == 0x11)
            test(rso[0][2] == 0x12)
            test(rso[1][0] == 0xFF)
            test(rso[2][0] == 0x0E)
            test(rso[3][0] == 0xF2)
            test(rso[3][1] == 0xF1)
            self.called()
        except Exception:
            test(False)

    def opBoolSS(self, f: Ice.Future):
        try:
            (rso, bso) = f.result()
            test(len(bso) == 4)
            test(len(bso[0]) == 1)
            test(bso[0][0])
            test(len(bso[1]) == 1)
            test(not bso[1][0])
            test(len(bso[2]) == 2)
            test(bso[2][0])
            test(bso[2][1])
            test(len(bso[3]) == 3)
            test(not bso[3][0])
            test(not bso[3][1])
            test(bso[3][2])
            test(len(rso) == 3)
            test(len(rso[0]) == 2)
            test(rso[0][0])
            test(rso[0][1])
            test(len(rso[1]) == 1)
            test(not rso[1][0])
            test(len(rso[2]) == 1)
            test(rso[2][0])
            self.called()
        except Exception:
            test(False)

    def opShortIntLongSS(self, f: Ice.Future):
        try:
            (rso, sso, iso, lso) = f.result()
            test(len(rso) == 1)
            test(len(rso[0]) == 2)
            test(rso[0][0] == 496)
            test(rso[0][1] == 1729)
            test(len(sso) == 3)
            test(len(sso[0]) == 3)
            test(sso[0][0] == 1)
            test(sso[0][1] == 2)
            test(sso[0][2] == 5)
            test(len(sso[1]) == 1)
            test(sso[1][0] == 13)
            test(len(sso[2]) == 0)
            test(len(iso) == 2)
            test(len(iso[0]) == 1)
            test(iso[0][0] == 42)
            test(len(iso[1]) == 2)
            test(iso[1][0] == 24)
            test(iso[1][1] == 98)
            test(len(lso) == 2)
            test(len(lso[0]) == 2)
            test(lso[0][0] == 496)
            test(lso[0][1] == 1729)
            test(len(lso[1]) == 2)
            test(lso[1][0] == 496)
            test(lso[1][1] == 1729)
            self.called()
        except Exception:
            test(False)

    def opFloatDoubleSS(self, f: Ice.Future):
        try:
            (rso, fso, dso) = f.result()
            test(len(fso) == 3)
            test(len(fso[0]) == 1)
            test(fso[0][0] - 3.14 < 0.001)
            test(len(fso[1]) == 1)
            test(fso[1][0] - 1.11 < 0.001)
            test(len(fso[2]) == 0)
            test(len(dso) == 1)
            test(len(dso[0]) == 3)
            test(dso[0][0] == 1.1e10)
            test(dso[0][1] == 1.2e10)
            test(dso[0][2] == 1.3e10)
            test(len(rso) == 2)
            test(len(rso[0]) == 3)
            test(rso[0][0] == 1.1e10)
            test(rso[0][1] == 1.2e10)
            test(rso[0][2] == 1.3e10)
            test(len(rso[1]) == 3)
            test(rso[1][0] == 1.1e10)
            test(rso[1][1] == 1.2e10)
            test(rso[1][2] == 1.3e10)
            self.called()
        except Exception:
            test(False)

    def opStringSS(self, f: Ice.Future):
        try:
            (rso, sso) = f.result()
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
        except Exception:
            test(False)

    def opByteBoolD(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
            di1 = {10: True, 100: False}
            test(do == di1)
            test(len(ro) == 4)
            test(ro[10])
            test(not ro[11])
            test(not ro[100])
            test(ro[101])
            self.called()
        except Exception:
            test(False)

    def opShortIntD(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
            di1 = {110: -1, 1100: 123123}
            test(do == di1)
            test(len(ro) == 4)
            test(ro[110] == -1)
            test(ro[111] == -100)
            test(ro[1100] == 123123)
            test(ro[1101] == 0)
            self.called()
        except Exception:
            test(False)

    def opLongFloatD(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
            di1 = {999999110: -1.1, 999999111: 123123.2}
            for k in do:
                test(math.fabs(do[k] - di1[k]) < 0.01)
            test(len(ro) == 4)
            test(ro[999999110] - -1.1 < 0.01)
            test(ro[999999120] - -100.4 < 0.01)
            test(ro[999999111] - 123123.2 < 0.01)
            test(ro[999999130] - 0.5 < 0.01)
            self.called()
        except Exception:
            test(False)

    def opStringStringD(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
            di1 = {"foo": "abc -1.1", "bar": "abc 123123.2"}
            test(do == di1)
            test(len(ro) == 4)
            test(ro["foo"] == "abc -1.1")
            test(ro["FOO"] == "abc -100.4")
            test(ro["bar"] == "abc 123123.2")
            test(ro["BAR"] == "abc 0.5")
            self.called()
        except Exception:
            test(False)

    def opStringMyEnumD(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
            di1 = {"abc": Test.MyEnum.enum1, "": Test.MyEnum.enum2}
            test(do == di1)
            test(len(ro) == 4)
            test(ro["abc"] == Test.MyEnum.enum1)
            test(ro["qwerty"] == Test.MyEnum.enum3)
            test(ro[""] == Test.MyEnum.enum2)
            test(ro["Hello!!"] == Test.MyEnum.enum2)
            self.called()
        except Exception:
            test(False)

    def opMyEnumStringD(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
            di1 = {Test.MyEnum.enum1: "abc"}
            test(do == di1)
            test(len(ro) == 3)
            test(ro[Test.MyEnum.enum1] == "abc")
            test(ro[Test.MyEnum.enum2] == "Hello!!")
            test(ro[Test.MyEnum.enum3] == "qwerty")
            self.called()
        except Exception:
            test(False)

    def opMyStructMyEnumD(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
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
        except Exception:
            test(False)

    def opByteBoolDS(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
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
        except Exception:
            test(False)

    def opShortIntDS(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
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
        except Exception:
            test(False)

    def opLongFloatDS(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
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
        except Exception:
            test(False)

    def opStringStringDS(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
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
        except Exception:
            test(False)

    def opStringMyEnumDS(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
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
        except Exception:
            test(False)

    def opMyEnumStringDS(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
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
        except Exception:
            test(False)

    def opMyStructMyEnumDS(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
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
        except Exception:
            test(False)

    def opByteByteSD(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
            test(len(do) == 1)
            test(len(do[0xF1]) == 2)
            test(do[0xF1][0] == 0xF2)
            test(do[0xF1][1] == 0xF3)
            test(len(ro) == 3)
            test(len(ro[0x01]) == 2)
            test(ro[0x01][0] == 0x01)
            test(ro[0x01][1] == 0x11)
            test(len(ro[0x22]) == 1)
            test(ro[0x22][0] == 0x12)
            test(len(ro[0xF1]) == 2)
            test(ro[0xF1][0] == 0xF2)
            test(ro[0xF1][1] == 0xF3)
            self.called()
        except Exception:
            test(False)

    def opBoolBoolSD(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
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
        except Exception:
            test(False)

    def opShortShortSD(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
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
        except Exception:
            test(False)

    def opIntIntSD(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
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
        except Exception:
            test(False)

    def opLongLongSD(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
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
        except Exception:
            test(False)

    def opStringFloatSD(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
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
        except Exception:
            test(False)

    def opStringDoubleSD(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
            test(len(do) == 1)
            test(len(do[""]) == 2)
            test(do[""][0] == 1.6e10)
            test(do[""][1] == 1.7e10)
            test(len(ro) == 3)
            test(len(ro["Hello!!"]) == 3)
            test(ro["Hello!!"][0] == 1.1e10)
            test(ro["Hello!!"][1] == 1.2e10)
            test(ro["Hello!!"][2] == 1.3e10)
            test(len(ro["Goodbye"]) == 2)
            test(ro["Goodbye"][0] == 1.4e10)
            test(ro["Goodbye"][1] == 1.5e10)
            test(len(ro[""]) == 2)
            test(ro[""][0] == 1.6e10)
            test(ro[""][1] == 1.7e10)
            self.called()
        except Exception:
            test(False)

    def opStringStringSD(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
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
        except Exception:
            test(False)

    def opMyEnumMyEnumSD(self, f: Ice.Future):
        try:
            (ro, do) = f.result()
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
        except Exception:
            test(False)

    def opIntS(self, f: Ice.Future):
        try:
            r = f.result()
            for j in range(0, len(r)):
                test(r[j] == -j)
            self.called()
        except Exception:
            test(False)

    def opIdempotent(self, f: Ice.Future):
        self.called()

    def opDerived(self, f: Ice.Future):
        self.called()


def twowaysFuture(helper: TestHelper, p: Test.MyClassPrx):
    communicator = helper.communicator()
    f = p.ice_pingAsync()
    assert isinstance(f, Ice.Future)
    test(f.result() is None)

    f = p.ice_isAAsync(Test.MyClass.ice_staticId())
    assert isinstance(f, Ice.Future)
    test(f.result())

    f = p.ice_idAsync()
    assert isinstance(f, Ice.Future)
    test(f.result() == "::Test::MyDerivedClass")

    f = p.ice_idsAsync()
    assert isinstance(f, Ice.Future)
    test(len(f.result()) == 3)

    f = p.opVoidAsync()
    assert isinstance(f, Ice.Future)
    test(f.result() is None)

    cb = Callback()
    cast(Ice.InvocationFuture, p.opVoidAsync()).add_done_callback(lambda f: cb.called())
    cb.check()

    f = p.opByteAsync(0xFF, 0x0F)
    assert isinstance(f, Ice.Future)
    (ret, p3) = f.result()
    test(p3 == 0xF0)
    test(ret == 0xFF)

    cb = Callback()
    cast(Ice.InvocationFuture, p.opByteAsync(0xFF, 0x0F)).add_done_callback(cb.opByte)
    cb.check()

    cb = Callback()
    cast(Ice.InvocationFuture, p.opBoolAsync(True, False)).add_done_callback(cb.opBool)
    cb.check()

    cb = Callback()
    cast(Ice.InvocationFuture, p.opShortIntLongAsync(10, 11, 12)).add_done_callback(cb.opShortIntLong)
    cb.check()

    cb = Callback()
    cast(Ice.InvocationFuture, p.opFloatDoubleAsync(3.14, 1.1e10)).add_done_callback(cb.opFloatDouble)
    cb.check()

    cb = Callback()
    cast(Ice.InvocationFuture, p.opStringAsync("hello", "world")).add_done_callback(cb.opString)
    cb.check()

    cb = Callback()
    cast(Ice.InvocationFuture, p.opMyEnumAsync(Test.MyEnum.enum2)).add_done_callback(cb.opMyEnum)
    cb.check()

    cb = Callback(communicator)
    cast(Ice.InvocationFuture, p.opMyClassAsync(p)).add_done_callback(cb.opMyClass)
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
    cast(Ice.InvocationFuture, p.opStructAsync(si1, si2)).add_done_callback(cb.opStruct)
    cb.check()

    bsi1 = (0x01, 0x11, 0x12, 0x22)
    bsi2 = (0xF1, 0xF2, 0xF3, 0xF4)

    cb = Callback()
    cast(Ice.InvocationFuture, p.opByteSAsync(bsi1, bsi2)).add_done_callback(cb.opByteS)
    cb.check()

    bsi1 = (True, True, False)
    bsi2 = (False,)

    cb = Callback()
    cast(Ice.InvocationFuture, p.opBoolSAsync(bsi1, bsi2)).add_done_callback(cb.opBoolS)
    cb.check()

    ssi = (1, 2, 3)
    isi = (5, 6, 7, 8)
    lsi = (10, 30, 20)

    cb = Callback()
    cast(Ice.InvocationFuture, p.opShortIntLongSAsync(ssi, isi, lsi)).add_done_callback(cb.opShortIntLongS)
    cb.check()

    fsi = (3.14, 1.11)
    dsi = (1.1e10, 1.2e10, 1.3e10)

    cb = Callback()
    cast(Ice.InvocationFuture, p.opFloatDoubleSAsync(fsi, dsi)).add_done_callback(cb.opFloatDoubleS)
    cb.check()

    ssi1 = ("abc", "de", "fghi")
    ssi2 = ("xyz",)

    cb = Callback()
    cast(Ice.InvocationFuture, p.opStringSAsync(ssi1, ssi2)).add_done_callback(cb.opStringS)
    cb.check()

    bsi1 = ((0x01, 0x11, 0x12), (0xFF,))
    bsi2 = ((0x0E,), (0xF2, 0xF1))

    cb = Callback()
    cast(Ice.InvocationFuture, p.opByteSSAsync(bsi1, bsi2)).add_done_callback(cb.opByteSS)
    cb.check()

    bsi1 = (
        (True,),
        (False,),
        (True, True),
    )
    bsi2 = ((False, False, True),)

    cb = Callback()
    cast(Ice.InvocationFuture, p.opBoolSSAsync(bsi1, bsi2)).add_done_callback(cb.opBoolSS)
    cb.check()

    ssi = ((1, 2, 5), (13,), ())
    isi = ((24, 98), (42,))
    lsi = ((496, 1729),)

    cb = Callback()
    cast(Ice.InvocationFuture, p.opShortIntLongSSAsync(ssi, isi, lsi)).add_done_callback(cb.opShortIntLongSS)
    cb.check()

    fsi = ((3.14,), (1.11,), ())
    dsi = ((1.1e10, 1.2e10, 1.3e10),)

    cb = Callback()
    cast(Ice.InvocationFuture, p.opFloatDoubleSSAsync(fsi, dsi)).add_done_callback(cb.opFloatDoubleSS)
    cb.check()

    ssi1 = (("abc",), ("de", "fghi"))
    ssi2 = ((), (), ("xyz",))

    cb = Callback()
    cast(Ice.InvocationFuture, p.opStringSSAsync(ssi1, ssi2)).add_done_callback(cb.opStringSS)
    cb.check()

    di1 = {10: True, 100: False}
    di2 = {10: True, 11: False, 101: True}

    cb = Callback()
    cast(Ice.InvocationFuture, p.opByteBoolDAsync(di1, di2)).add_done_callback(cb.opByteBoolD)
    cb.check()

    di1 = {110: -1, 1100: 123123}
    di2 = {110: -1, 111: -100, 1101: 0}

    cb = Callback()
    cast(Ice.InvocationFuture, p.opShortIntDAsync(di1, di2)).add_done_callback(cb.opShortIntD)
    cb.check()

    di1 = {999999110: -1.1, 999999111: 123123.2}
    di2 = {999999110: -1.1, 999999120: -100.4, 999999130: 0.5}

    cb = Callback()
    cast(Ice.InvocationFuture, p.opLongFloatDAsync(di1, di2)).add_done_callback(cb.opLongFloatD)
    cb.check()

    di1 = {"foo": "abc -1.1", "bar": "abc 123123.2"}
    di2 = {"foo": "abc -1.1", "FOO": "abc -100.4", "BAR": "abc 0.5"}

    cb = Callback()
    cast(Ice.InvocationFuture, p.opStringStringDAsync(di1, di2)).add_done_callback(cb.opStringStringD)
    cb.check()

    di1 = {"abc": Test.MyEnum.enum1, "": Test.MyEnum.enum2}
    di2 = {
        "abc": Test.MyEnum.enum1,
        "qwerty": Test.MyEnum.enum3,
        "Hello!!": Test.MyEnum.enum2,
    }

    cb = Callback()
    cast(Ice.InvocationFuture, p.opStringMyEnumDAsync(di1, di2)).add_done_callback(cb.opStringMyEnumD)
    cb.check()

    di1 = {Test.MyEnum.enum1: "abc"}
    di2 = {Test.MyEnum.enum2: "Hello!!", Test.MyEnum.enum3: "qwerty"}

    cb = Callback()
    cast(Ice.InvocationFuture, p.opMyEnumStringDAsync(di1, di2)).add_done_callback(cb.opMyEnumStringD)
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
    cast(Ice.InvocationFuture, p.opMyStructMyEnumDAsync(di1, di2)).add_done_callback(cb.opMyStructMyEnumD)
    cb.check()

    dsi1 = ({10: True, 100: False}, {10: True, 11: False, 101: True})
    dsi2 = ({100: False, 101: False},)

    cb = Callback()
    cast(Ice.InvocationFuture, p.opByteBoolDSAsync(dsi1, dsi2)).add_done_callback(cb.opByteBoolDS)
    cb.check()

    dsi1 = ({110: -1, 1100: 123123}, {110: -1, 111: -100, 1101: 0})
    dsi2 = ({100: -1001},)

    cb = Callback()
    cast(Ice.InvocationFuture, p.opShortIntDSAsync(dsi1, dsi2)).add_done_callback(cb.opShortIntDS)
    cb.called()

    dsi1 = (
        {999999110: -1.1, 999999111: 123123.2},
        {999999110: -1.1, 999999120: -100.4, 999999130: 0.5},
    )
    dsi2 = ({999999140: 3.14},)

    cb = Callback()
    cast(Ice.InvocationFuture, p.opLongFloatDSAsync(dsi1, dsi2)).add_done_callback(cb.opLongFloatDS)
    cb.called()

    dsi1 = (
        {"foo": "abc -1.1", "bar": "abc 123123.2"},
        {"foo": "abc -1.1", "FOO": "abc -100.4", "BAR": "abc 0.5"},
    )
    dsi2 = ({"f00": "ABC -3.14"},)

    cb = Callback()
    cast(Ice.InvocationFuture, p.opStringStringDSAsync(dsi1, dsi2)).add_done_callback(cb.opStringStringDS)
    cb.called()

    dsi1 = (
        {"abc": Test.MyEnum.enum1, "": Test.MyEnum.enum2},
        {
            "abc": Test.MyEnum.enum1,
            "qwerty": Test.MyEnum.enum3,
            "Hello!!": Test.MyEnum.enum2,
        },
    )

    dsi2 = ({"Goodbye": Test.MyEnum.enum1},)

    cb = Callback()
    cast(Ice.InvocationFuture, p.opStringMyEnumDSAsync(dsi1, dsi2)).add_done_callback(cb.opStringMyEnumDS)
    cb.called()

    dsi1 = (
        {Test.MyEnum.enum1: "abc"},
        {Test.MyEnum.enum2: "Hello!!", Test.MyEnum.enum3: "qwerty"},
    )
    dsi2 = ({Test.MyEnum.enum1: "Goodbye"},)

    cb = Callback()
    cast(Ice.InvocationFuture, p.opMyEnumStringDSAsync(dsi1, dsi2)).add_done_callback(cb.opMyEnumStringDS)
    cb.called()

    s11 = Test.MyStruct(1, 1)
    s12 = Test.MyStruct(1, 2)

    s22 = Test.MyStruct(2, 2)
    s23 = Test.MyStruct(2, 3)

    dsi1 = (
        {s11: Test.MyEnum.enum1, s12: Test.MyEnum.enum2},
        {s11: Test.MyEnum.enum1, s22: Test.MyEnum.enum3, s23: Test.MyEnum.enum2},
    )
    dsi2 = ({s23: Test.MyEnum.enum3},)

    cb = Callback()
    cast(Ice.InvocationFuture, p.opMyStructMyEnumDSAsync(dsi1, dsi2)).add_done_callback(cb.opMyStructMyEnumDS)
    cb.called()

    sdi1 = {0x01: (0x01, 0x11), 0x22: (0x12,)}
    sdi2 = {0xF1: (0xF2, 0xF3)}

    cb = Callback()
    cast(Ice.InvocationFuture, p.opByteByteSDAsync(sdi1, sdi2)).add_done_callback(cb.opByteByteSD)
    cb.called()

    sdi1 = {False: (True, False), True: (False, True, True)}
    sdi2 = {False: (True, False)}

    cb = Callback()
    cast(Ice.InvocationFuture, p.opBoolBoolSDAsync(sdi1, sdi2)).add_done_callback(cb.opBoolBoolSD)
    cb.called()

    sdi1 = {1: (1, 2, 3), 2: (4, 5)}
    sdi2 = {4: (6, 7)}

    cb = Callback()
    cast(Ice.InvocationFuture, p.opShortShortSDAsync(sdi1, sdi2)).add_done_callback(cb.opShortShortSD)
    cb.called()

    sdi1 = {100: (100, 200, 300), 200: (400, 500)}
    sdi2 = {400: (600, 700)}

    cb = Callback()
    cast(Ice.InvocationFuture, p.opIntIntSDAsync(sdi1, sdi2)).add_done_callback(cb.opIntIntSD)
    cb.called()

    sdi1 = {
        999999990: (999999110, 999999111, 999999110),
        999999991: (999999120, 999999130),
    }
    sdi2 = {999999992: (999999110, 999999120)}

    cb = Callback()
    cast(Ice.InvocationFuture, p.opLongLongSDAsync(sdi1, sdi2)).add_done_callback(cb.opLongLongSD)
    cb.called()

    sdi1 = {"abc": (-1.1, 123123.2, 100.0), "ABC": (42.24, -1.61)}
    sdi2 = {"aBc": (-3.14, 3.14)}

    cb = Callback()
    cast(Ice.InvocationFuture, p.opStringFloatSDAsync(sdi1, sdi2)).add_done_callback(cb.opStringFloatSD)
    cb.called()

    sdi1 = {"Hello!!": (1.1e10, 1.2e10, 1.3e10), "Goodbye": (1.4e10, 1.5e10)}
    sdi2 = {"": (1.6e10, 1.7e10)}

    cb = Callback()
    cast(Ice.InvocationFuture, p.opStringDoubleSDAsync(sdi1, sdi2)).add_done_callback(cb.opStringDoubleSD)
    cb.called()

    sdi1 = {"abc": ("abc", "de", "fghi"), "def": ("xyz", "or")}
    sdi2 = {"ghi": ("and", "xor")}

    cb = Callback()
    cast(Ice.InvocationFuture, p.opStringStringSDAsync(sdi1, sdi2)).add_done_callback(cb.opStringStringSD)
    cb.called()

    sdi1 = {
        Test.MyEnum.enum3: (Test.MyEnum.enum1, Test.MyEnum.enum1, Test.MyEnum.enum2),
        Test.MyEnum.enum2: (Test.MyEnum.enum1, Test.MyEnum.enum2),
    }
    sdi2 = {Test.MyEnum.enum1: (Test.MyEnum.enum3, Test.MyEnum.enum3)}

    cb = Callback()
    cast(Ice.InvocationFuture, p.opMyEnumMyEnumSDAsync(sdi1, sdi2)).add_done_callback(cb.opMyEnumMyEnumSD)
    cb.called()

    lengths = (0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000)
    for length in lengths:
        s = [i for i in range(length)]
        cb = Callback()
        cast(Ice.InvocationFuture, p.opIntSAsync(s)).add_done_callback(cb.opIntS)
        cb.check()

    ctx = {"one": "ONE", "two": "TWO", "three": "THREE"}

    test(len(cast(dict[str, str], p.ice_getContext())) == 0)
    f = p.opContextAsync()
    assert isinstance(f, Ice.Future)
    c = f.result()
    test(c != ctx)

    test(len(cast(dict[str, str], p.ice_getContext())) == 0)
    f = p.opContextAsync(context=ctx)
    assert isinstance(f, Ice.Future)
    c = f.result()
    test(c == ctx)

    p2 = Test.MyClassPrx.checkedCast(p.ice_context(ctx))
    assert p2 is not None
    test(cast(dict[str, str], p2.ice_getContext()) == ctx)
    f = p2.opContextAsync()
    assert isinstance(f, Ice.Future)
    c = f.result()
    test(c == ctx)

    f = p2.opContextAsync(context=ctx)
    assert isinstance(f, Ice.Future)
    c = f.result()
    test(c == ctx)

    #
    # Test implicit context propagation
    #
    if p.ice_getConnection():
        impls = ("Shared", "PerThread")
        for i in impls:
            initData = Ice.InitializationData()
            initData.properties = communicator.getProperties().clone()
            initData.properties.setProperty("Ice.ImplicitContext", i)
            ic = Ice.Communicator(initData=initData)

            ctx = {"one": "ONE", "two": "TWO", "three": "THREE"}

            p3 = Test.MyClassPrx(ic, f"test:{helper.getTestEndpoint()}")

            cast(Ice.ImplicitContext, ic.getImplicitContext()).setContext(ctx)
            test(cast(Ice.ImplicitContext, ic.getImplicitContext()).getContext() == ctx)
            f = p3.opContextAsync()
            assert isinstance(f, Ice.Future)
            c = f.result()
            test(c == ctx)

            cast(Ice.ImplicitContext, ic.getImplicitContext()).put("zero", "ZERO")

            ctx = cast(Ice.ImplicitContext, ic.getImplicitContext()).getContext()
            f = p3.opContextAsync()
            assert isinstance(f, Ice.Future)
            c = f.result()
            test(c == ctx)

            prxContext = {"one": "UN", "four": "QUATRE"}

            combined = {}
            combined.update(ctx)
            combined.update(prxContext)
            test(combined["one"] == "UN")

            p3 = Test.MyClassPrx.uncheckedCast(p3.ice_context(prxContext))
            cast(Ice.ImplicitContext, ic.getImplicitContext()).setContext({})
            f = p3.opContextAsync()
            assert isinstance(f, Ice.Future)
            c = f.result()
            test(c == prxContext)

            cast(Ice.ImplicitContext, ic.getImplicitContext()).setContext(ctx)
            f = p3.opContextAsync()
            assert isinstance(f, Ice.Future)
            c = f.result()
            test(c == combined)

            ic.destroy()

    cb = Callback()
    cast(Ice.InvocationFuture, p.opIdempotentAsync()).add_done_callback(cb.opIdempotent)
    cb.check()

    derived = Test.MyDerivedClassPrx.checkedCast(p)
    assert derived is not None
    cb = Callback()
    cast(Ice.InvocationFuture, derived.opDerivedAsync()).add_done_callback(cb.opDerived)
    cb.check()

    f = p.opByte1Async(0xFF)
    assert isinstance(f, Ice.Future)
    test(f.result() == 0xFF)

    f = p.opShort1Async(0x7FFF)
    assert isinstance(f, Ice.Future)
    test(f.result() == 0x7FFF)

    f = p.opInt1Async(0x7FFFFFFF)
    assert isinstance(f, Ice.Future)
    test(f.result() == 0x7FFFFFFF)

    f = p.opLong1Async(0x7FFFFFFFFFFFFFFF)
    assert isinstance(f, Ice.Future)
    test(f.result() == 0x7FFFFFFFFFFFFFFF)

    f = p.opFloat1Async(1.0)
    assert isinstance(f, Ice.Future)
    test(f.result() == 1.0)

    f = p.opDouble1Async(1.0)
    assert isinstance(f, Ice.Future)
    test(f.result() == 1.0)

    f = p.opString1Async("opString1")
    assert isinstance(f, Ice.Future)
    test(f.result() == "opString1")

    f = p.opStringS1Async(None)  # type: ignore[reportArgumentType]
    assert isinstance(f, Ice.Future)
    test(len(f.result()) == 0)

    f = p.opByteBoolD1Async(None)  # type: ignore[reportArgumentType]
    assert isinstance(f, Ice.Future)
    test(len(f.result()) == 0)

    f = p.opStringS2Async(None)  # type: ignore[reportArgumentType]
    assert isinstance(f, Ice.Future)
    test(len(f.result()) == 0)

    f = p.opByteBoolD2Async(None)  # type: ignore[reportArgumentType]
    assert isinstance(f, Ice.Future)
    test(len(f.result()) == 0)
