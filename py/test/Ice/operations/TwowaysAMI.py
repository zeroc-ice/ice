# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

class AMI_MyClass_opVoidExI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        test(isinstance(ex, Ice.NoEndpointException))
        self.called()

class AMI_MyClass_opByteI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, r, b):
        test(b == 0xf0)
        test(r == 0xff)
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opByteExI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, r, b):
        test(False)

    def ice_exception(self, ex):
        test(isinstance(ex, Ice.NoEndpointException))
        self.called()

class AMI_MyClass_opBoolI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, r, b):
        test(b)
        test(not r)
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opShortIntLongI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, r, s, i, l):
        test(s == 10)
        test(i == 11)
        test(l == 12)
        test(r == 12)
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opFloatDoubleI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, r, f, d):
        test(f - 3.14 < 0.001)
        test(d == 1.1E10)
        test(r == 1.1E10)
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opStringI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, r, s):
        test(s == "world hello")
        test(r == "hello world")
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opMyEnumI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, r, e):
        test(e == Test.MyEnum.enum2)
        test(r == Test.MyEnum.enum3)
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opMyClassI(CallbackBase):
    def __init__(self, communicator):
        CallbackBase.__init__(self)
        self._communicator = communicator

    def ice_response(self, r, c1, c2):
        test(c1.ice_getIdentity() == self._communicator.stringToIdentity("test"))
        test(c2.ice_getIdentity() == self._communicator.stringToIdentity("noSuchIdentity"))
        test(r.ice_getIdentity() == self._communicator.stringToIdentity("test"))
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

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opStructI(CallbackBase):

    def __init__(self, communicator):
        CallbackBase.__init__(self)
        self._communicator = communicator

    def ice_response(self, rso, so):
        test(rso.p == None)
        test(rso.e == Test.MyEnum.enum2)
        test(rso.s.s == "def")
        test(so.e == Test.MyEnum.enum3)
        test(so.s.s == "a new string")
        # We can't do the callbacks below in serialize mode.
        if self._communicator.getProperties().getPropertyAsInt("Ice.ThreadPool.Client.Serialize") == 0:
            so.p.opVoid()
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opByteSI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, rso, bso):
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

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opBoolSI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, rso, bso):
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

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opShortIntLongSI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, rso, sso, iso, lso):
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

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opFloatDoubleSI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, rso, fso, dso):
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

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opStringSI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, rso, sso):
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

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opByteSSI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, rso, bso):
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

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opFloatDoubleSSI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, rso, fso, dso):
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

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opStringSSI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, rso, sso):
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

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opByteBoolDI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, ro, do):
        di1 = {10: True, 100: False}
        test(do == di1)
        test(len(ro) == 4)
        test(ro[10])
        test(not ro[11])
        test(not ro[100])
        test(ro[101])
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opShortIntDI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, ro, do):
        di1 = {110: -1, 1100: 123123}
        test(do == di1)
        test(len(ro) == 4)
        test(ro[110] == -1)
        test(ro[111] == -100)
        test(ro[1100] == 123123)
        test(ro[1101] == 0)
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opLongFloatDI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, ro, do):
        di1 = {999999110: -1.1, 999999111: 123123.2}
        for k in do:
            test(math.fabs(do[k] - di1[k]) < 0.01)
        test(len(ro) == 4)
        test(ro[999999110] - -1.1 < 0.01)
        test(ro[999999120] - -100.4 < 0.01)
        test(ro[999999111] - 123123.2 < 0.01)
        test(ro[999999130] - 0.5 < 0.01)
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opStringStringDI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, ro, do):
        di1 = {'foo': 'abc -1.1', 'bar': 'abc 123123.2'}
        test(do == di1)
        test(len(ro) == 4)
        test(ro["foo"] == "abc -1.1")
        test(ro["FOO"] == "abc -100.4")
        test(ro["bar"] == "abc 123123.2")
        test(ro["BAR"] == "abc 0.5")
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opStringMyEnumDI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, ro, do):
        di1 = {'abc': Test.MyEnum.enum1, '': Test.MyEnum.enum2}
        test(do == di1)
        test(len(ro) == 4)
        test(ro["abc"] == Test.MyEnum.enum1)
        test(ro["qwerty"] == Test.MyEnum.enum3)
        test(ro[""] == Test.MyEnum.enum2)
        test(ro["Hello!!"] == Test.MyEnum.enum2)
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opMyEnumStringDI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, ro, do):
        di1 = {Test.MyEnum.enum1: 'abc'}
        test(do == di1)
        test(len(ro) == 3)
        test(ro[Test.MyEnum.enum1] == "abc")
        test(ro[Test.MyEnum.enum2] == "Hello!!")
        test(ro[Test.MyEnum.enum3] == "qwerty")
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opMyStructMyEnumDI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self, ro, do):
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

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opIntSI(CallbackBase):
    def __init__(self, l):
        CallbackBase.__init__(self)
        self._l = l

    def ice_response(self, r):
        test(len(r) == self._l)
        for j in range(0, self._l):
            test(r[j] == -j)
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opContextEqualI(CallbackBase):
    def __init__(self, d):
        CallbackBase.__init__(self)
        self._d = d

    def ice_response(self, r):
        test(r == self._d)
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMI_MyClass_opContextNotEqualI(CallbackBase):
    def __init__(self, d):
        CallbackBase.__init__(self)
        self._d = d

    def ice_response(self, r):
        test(r != self._d)
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

