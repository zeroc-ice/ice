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

def allTests(communicator):
    sys.stdout.write("testing stringToProxy... ")
    sys.stdout.flush()
    ref = "initial:default -p 12010"
    base = communicator.stringToProxy(ref)
    test(base)
    print("ok")

    sys.stdout.write("testing checked cast... ")
    sys.stdout.flush()
    initial = Test.InitialPrx.checkedCast(base)
    test(initial)
    test(initial == base)
    print("ok")

    sys.stdout.write("testing optional data members... ")
    sys.stdout.flush()

    oo1 = Test.OneOptional()
    test(oo1.a == Ice.Unset)
    oo1.a = 15

    oo2 = Test.OneOptional(16)
    test(oo2.a == 16)

    mo1 = Test.MultiOptional()
    test(mo1.a == Ice.Unset)
    test(mo1.b == Ice.Unset)
    test(mo1.c == Ice.Unset)
    test(mo1.d == Ice.Unset)
    test(mo1.e == Ice.Unset)
    test(mo1.f == Ice.Unset)
    test(mo1.g == Ice.Unset)
    test(mo1.h == Ice.Unset)
    test(mo1.i == Ice.Unset)
    test(mo1.j == Ice.Unset)
    test(mo1.k == Ice.Unset)
    test(mo1.bs == Ice.Unset)
    test(mo1.ss == Ice.Unset)
    test(mo1.iid == Ice.Unset)
    test(mo1.sid == Ice.Unset)
    test(mo1.fs == Ice.Unset)
    test(mo1.vs == Ice.Unset)

    test(mo1.shs == Ice.Unset)
    test(mo1.es == Ice.Unset)
    test(mo1.fss == Ice.Unset)
    test(mo1.vss == Ice.Unset)
    test(mo1.oos == Ice.Unset)
    test(mo1.oops == Ice.Unset)

    test(mo1.ied == Ice.Unset)
    test(mo1.ifsd == Ice.Unset)
    test(mo1.ivsd == Ice.Unset)
    test(mo1.iood == Ice.Unset)
    test(mo1.ioopd == Ice.Unset)

    test(mo1.bos == Ice.Unset)

    fs = Test.FixedStruct(78)
    vs = Test.VarStruct("hello")
    mo1 = Test.MultiOptional(15, True, 19, 78, 99, 5.5, 1.0, "test", Test.MyEnum.MyEnumMember, \
                             Test.MultiOptionalPrx.uncheckedCast(communicator.stringToProxy("test")), \
                             None, [5], ["test", "test2"], {4:3}, {"test":10}, fs, vs, [1], \
                             [Test.MyEnum.MyEnumMember, Test.MyEnum.MyEnumMember], \
                             [ fs ], [ vs ], [ oo1 ], \
                             [ Test.OneOptionalPrx.uncheckedCast(communicator.stringToProxy("test")) ], \
                             {4:Test.MyEnum.MyEnumMember}, {4:fs}, {5:vs}, {5:Test.OneOptional(15)}, \
                             {5:Test.OneOptionalPrx.uncheckedCast(communicator.stringToProxy("test"))}, \
                             [False, True, False])

    test(mo1.a == 15)
    test(mo1.b == True)
    test(mo1.c == 19)
    test(mo1.d == 78)
    test(mo1.e == 99)
    test(mo1.f == 5.5)
    test(mo1.g == 1.0)
    test(mo1.h == "test")
    test(mo1.i == Test.MyEnum.MyEnumMember)
    test(mo1.j == Test.MultiOptionalPrx.uncheckedCast(communicator.stringToProxy("test")))
    test(mo1.k == None)
    test(mo1.bs == [5])
    test(mo1.ss == ["test", "test2"])
    test(mo1.iid[4] == 3)
    test(mo1.sid["test"] == 10)
    test(mo1.fs == Test.FixedStruct(78))
    test(mo1.vs == Test.VarStruct("hello"))

    test(mo1.shs[0] == 1)
    test(mo1.es[0] == Test.MyEnum.MyEnumMember and mo1.es[1] == Test.MyEnum.MyEnumMember)
    test(mo1.fss[0] == Test.FixedStruct(78))
    test(mo1.vss[0] == Test.VarStruct("hello"))
    test(mo1.oos[0] == oo1)
    test(mo1.oops[0] == Test.OneOptionalPrx.uncheckedCast(communicator.stringToProxy("test")))

    test(mo1.ied[4] == Test.MyEnum.MyEnumMember)
    test(mo1.ifsd[4] == Test.FixedStruct(78))
    test(mo1.ivsd[5] == Test.VarStruct("hello"))
    test(mo1.iood[5].a == 15)
    test(mo1.ioopd[5] == Test.OneOptionalPrx.uncheckedCast(communicator.stringToProxy("test")))

    test(mo1.bos == [False, True, False])

    print("ok")

    sys.stdout.write("testing marshaling... ")
    sys.stdout.flush()

    oo4 = initial.pingPong(Test.OneOptional())
    test(oo4.a == Ice.Unset)

    oo5 = initial.pingPong(oo1)
    test(oo1.a == oo5.a)

    mo4 = initial.pingPong(Test.MultiOptional())
    test(mo4.a == Ice.Unset)
    test(mo4.b == Ice.Unset)
    test(mo4.c == Ice.Unset)
    test(mo4.d == Ice.Unset)
    test(mo4.e == Ice.Unset)
    test(mo4.f == Ice.Unset)
    test(mo4.g == Ice.Unset)
    test(mo4.h == Ice.Unset)
    test(mo4.i == Ice.Unset)
    test(mo4.j == Ice.Unset)
    test(mo4.k == Ice.Unset)
    test(mo4.bs == Ice.Unset)
    test(mo4.ss == Ice.Unset)
    test(mo4.iid == Ice.Unset)
    test(mo4.sid == Ice.Unset)
    test(mo4.fs == Ice.Unset)
    test(mo4.vs == Ice.Unset)

    test(mo4.shs == Ice.Unset)
    test(mo4.es == Ice.Unset)
    test(mo4.fss == Ice.Unset)
    test(mo4.vss == Ice.Unset)
    test(mo4.oos == Ice.Unset)
    test(mo4.oops == Ice.Unset)

    test(mo4.ied == Ice.Unset)
    test(mo4.ifsd == Ice.Unset)
    test(mo4.ivsd == Ice.Unset)
    test(mo4.iood == Ice.Unset)
    test(mo4.ioopd == Ice.Unset)

    test(mo4.bos == Ice.Unset)

    mo5 = initial.pingPong(mo1)
    test(mo5.a == mo1.a)
    test(mo5.b == mo1.b)
    test(mo5.c == mo1.c)
    test(mo5.d == mo1.d)
    test(mo5.e == mo1.e)
    test(mo5.f == mo1.f)
    test(mo5.g == mo1.g)
    test(mo5.h == mo1.h)
    test(mo5.i == mo1.i)
    test(mo5.j == mo1.j)
    test(mo5.k == None)
    if sys.version_info[0] == 2:
        test(mo5.bs == "\x05")
    else:
        test(mo5.bs[0] == 5)
    test(mo5.ss == mo1.ss)
    test(mo5.iid[4] == 3)
    test(mo5.sid["test"] == 10)
    test(mo5.fs == mo1.fs)
    test(mo5.vs == mo1.vs)
    test(mo5.shs == mo1.shs)
    test(mo5.es[0] == Test.MyEnum.MyEnumMember and mo1.es[1] == Test.MyEnum.MyEnumMember)
    test(mo5.fss[0] == Test.FixedStruct(78))
    test(mo5.vss[0] == Test.VarStruct("hello"))
    test(mo5.oos[0].a == 15)
    test(mo5.oops[0] == Test.OneOptionalPrx.uncheckedCast(communicator.stringToProxy("test")))

    test(mo5.ied[4] == Test.MyEnum.MyEnumMember)
    test(mo5.ifsd[4] == Test.FixedStruct(78))
    test(mo5.ivsd[5] == Test.VarStruct("hello"))
    test(mo5.iood[5].a == 15)
    test(mo5.ioopd[5] == Test.OneOptionalPrx.uncheckedCast(communicator.stringToProxy("test")))

    test(mo5.bos == mo1.bos)

    # Clear the first half of the optional members
    mo6 = Test.MultiOptional()
    mo6.b = mo5.b
    mo6.d = mo5.d
    mo6.f = mo5.f
    mo6.h = mo5.h
    mo6.j = mo5.j
    mo6.bs = mo5.bs
    mo6.iid = mo5.iid
    mo6.fs = mo5.fs
    mo6.shs = mo5.shs
    mo6.fss = mo5.fss
    mo6.oos = mo5.oos
    mo6.ifsd = mo5.ifsd
    mo6.iood = mo5.iood
    mo6.bos = mo5.bos

    mo7 = initial.pingPong(mo6)
    test(mo7.a == Ice.Unset)
    test(mo7.b == mo1.b)
    test(mo7.c == Ice.Unset)
    test(mo7.d == mo1.d)
    test(mo7.e == Ice.Unset)
    test(mo7.f == mo1.f)
    test(mo7.g == Ice.Unset)
    test(mo7.h == mo1.h)
    test(mo7.i == Ice.Unset)
    test(mo7.j == mo1.j)
    test(mo7.k == Ice.Unset)
    if sys.version_info[0] == 2:
        test(mo7.bs == "\x05")
    else:
        test(mo7.bs[0] == 5)
    test(mo7.ss == Ice.Unset)
    test(mo7.iid[4] == 3)
    test(mo7.sid == Ice.Unset)
    test(mo7.fs == mo1.fs)
    test(mo7.vs == Ice.Unset)

    test(mo7.shs == mo1.shs)
    test(mo7.es == Ice.Unset)
    test(mo7.fss[0] == Test.FixedStruct(78))
    test(mo7.vss == Ice.Unset)
    test(mo7.oos[0].a == 15)
    test(mo7.oops == Ice.Unset)

    test(mo7.ied == Ice.Unset)
    test(mo7.ifsd[4] == Test.FixedStruct(78))
    test(mo7.ivsd == Ice.Unset)
    test(mo7.iood[5].a == 15)
    test(mo7.ioopd == Ice.Unset)

    test(mo7.bos == [False, True, False])

    # Clear the second half of the optional members
    mo8 = Test.MultiOptional()
    mo8.a = mo5.a
    mo8.c = mo5.c
    mo8.e = mo5.e
    mo8.g = mo5.g
    mo8.i = mo5.i
    mo8.k = mo8
    mo8.ss = mo5.ss
    mo8.sid = mo5.sid
    mo8.vs = mo5.vs

    mo8.es = mo5.es
    mo8.vss = mo5.vss
    mo8.oops = mo5.oops

    mo8.ied = mo5.ied
    mo8.ivsd = mo5.ivsd
    mo8.ioopd = mo5.ioopd

    mo9 = initial.pingPong(mo8)
    test(mo9.a == mo1.a)
    test(mo9.b == Ice.Unset)
    test(mo9.c == mo1.c)
    test(mo9.d == Ice.Unset)
    test(mo9.e == mo1.e)
    test(mo9.f == Ice.Unset)
    test(mo9.g == mo1.g)
    test(mo9.h == Ice.Unset)
    test(mo9.i == mo1.i)
    test(mo9.j == Ice.Unset)
    test(mo9.k == mo9)
    test(mo9.bs == Ice.Unset)
    test(mo9.ss == mo1.ss)
    test(mo9.iid == Ice.Unset)
    test(mo9.sid["test"] == 10)
    test(mo9.fs == Ice.Unset)
    test(mo9.vs == mo1.vs)

    test(mo9.shs == Ice.Unset)
    test(mo9.es[0] == Test.MyEnum.MyEnumMember and mo1.es[1] == Test.MyEnum.MyEnumMember)
    test(mo9.fss == Ice.Unset)
    test(mo9.vss[0] == Test.VarStruct("hello"))
    test(mo9.oos == Ice.Unset)
    test(mo9.oops[0] == Test.OneOptionalPrx.uncheckedCast(communicator.stringToProxy("test")))

    test(mo9.ied[4] == Test.MyEnum.MyEnumMember)
    test(mo9.ifsd == Ice.Unset)
    test(mo9.ivsd[5] == Test.VarStruct("hello"))
    test(mo9.iood == Ice.Unset)
    test(mo9.ioopd[5] == Test.OneOptionalPrx.uncheckedCast(communicator.stringToProxy("test")))

    test(mo9.bos == Ice.Unset)

    #
    # Use the 1.0 encoding with operations whose only class parameters are optional.
    #
    initial.sendOptionalClass(True, Test.OneOptional(53))
    initial.ice_encodingVersion(Ice.Encoding_1_0).sendOptionalClass(True, Test.OneOptional(53))

    r = initial.returnOptionalClass(True)
    test(r != Ice.Unset)
    r = initial.ice_encodingVersion(Ice.Encoding_1_0).returnOptionalClass(True)
    test(r == Ice.Unset)

    print("ok")

    sys.stdout.write("testing marshaling of large containers with fixed size elements... ")
    sys.stdout.flush()

    mc = Test.MultiOptional()

    mc.bs = []
    for i in range(1000):
        mc.bs.append(0)
    mc.shs = []
    for i in range(300):
        mc.shs.append(0)

    mc.fss = []
    for i in range(300):
        mc.fss.append(Test.FixedStruct())

    mc.ifsd = {}
    for i in range(300):
        mc.ifsd[i] = Test.FixedStruct()

    mc = initial.pingPong(mc)
    test(len(mc.bs) == 1000)
    test(len(mc.shs) == 300)
    test(len(mc.fss) == 300)
    test(len(mc.ifsd) == 300)

    initial2 = Test.Initial2Prx.uncheckedCast(base)
    initial2.opVoid(15, "test")

    print("ok")

    sys.stdout.write("testing tag marshaling... ")
    sys.stdout.flush()

    b = Test.B()
    b2 = initial.pingPong(b)
    test(b2.ma == Ice.Unset)
    test(b2.mb == Ice.Unset)
    test(b2.mc == Ice.Unset)

    b.ma = 10
    b.mb = 11
    b.mc = 12
    b.md = 13

    b2 = initial.pingPong(b)
    test(b2.ma == 10)
    test(b2.mb == 11)
    test(b2.mc == 12)
    test(b2.md == 13)

    print("ok")

    sys.stdout.write("testing marshalling of objects with optional objects...")
    sys.stdout.flush()

    f = Test.F()
    
    f.af = Test.A()
    f.ae = f.af
    
    rf = initial.pingPong(f)
    test(rf.ae == rf.af)
    
    print("ok")

    sys.stdout.write("testing optional with default values... ")
    sys.stdout.flush()

    wd = initial.pingPong(Test.WD())
    test(wd.a == 5)
    test(wd.s == "test")
    wd.a = Ice.Unset
    wd.s = Ice.Unset
    wd = initial.pingPong(wd)
    test(wd.a == Ice.Unset)
    test(wd.s == Ice.Unset)

    print("ok")

    if communicator.getProperties().getPropertyAsInt("Ice.Default.SlicedFormat") > 0:
        sys.stdout.write("testing marshaling with unknown class slices... ")
        sys.stdout.flush()

        c = Test.C()
        c.ss = "test"
        c.ms = "testms"
        c = initial.pingPong(c)
        test(c.ma == Ice.Unset)
        test(c.mb == Ice.Unset)
        test(c.mc == Ice.Unset)
        test(c.md == Ice.Unset)
        test(c.ss == "test")
        test(c.ms == "testms")

        print("ok")

        sys.stdout.write("testing optionals with unknown classes... ")
        sys.stdout.flush()

        initial2 = Test.Initial2Prx.uncheckedCast(base)
        d = Test.D()
        d.ds = "test"
        d.seq = ["test1", "test2", "test3", "test4"]
        d.ao = Test.A(18)
        d.requiredB = 14;
        d.requiredA = 14;
        initial2.opClassAndUnknownOptional(Test.A(), d)

        print("ok")

    sys.stdout.write("testing optional parameters... ")
    sys.stdout.flush()

    (p2, p3) = initial.opByte(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    (p2, p3) = initial.opByte(56)
    test(p2 == 56 and p3 == 56)
    r = initial.begin_opByte(56)
    (p2, p3) = initial.end_opByte(r)
    test(p2 == 56 and p3 == 56)

    (p2, p3) = initial.opBool(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    (p2, p3) = initial.opBool(True)
    test(p2 == True and p3 == True)
    r = initial.begin_opBool(True)
    (p2, p3) = initial.end_opBool(r)
    test(p2 == True and p3 == True)

    (p2, p3) = initial.opShort(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    (p2, p3) = initial.opShort(56)
    test(p2 == 56 and p3 == 56)
    r = initial.begin_opShort(56)
    (p2, p3) = initial.end_opShort(r)
    test(p2 == 56 and p3 == 56)

    (p2, p3) = initial.opInt(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    (p2, p3) = initial.opInt(56)
    test(p2 == 56 and p3 == 56)
    r = initial.begin_opInt(56)
    (p2, p3) = initial.end_opInt(r)
    test(p2 == 56 and p3 == 56)

    (p2, p3) = initial.opLong(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    (p2, p3) = initial.opLong(56)
    test(p2 == 56 and p3 == 56)
    r = initial.begin_opLong(56)
    (p2, p3) = initial.end_opLong(r)
    test(p2 == 56 and p3 == 56)

    (p2, p3) = initial.opFloat(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    (p2, p3) = initial.opFloat(1.0)
    test(p2 == 1.0 and p3 == 1.0)
    r = initial.begin_opFloat(1.0)
    (p2, p3) = initial.end_opFloat(r)
    test(p2 == 1.0 and p3 == 1.0)

    (p2, p3) = initial.opDouble(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    (p2, p3) = initial.opDouble(1.0)
    test(p2 == 1.0 and p3 == 1.0)
    r = initial.begin_opDouble(1.0)
    (p2, p3) = initial.end_opDouble(r)
    test(p2 == 1.0 and p3 == 1.0)

    (p2, p3) = initial.opString(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    (p2, p3) = initial.opString("test")
    test(p2 == "test" and p3 == "test")
    r = initial.begin_opString("test")
    (p2, p3) = initial.end_opString(r)
    test(p2 == "test" and p3 == "test")

    (p2, p3) = initial.opMyEnum(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    (p2, p3) = initial.opMyEnum(Test.MyEnum.MyEnumMember)
    test(p2 == Test.MyEnum.MyEnumMember and p3 == Test.MyEnum.MyEnumMember)
    r = initial.begin_opMyEnum(Test.MyEnum.MyEnumMember)
    (p2, p3) = initial.end_opMyEnum(r)
    test(p2 == Test.MyEnum.MyEnumMember and p3 == Test.MyEnum.MyEnumMember)

    (p2, p3) = initial.opSmallStruct(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    p1 = Test.SmallStruct(56)
    (p2, p3) = initial.opSmallStruct(p1)
    test(p2 == p1 and p3 == p1)
    r = initial.begin_opSmallStruct(p1)
    (p2, p3) = initial.end_opSmallStruct(r)
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opFixedStruct(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    p1 = Test.FixedStruct(56)
    (p2, p3) = initial.opFixedStruct(p1)
    test(p2 == p1 and p3 == p1)
    r = initial.begin_opFixedStruct(p1)
    (p2, p3) = initial.end_opFixedStruct(r)
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opVarStruct(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    p1 = Test.VarStruct("test")
    (p2, p3) = initial.opVarStruct(p1)
    test(p2 == p1 and p3 == p1)
    r = initial.begin_opVarStruct(p1)
    (p2, p3) = initial.end_opVarStruct(r)
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opOneOptional(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    p1 = Test.OneOptional(58)
    (p2, p3) = initial.opOneOptional(p1)
    test(p2.a == p1.a and p3.a == p1.a)
    r = initial.begin_opOneOptional(p1)
    (p2, p3) = initial.end_opOneOptional(r)
    test(p2.a == p1.a and p3.a == p1.a)

    (p2, p3) = initial.opOneOptionalProxy(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    p1 = Test.OneOptionalPrx.uncheckedCast(communicator.stringToProxy("test"))
    (p2, p3) = initial.opOneOptionalProxy(p1)
    test(p2 == p1 and p3 == p1)
    r = initial.begin_opOneOptionalProxy(p1)
    (p2, p3) = initial.end_opOneOptionalProxy(r)
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opByteSeq(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    p1 = [56 for x in range(100)]
    (p2, p3) = initial.opByteSeq(p1)
    test(len(p2) == len(p1) and len(p3) == len(p1))
    if sys.version_info[0] == 2:
        test(p2[0] == '\x38')
        test(p3[0] == '\x38')
    else:
        test(p2[0] == 0x38)
        test(p3[0] == 0x38)
    r = initial.begin_opByteSeq(p1)
    (p2, p3) = initial.end_opByteSeq(r)
    test(len(p2) == len(p1) and len(p3) == len(p1))
    if sys.version_info[0] == 2:
        test(p2[0] == '\x38')
        test(p3[0] == '\x38')
    else:
        test(p2[0] == 0x38)
        test(p3[0] == 0x38)

    (p2, p3) = initial.opBoolSeq(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    p1 = [True for x in range(100)]
    (p2, p3) = initial.opBoolSeq(p1)
    test(p2 == p1 and p3 == p1)
    r = initial.begin_opBoolSeq(p1)
    (p2, p3) = initial.end_opBoolSeq(r)
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opShortSeq(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    p1 = [56 for x in range(100)]
    (p2, p3) = initial.opShortSeq(p1)
    test(p2 == p1 and p3 == p1)
    r = initial.begin_opShortSeq(p1)
    (p2, p3) = initial.end_opShortSeq(r)
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opIntSeq(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    p1 = [56 for x in range(100)]
    (p2, p3) = initial.opIntSeq(p1)
    test(p2 == p1 and p3 == p1)
    r = initial.begin_opIntSeq(p1)
    (p2, p3) = initial.end_opIntSeq(r)
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opLongSeq(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    p1 = [56 for x in range(100)]
    (p2, p3) = initial.opLongSeq(p1)
    test(p2 == p1 and p3 == p1)
    r = initial.begin_opLongSeq(p1)
    (p2, p3) = initial.end_opLongSeq(r)
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opFloatSeq(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    p1 = [1.0 for x in range(100)]
    (p2, p3) = initial.opFloatSeq(p1)
    test(p2 == p1 and p3 == p1)
    r = initial.begin_opFloatSeq(p1)
    (p2, p3) = initial.end_opFloatSeq(r)
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opDoubleSeq(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    p1 = [1.0 for x in range(100)]
    (p2, p3) = initial.opDoubleSeq(p1)
    test(p2 == p1 and p3 == p1)
    r = initial.begin_opDoubleSeq(p1)
    (p2, p3) = initial.end_opDoubleSeq(r)
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opStringSeq(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    p1 = ["test1" for x in range(100)]
    (p2, p3) = initial.opStringSeq(p1)
    test(p2 == p1 and p3 == p1)
    r = initial.begin_opStringSeq(p1)
    (p2, p3) = initial.end_opStringSeq(r)
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opSmallStructSeq(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    p1 = [Test.SmallStruct(1) for x in range(10)]
    (p2, p3) = initial.opSmallStructSeq(p1)
    test(p2 == p1 and p3 == p1)
    r = initial.begin_opSmallStructSeq(p1)
    (p2, p3) = initial.end_opSmallStructSeq(r)
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opSmallStructList(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    p1 = tuple([Test.SmallStruct(1) for x in range(10)])
    (p2, p3) = initial.opSmallStructList(p1)
    test(p2 == p1 and p3 == p1)
    r = initial.begin_opSmallStructList(p1)
    (p2, p3) = initial.end_opSmallStructList(r)
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opFixedStructSeq(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    p1 = [Test.FixedStruct(1) for x in range(10)]
    (p2, p3) = initial.opFixedStructSeq(p1)
    test(p2 == p1 and p3 == p1)
    r = initial.begin_opFixedStructSeq(p1)
    (p2, p3) = initial.end_opFixedStructSeq(r)
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opFixedStructList(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    p1 = tuple([Test.FixedStruct(1) for x in range(10)])
    (p2, p3) = initial.opFixedStructList(p1)
    test(p2 == p1 and p3 == p1)
    r = initial.begin_opFixedStructList(p1)
    (p2, p3) = initial.end_opFixedStructList(r)
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opVarStructSeq(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    p1 = [Test.VarStruct("test") for x in range(10)]
    (p2, p3) = initial.opVarStructSeq(p1)
    test(p2 == p1 and p3 == p1)
    r = initial.begin_opVarStructSeq(p1)
    (p2, p3) = initial.end_opVarStructSeq(r)
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opIntIntDict(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    p1 = {1:2, 2:3}
    (p2, p3) = initial.opIntIntDict(p1)
    test(p2 == p1 and p3 == p1)
    r = initial.begin_opIntIntDict(p1)
    (p2, p3) = initial.end_opIntIntDict(r)
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opStringIntDict(Ice.Unset)
    test(p2 == Ice.Unset and p3 == Ice.Unset)
    p1 = {"1":2, "2":3}
    (p2, p3) = initial.opStringIntDict(p1)
    test(p2 == p1 and p3 == p1)
    r = initial.begin_opStringIntDict(p1)
    (p2, p3) = initial.end_opStringIntDict(r)
    test(p2 == p1 and p3 == p1)

    print("ok")

    sys.stdout.write("testing exception optionals... ")
    sys.stdout.flush()

    try:
        initial.opOptionalException(Ice.Unset, Ice.Unset, Ice.Unset)
    except Test.OptionalException as ex:
        test(ex.a == Ice.Unset)
        test(ex.b == Ice.Unset)
        test(ex.o == Ice.Unset)

    try:
        initial.opOptionalException(30, "test", Test.OneOptional(53))
    except Test.OptionalException as ex:
        test(ex.a == 30)
        test(ex.b == "test")
        test(ex.o.a == 53)

    try:
        #
        # Use the 1.0 encoding with an exception whose only class members are optional.
        #
        initial.ice_encodingVersion(Ice.Encoding_1_0).opOptionalException(30, "test", Test.OneOptional(53))
    except Test.OptionalException as ex:
        test(ex.a == Ice.Unset)
        test(ex.b == Ice.Unset)
        test(ex.o == Ice.Unset)

    try:
        initial.opDerivedException(Ice.Unset, Ice.Unset, Ice.Unset)
    except Test.DerivedException as ex:
        test(ex.a == Ice.Unset)
        test(ex.b == Ice.Unset)
        test(ex.o == Ice.Unset)
        test(ex.ss == Ice.Unset)
        test(ex.o2 == Ice.Unset)

    try:
        initial.opDerivedException(30, "test2", Test.OneOptional(53))
    except Test.DerivedException as ex:
        test(ex.a == 30)
        test(ex.b == "test2")
        test(ex.o.a == 53)
        test(ex.ss == "test2")
        test(ex.o2 == ex.o)

    try:
        initial.opRequiredException(Ice.Unset, Ice.Unset, Ice.Unset)
    except Test.RequiredException as ex:
        test(ex.a == Ice.Unset)
        test(ex.b == Ice.Unset)
        test(ex.o == Ice.Unset)
        test(ex.ss == "test")
        test(ex.o2 == None)

    try:
        initial.opRequiredException(30, "test2", Test.OneOptional(53))
    except Test.RequiredException as ex:
        test(ex.a == 30)
        test(ex.b == "test2")
        test(ex.o.a == 53)
        test(ex.ss == "test2")
        test(ex.o2 == ex.o)

    print("ok")

    return initial