class AMI_MyDerivedClass_opDerivedI(CallbackBase):
    def __init__(self):
        CallbackBase.__init__(self)

    def ice_response(self):
        self.called()

    def ice_exception(self, ex):
        test(False)

def twowaysAMI(communicator, p):
    # Check that a call to a void operation raises NoEndpointException
    # in the ice_exception() callback instead of at the point of call.
    indirect = Test.MyClassPrx.uncheckedCast(p.ice_adapterId("dummy"))
    cb = AMI_MyClass_opVoidExI()
    try:
        test(not indirect.opVoid_async(cb))
    except Ice.Exception:
        test(False)
    cb.check()

    # Check that a call to a twoway operation raises NoEndpointException
    # in the ice_exception() callback instead of at the point of call.
    indirect = Test.MyClassPrx.uncheckedCast(p.ice_adapterId("dummy"))
    cb = AMI_MyClass_opByteExI()
    try:
        test(not indirect.opByte_async(cb, 0, 0))
    except Ice.Exception:
        test(False)
    cb.check()

    #
    # opVoid
    #
    cb = AMI_MyClass_opVoidI()
    p.opVoid_async(cb)
    cb.check()
    # Let's check if we can reuse the same callback object for another call.
    p.opVoid_async(cb)
    cb.check()

    # Check that CommunicatorDestroyedException is raised directly.
    initData = Ice.InitializationData()
    initData.properties = communicator.getProperties().clone()
    ic = Ice.initialize(initData)
    obj = ic.stringToProxy(p.ice_toString())
    p2 = Test.MyClassPrx.checkedCast(obj)

    ic.destroy()

    cb = AMI_MyClass_opVoidI()
    try:
        test(not p2.opVoid_async(cb))
        test(False)
    except Ice.CommunicatorDestroyedException:
        pass # Expected.

    #
    # opByte
    #
    cb = AMI_MyClass_opByteI()
    p.opByte_async(cb, 0xff, 0x0f)
    cb.check()

    #
    # opBool
    #
    cb = AMI_MyClass_opBoolI()
    p.opBool_async(cb, True, False)
    cb.check()

    #
    # opShortIntLong
    #
    cb = AMI_MyClass_opShortIntLongI()
    p.opShortIntLong_async(cb, 10, 11, 12)
    cb.check()

    #
    # opFloatDouble
    #
    cb = AMI_MyClass_opFloatDoubleI()
    p.opFloatDouble_async(cb, 3.14, 1.1E10)
    cb.check()
    # Let's check if we can reuse the same callback object for another call.
    p.opFloatDouble_async(cb, 3.14, 1.1E10)
    cb.check()

    #
    # opString
    #
    cb = AMI_MyClass_opStringI()
    p.opString_async(cb, "hello", "world")
    cb.check()

    #
    # opMyEnum
    #
    cb = AMI_MyClass_opMyEnumI()
    p.opMyEnum_async(cb, Test.MyEnum.enum2)
    cb.check()

    #
    # opMyClass
    #
    cb = AMI_MyClass_opMyClassI(communicator)
    p.opMyClass_async(cb, p)
    cb.check()

    #
    # opStruct
    #
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

    cb = AMI_MyClass_opStructI(communicator)
    p.opStruct_async(cb, si1, si2)
    cb.check()

    #
    # opByteS
    #
    bsi1 = (0x01, 0x11, 0x12, 0x22)
    bsi2 = (0xf1, 0xf2, 0xf3, 0xf4)

    cb = AMI_MyClass_opByteSI()
    p.opByteS_async(cb, bsi1, bsi2)
    cb.check()

    #
    # opBoolS
    #
    bsi1 = (True, True, False)
    bsi2 = (False,)

    cb = AMI_MyClass_opBoolSI()
    p.opBoolS_async(cb, bsi1, bsi2)
    cb.check()

    #
    # opShortIntLongS
    #
    ssi = (1, 2, 3)
    isi = (5, 6, 7, 8)
    lsi = (10, 30, 20)

    cb = AMI_MyClass_opShortIntLongSI()
    p.opShortIntLongS_async(cb, ssi, isi, lsi)
    cb.check()

    #
    # opFloatDoubleS
    #
    fsi = (3.14, 1.11)
    dsi = (1.1E10, 1.2E10, 1.3E10)

    cb = AMI_MyClass_opFloatDoubleSI()
    p.opFloatDoubleS_async(cb, fsi, dsi)
    cb.check()

    #
    # opStringS
    #
    ssi1 = ('abc', 'de', 'fghi')
    ssi2 = ('xyz',)

    cb = AMI_MyClass_opStringSI()
    p.opStringS_async(cb, ssi1, ssi2)
    cb.check()

    #
    # opByteSS
    #
    bsi1 = ((0x01, 0x11, 0x12), (0xff,))
    bsi2 = ((0x0e,), (0xf2, 0xf1))

    cb = AMI_MyClass_opByteSSI()
    p.opByteSS_async(cb, bsi1, bsi2)
    cb.check()

    #
    # opFloatDoubleSS
    #
    fsi = ((3.14,), (1.11,), ())
    dsi = ((1.1E10, 1.2E10, 1.3E10),)

    cb = AMI_MyClass_opFloatDoubleSSI()
    p.opFloatDoubleSS_async(cb, fsi, dsi)
    cb.check()

    #
    # opStringSS
    #
    ssi1 = (('abc',), ('de', 'fghi'))
    ssi2 = ((), (), ('xyz',))

    cb = AMI_MyClass_opStringSSI()
    p.opStringSS_async(cb, ssi1, ssi2)
    cb.check()

    #
    # opByteBoolD
    #
    di1 = {10: True, 100: False}
    di2 = {10: True, 11: False, 101: True}

    cb = AMI_MyClass_opByteBoolDI()
    p.opByteBoolD_async(cb, di1, di2)
    cb.check()

    #
    # opShortIntD
    #
    di1 = {110: -1, 1100: 123123}
    di2 = {110: -1, 111: -100, 1101: 0}

    cb = AMI_MyClass_opShortIntDI()
    p.opShortIntD_async(cb, di1, di2)
    cb.check()

    #
    # opLongFloatD
    #
    di1 = {999999110: -1.1, 999999111: 123123.2}
    di2 = {999999110: -1.1, 999999120: -100.4, 999999130: 0.5}

    cb = AMI_MyClass_opLongFloatDI()
    p.opLongFloatD_async(cb, di1, di2)
    cb.check()

    #
    # opStringStringD
    #
    di1 = {'foo': 'abc -1.1', 'bar': 'abc 123123.2'}
    di2 = {'foo': 'abc -1.1', 'FOO': 'abc -100.4', 'BAR': 'abc 0.5'}

    cb = AMI_MyClass_opStringStringDI()
    p.opStringStringD_async(cb, di1, di2)
    cb.check()

    #
    # opStringMyEnumD
    #
    di1 = {'abc': Test.MyEnum.enum1, '': Test.MyEnum.enum2}
    di2 = {'abc': Test.MyEnum.enum1, 'qwerty': Test.MyEnum.enum3, 'Hello!!': Test.MyEnum.enum2}

    cb = AMI_MyClass_opStringMyEnumDI()
    p.opStringMyEnumD_async(cb, di1, di2)
    cb.check()

    #
    # opMyEnumStringD
    #
    di1 = {Test.MyEnum.enum1: 'abc'}
    di2 = {Test.MyEnum.enum2: 'Hello!!', Test.MyEnum.enum3: 'qwerty'}

    cb = AMI_MyClass_opMyEnumStringDI()
    p.opMyEnumStringD_async(cb, di1, di2)
    cb.check()

    #
    # opMyStructMyEnumD
    #
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

    cb = AMI_MyClass_opMyStructMyEnumDI()
    p.opMyStructMyEnumD_async(cb, di1, di2)
    cb.check()

    #
    # opIntS
    #
    lengths = ( 0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000 )
    for l in lengths:
        s = []
        for i in range(l):
            s.append(i)
        cb = AMI_MyClass_opIntSI(l)
        p.opIntS_async(cb, s)
        cb.check()

    #
    # opContext
    #
    ctx = {'one': 'ONE', 'two': 'TWO', 'three': 'THREE'}

    test(len(p.ice_getContext()) == 0)
    cb = AMI_MyClass_opContextNotEqualI(ctx)
    p.opContext_async(cb)
    cb.check()

    test(len(p.ice_getContext()) == 0)
    cb = AMI_MyClass_opContextEqualI(ctx)
    p.opContext_async(cb, ctx)
    cb.check()

    p2 = Test.MyClassPrx.checkedCast(p.ice_context(ctx))
    test(p2.ice_getContext() == ctx)
    cb = AMI_MyClass_opContextEqualI(ctx)
    p2.opContext_async(cb)
    cb.check()

    p2 = Test.MyClassPrx.checkedCast(p.ice_context(ctx))
    test(p2.ice_getContext() == ctx)
    cb = AMI_MyClass_opContextEqualI(ctx)
    p2.opContext_async(cb, ctx)
    cb.check()

    #
    # Test implicit context propagation
    #
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

        cb = AMI_MyClass_opContextEqualI(ctx)
        p3.opContext_async(cb)
        cb.check()

        ic.getImplicitContext().put('zero', 'ZERO')
        ctx = ic.getImplicitContext().getContext()

        cb = AMI_MyClass_opContextEqualI(ctx)
        p3.opContext_async(cb)
        cb.check()

        prxContext = {'one': 'UN', 'four': 'QUATRE'}

        combined = ctx
        combined.update(prxContext)
        test(combined['one'] == 'UN')

        p3 = Test.MyClassPrx.uncheckedCast(p3.ice_context(prxContext))
        ic.getImplicitContext().setContext({})

        cb = AMI_MyClass_opContextEqualI(prxContext)
        p3.opContext_async(cb)
        cb.check()

        ic.getImplicitContext().setContext(ctx)

        cb = AMI_MyClass_opContextEqualI(combined)
        p3.opContext_async(cb)
        cb.check()

        ic.destroy()

    #
    # opIdempotent
    #
    cb = AMI_MyClass_opIdempotentI()
    p.opIdempotent_async(cb)
    cb.check()

    #
    # opNonmutating
    #
    cb = AMI_MyClass_opNonmutatingI()
    p.opNonmutating_async(cb)
    cb.check()

    derived = Test.MyDerivedClassPrx.checkedCast(p)
    test(derived)
    cb = AMI_MyDerivedClass_opDerivedI()
    derived.opDerived_async(cb)
    cb.check()
