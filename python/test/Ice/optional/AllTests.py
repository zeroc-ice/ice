# Copyright (c) ZeroC, Inc.

import sys

import Test

import Ice


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


def allTests(helper, communicator):
    initial = Test.InitialPrx(communicator, f"initial:{helper.getTestEndpoint()}")

    sys.stdout.write("testing optional data members... ")
    sys.stdout.flush()

    oo1 = Test.OneOptional()
    test(oo1.a is None)
    oo1.a = 15

    oo2 = Test.OneOptional(16)
    test(oo2.a == 16)

    mo1 = Test.MultiOptional()
    test(mo1.a is None)
    test(mo1.b is None)
    test(mo1.c is None)
    test(mo1.d is None)
    test(mo1.e is None)
    test(mo1.f is None)
    test(mo1.g is None)
    test(mo1.h is None)
    test(mo1.i is None)
    test(mo1.j is None)
    test(mo1.bs is None)
    test(mo1.ss is None)
    test(mo1.iid is None)
    test(mo1.sid is None)
    test(mo1.fs is None)
    test(mo1.vs is None)

    test(mo1.shs is None)
    test(mo1.es is None)
    test(mo1.fss is None)
    test(mo1.vss is None)
    test(mo1.mips is None)

    test(mo1.ied is None)
    test(mo1.ifsd is None)
    test(mo1.ivsd is None)
    test(mo1.imipd is None)

    test(mo1.bos is None)

    ss = Test.SmallStruct()
    fs = Test.FixedStruct(78)
    vs = Test.VarStruct("hello")
    mo1 = Test.MultiOptional(
        15,
        True,
        19,
        78,
        99,
        5.5,
        1.0,
        "test",
        Test.MyEnum.MyEnumMember,
        Test.MyInterfacePrx(communicator, "test"),
        [5],
        ["test", "test2"],
        {4: 3},
        {"test": 10},
        fs,
        vs,
        [1],
        [Test.MyEnum.MyEnumMember, Test.MyEnum.MyEnumMember],
        [fs],
        [vs],
        [Test.MyInterfacePrx(communicator, "test")],
        {4: Test.MyEnum.MyEnumMember},
        {4: fs},
        {5: vs},
        {5: Test.MyInterfacePrx(communicator, "test")},
        [False, True, False],
    )

    test(mo1.a == 15)
    test(mo1.b is True)
    test(mo1.c == 19)
    test(mo1.d == 78)
    test(mo1.e == 99)
    test(mo1.f == 5.5)
    test(mo1.g == 1.0)
    test(mo1.h == "test")
    test(mo1.i == Test.MyEnum.MyEnumMember)
    test(mo1.j == Test.MyInterfacePrx(communicator, "test"))
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
    test(mo1.mips[0] == Test.MyInterfacePrx(communicator, "test"))

    test(mo1.ied[4] == Test.MyEnum.MyEnumMember)
    test(mo1.ifsd[4] == Test.FixedStruct(78))
    test(mo1.ivsd[5] == Test.VarStruct("hello"))
    test(mo1.imipd[5] == Test.MyInterfacePrx(communicator, "test"))

    test(mo1.bos == [False, True, False])

    #
    # Test generated struct and classes compare with None
    #
    test(ss is not None)
    test(fs is not None)
    test(vs is not None)
    test(mo1 is not None)

    print("ok")

    sys.stdout.write("testing marshaling... ")
    sys.stdout.flush()

    oo4 = initial.pingPong(Test.OneOptional())
    test(oo4.a is None)

    oo5 = initial.pingPong(oo1)
    test(oo1.a == oo5.a)

    mo4 = initial.pingPong(Test.MultiOptional())
    test(mo4.a is None)
    test(mo4.b is None)
    test(mo4.c is None)
    test(mo4.d is None)
    test(mo4.e is None)
    test(mo4.f is None)
    test(mo4.g is None)
    test(mo4.h is None)
    test(mo4.i is None)
    test(mo4.j is None)
    test(mo4.bs is None)
    test(mo4.ss is None)
    test(mo4.iid is None)
    test(mo4.sid is None)
    test(mo4.fs is None)
    test(mo4.vs is None)

    test(mo4.shs is None)
    test(mo4.es is None)
    test(mo4.fss is None)
    test(mo4.vss is None)
    test(mo4.mips is None)

    test(mo4.ied is None)
    test(mo4.ifsd is None)
    test(mo4.ivsd is None)
    test(mo4.imipd is None)

    test(mo4.bos is None)

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
    test(mo5.mips[0] == Test.MyInterfacePrx(communicator, "test"))

    test(mo5.ied[4] == Test.MyEnum.MyEnumMember)
    test(mo5.ifsd[4] == Test.FixedStruct(78))
    test(mo5.ivsd[5] == Test.VarStruct("hello"))
    test(mo5.imipd[5] == Test.MyInterfacePrx(communicator, "test"))

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
    mo6.ifsd = mo5.ifsd
    mo6.bos = mo5.bos

    mo7 = initial.pingPong(mo6)
    test(mo7.a is None)
    test(mo7.b == mo1.b)
    test(mo7.c is None)
    test(mo7.d == mo1.d)
    test(mo7.e is None)
    test(mo7.f == mo1.f)
    test(mo7.g is None)
    test(mo7.h == mo1.h)
    test(mo7.i is None)
    test(mo7.j == mo1.j)
    test(mo7.bs[0] == 5)
    test(mo7.ss is None)
    test(mo7.iid[4] == 3)
    test(mo7.sid is None)
    test(mo7.fs == mo1.fs)
    test(mo7.vs is None)

    test(mo7.shs == mo1.shs)
    test(mo7.es is None)
    test(mo7.fss[0] == Test.FixedStruct(78))
    test(mo7.vss is None)
    test(mo7.mips is None)

    test(mo7.ied is None)
    test(mo7.ifsd[4] == Test.FixedStruct(78))
    test(mo7.ivsd is None)
    test(mo7.imipd is None)

    test(mo7.bos == [False, True, False])

    # Clear the second half of the optional members
    mo8 = Test.MultiOptional()
    mo8.a = mo5.a
    mo8.c = mo5.c
    mo8.e = mo5.e
    mo8.g = mo5.g
    mo8.i = mo5.i
    mo8.ss = mo5.ss
    mo8.sid = mo5.sid
    mo8.vs = mo5.vs

    mo8.es = mo5.es
    mo8.vss = mo5.vss
    mo8.mips = mo5.mips

    mo8.ied = mo5.ied
    mo8.ivsd = mo5.ivsd
    mo8.imipd = mo5.imipd

    mo9 = initial.pingPong(mo8)
    test(mo9.a == mo1.a)
    test(mo9.b is None)
    test(mo9.c == mo1.c)
    test(mo9.d is None)
    test(mo9.e == mo1.e)
    test(mo9.f is None)
    test(mo9.g == mo1.g)
    test(mo9.h is None)
    test(mo9.i == mo1.i)
    test(mo9.j is None)
    test(mo9.bs is None)
    test(mo9.ss == mo1.ss)
    test(mo9.iid is None)
    test(mo9.sid["test"] == 10)
    test(mo9.fs is None)
    test(mo9.vs == mo1.vs)

    test(mo9.shs is None)
    test(mo9.es[0] == Test.MyEnum.MyEnumMember and mo1.es[1] == Test.MyEnum.MyEnumMember)
    test(mo9.fss is None)
    test(mo9.vss[0] == Test.VarStruct("hello"))
    test(mo9.mips[0] == Test.MyInterfacePrx(communicator, "test"))

    test(mo9.ied[4] == Test.MyEnum.MyEnumMember)
    test(mo9.ifsd is None)
    test(mo9.ivsd[5] == Test.VarStruct("hello"))
    test(mo9.imipd[5] == Test.MyInterfacePrx(communicator, "test"))

    test(mo9.bos is None)

    g = Test.G()
    g.gg1Opt = Test.G1("gg1Opt")
    g.gg2 = Test.G2(10)
    g.gg2Opt = Test.G2(20)
    g.gg1 = Test.G1("gg1")
    r = initial.opG(g)
    test(r.gg1Opt.a == "gg1Opt")
    test(r.gg2.a == 10)
    test(r.gg2Opt.a == 20)
    test(r.gg1.a == "gg1")

    initial2 = Test.Initial2Prx(communicator, f"initial:{helper.getTestEndpoint()}")
    initial2.opVoid(15, "test")

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

    print("ok")

    sys.stdout.write("testing tag marshaling... ")
    sys.stdout.flush()

    b = Test.B()
    b2 = initial.pingPong(b)
    test(b2.ma is None)
    test(b2.mb is None)
    test(b2.mc is None)

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

    sys.stdout.write("testing marshaling of objects with optional members...")
    sys.stdout.flush()

    f = Test.F()

    f.fsf = Test.FixedStruct()
    f.fse = f.fsf

    rf = initial.pingPong(f)
    test(rf.fse == rf.fsf)

    print("ok")

    sys.stdout.write("testing optional with default values... ")
    sys.stdout.flush()

    wd = initial.pingPong(Test.WD())
    test(wd.a == 5)
    test(wd.s == "test")
    wd.a = None
    wd.s = None
    wd = initial.pingPong(wd)
    test(wd.a is None)
    test(wd.s is None)

    print("ok")

    if communicator.getProperties().getIcePropertyAsInt("Ice.Default.SlicedFormat") > 0:
        sys.stdout.write("testing marshaling with unknown class slices... ")
        sys.stdout.flush()

        c = Test.C()
        c.ss = "test"
        c.ms = "testms"
        c = initial.pingPong(c)
        test(c.ma is None)
        test(c.mb is None)
        test(c.mc is None)
        test(c.md is None)
        test(c.ss == "test")
        test(c.ms == "testms")

        print("ok")

        sys.stdout.write("testing operations with unknown optionals... ")
        sys.stdout.flush()

        initial2 = Test.Initial2Prx(communicator, f"initial:{helper.getTestEndpoint()}")
        ovs = Test.VarStruct("test")
        initial2.opClassAndUnknownOptional(Test.A(), ovs)

        print("ok")

    sys.stdout.write("testing optional parameters... ")
    sys.stdout.flush()

    (p2, p3) = initial.opByte(None)
    test(p2 is None and p3 is None)
    (p2, p3) = initial.opByte(56)
    test(p2 == 56 and p3 == 56)
    f = initial.opByteAsync(56)
    (p2, p3) = f.result()
    test(p2 == 56 and p3 == 56)

    (p2, p3) = initial.opBool(None)
    test(p2 is None and p3 is None)
    (p2, p3) = initial.opBool(True)
    test(p2 is True and p3 is True)
    f = initial.opBoolAsync(True)
    (p2, p3) = f.result()
    test(p2 is True and p3 is True)

    (p2, p3) = initial.opShort(None)
    test(p2 is None and p3 is None)
    (p2, p3) = initial.opShort(56)
    test(p2 == 56 and p3 == 56)
    f = initial.opShortAsync(56)
    (p2, p3) = f.result()
    test(p2 == 56 and p3 == 56)

    (p2, p3) = initial.opInt(None)
    test(p2 is None and p3 is None)
    (p2, p3) = initial.opInt(56)
    test(p2 == 56 and p3 == 56)
    f = initial.opIntAsync(56)
    (p2, p3) = f.result()
    test(p2 == 56 and p3 == 56)

    (p2, p3) = initial.opLong(None)
    test(p2 is None and p3 is None)
    (p2, p3) = initial.opLong(56)
    test(p2 == 56 and p3 == 56)
    f = initial.opLongAsync(56)
    (p2, p3) = f.result()
    test(p2 == 56 and p3 == 56)

    (p2, p3) = initial.opFloat(None)
    test(p2 is None and p3 is None)
    (p2, p3) = initial.opFloat(1.0)
    test(p2 == 1.0 and p3 == 1.0)
    f = initial.opFloatAsync(1.0)
    (p2, p3) = f.result()
    test(p2 == 1.0 and p3 == 1.0)

    (p2, p3) = initial.opDouble(None)
    test(p2 is None and p3 is None)
    (p2, p3) = initial.opDouble(1.0)
    test(p2 == 1.0 and p3 == 1.0)
    f = initial.opDoubleAsync(1.0)
    (p2, p3) = f.result()
    test(p2 == 1.0 and p3 == 1.0)

    (p2, p3) = initial.opString(None)
    test(p2 is None and p3 is None)
    (p2, p3) = initial.opString("test")
    test(p2 == "test" and p3 == "test")
    f = initial.opStringAsync("test")
    (p2, p3) = f.result()
    test(p2 == "test" and p3 == "test")

    (p2, p3) = initial.opMyEnum(None)
    test(p2 is None and p3 is None)
    (p2, p3) = initial.opMyEnum(Test.MyEnum.MyEnumMember)
    test(p2 == Test.MyEnum.MyEnumMember and p3 == Test.MyEnum.MyEnumMember)
    f = initial.opMyEnumAsync(Test.MyEnum.MyEnumMember)
    (p2, p3) = f.result()
    test(p2 == Test.MyEnum.MyEnumMember and p3 == Test.MyEnum.MyEnumMember)

    (p2, p3) = initial.opSmallStruct(None)
    test(p2 is None and p3 is None)
    p1 = Test.SmallStruct(56)
    (p2, p3) = initial.opSmallStruct(p1)
    test(p2 == p1 and p3 == p1)
    f = initial.opSmallStructAsync(p1)
    (p2, p3) = f.result()
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opFixedStruct(None)
    test(p2 is None and p3 is None)
    p1 = Test.FixedStruct(56)
    (p2, p3) = initial.opFixedStruct(p1)
    test(p2 == p1 and p3 == p1)
    f = initial.opFixedStructAsync(p1)
    (p2, p3) = f.result()
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opVarStruct(None)
    test(p2 is None and p3 is None)
    p1 = Test.VarStruct("test")
    (p2, p3) = initial.opVarStruct(p1)
    test(p2 == p1 and p3 == p1)
    f = initial.opVarStructAsync(p1)
    (p2, p3) = f.result()
    test(p2 == p1 and p3 == p1)

    p1 = Test.OneOptional()
    (p2, p3) = initial.opOneOptional(p1)
    test(p2.a is None and p3.a is None)
    p1 = Test.OneOptional(58)
    (p2, p3) = initial.opOneOptional(p1)
    test(p2.a == p1.a and p3.a == p1.a)
    f = initial.opOneOptionalAsync(p1)
    (p2, p3) = f.result()
    test(p2.a == p1.a and p3.a == p1.a)

    (p2, p3) = initial.opMyInterfaceProxy(None)
    test(p2 is None and p3 is None)
    p1 = Test.MyInterfacePrx.uncheckedCast(Test.MyInterfacePrx(communicator, "test"))
    (p2, p3) = initial.opMyInterfaceProxy(p1)
    test(p2 == p1 and p3 == p1)
    f = initial.opMyInterfaceProxyAsync(p1)
    (p2, p3) = f.result()
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opByteSeq(None)
    test(p2 is None and p3 is None)
    p1 = [56 for x in range(100)]
    (p2, p3) = initial.opByteSeq(p1)
    test(len(p2) == len(p1) and len(p3) == len(p1))
    test(p2[0] == 0x38)
    test(p3[0] == 0x38)
    f = initial.opByteSeqAsync(p1)
    (p2, p3) = f.result()
    test(len(p2) == len(p1) and len(p3) == len(p1))
    test(p2[0] == 0x38)
    test(p3[0] == 0x38)

    (p2, p3) = initial.opBoolSeq(None)
    test(p2 is None and p3 is None)
    p1 = [True for x in range(100)]
    (p2, p3) = initial.opBoolSeq(p1)
    test(p2 == p1 and p3 == p1)
    f = initial.opBoolSeqAsync(p1)
    (p2, p3) = f.result()
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opShortSeq(None)
    test(p2 is None and p3 is None)
    p1 = [56 for x in range(100)]
    (p2, p3) = initial.opShortSeq(p1)
    test(p2 == p1 and p3 == p1)
    f = initial.opShortSeqAsync(p1)
    (p2, p3) = f.result()
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opIntSeq(None)
    test(p2 is None and p3 is None)
    p1 = [56 for x in range(100)]
    (p2, p3) = initial.opIntSeq(p1)
    test(p2 == p1 and p3 == p1)
    f = initial.opIntSeqAsync(p1)
    (p2, p3) = f.result()
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opLongSeq(None)
    test(p2 is None and p3 is None)
    p1 = [56 for x in range(100)]
    (p2, p3) = initial.opLongSeq(p1)
    test(p2 == p1 and p3 == p1)
    f = initial.opLongSeqAsync(p1)
    (p2, p3) = f.result()
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opFloatSeq(None)
    test(p2 is None and p3 is None)
    p1 = [1.0 for x in range(100)]
    (p2, p3) = initial.opFloatSeq(p1)
    test(p2 == p1 and p3 == p1)
    f = initial.opFloatSeqAsync(p1)
    (p2, p3) = f.result()
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opDoubleSeq(None)
    test(p2 is None and p3 is None)
    p1 = [1.0 for x in range(100)]
    (p2, p3) = initial.opDoubleSeq(p1)
    test(p2 == p1 and p3 == p1)
    f = initial.opDoubleSeqAsync(p1)
    (p2, p3) = f.result()
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opStringSeq(None)
    test(p2 is None and p3 is None)
    p1 = ["test1" for x in range(100)]
    (p2, p3) = initial.opStringSeq(p1)
    test(p2 == p1 and p3 == p1)
    f = initial.opStringSeqAsync(p1)
    (p2, p3) = f.result()
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opSmallStructSeq(None)
    test(p2 is None and p3 is None)
    p1 = [Test.SmallStruct(1) for x in range(10)]
    (p2, p3) = initial.opSmallStructSeq(p1)
    test(p2 == p1 and p3 == p1)
    f = initial.opSmallStructSeqAsync(p1)
    (p2, p3) = f.result()
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opSmallStructList(None)
    test(p2 is None and p3 is None)
    p1 = tuple([Test.SmallStruct(1) for x in range(10)])
    (p2, p3) = initial.opSmallStructList(p1)
    test(p2 == p1 and p3 == p1)
    f = initial.opSmallStructListAsync(p1)
    (p2, p3) = f.result()
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opFixedStructSeq(None)
    test(p2 is None and p3 is None)
    p1 = [Test.FixedStruct(1) for x in range(10)]
    (p2, p3) = initial.opFixedStructSeq(p1)
    test(p2 == p1 and p3 == p1)
    f = initial.opFixedStructSeqAsync(p1)
    (p2, p3) = f.result()
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opFixedStructList(None)
    test(p2 is None and p3 is None)
    p1 = tuple([Test.FixedStruct(1) for x in range(10)])
    (p2, p3) = initial.opFixedStructList(p1)
    test(p2 == p1 and p3 == p1)
    f = initial.opFixedStructListAsync(p1)
    (p2, p3) = f.result()
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opVarStructSeq(None)
    test(p2 is None and p3 is None)
    p1 = [Test.VarStruct("test") for x in range(10)]
    (p2, p3) = initial.opVarStructSeq(p1)
    test(p2 == p1 and p3 == p1)
    f = initial.opVarStructSeqAsync(p1)
    (p2, p3) = f.result()
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opIntIntDict(None)
    test(p2 is None and p3 is None)
    p1 = {1: 2, 2: 3}
    (p2, p3) = initial.opIntIntDict(p1)
    test(p2 == p1 and p3 == p1)
    f = initial.opIntIntDictAsync(p1)
    (p2, p3) = f.result()
    test(p2 == p1 and p3 == p1)

    (p2, p3) = initial.opStringIntDict(None)
    test(p2 is None and p3 is None)
    p1 = {"1": 2, "2": 3}
    (p2, p3) = initial.opStringIntDict(p1)
    test(p2 == p1 and p3 == p1)
    f = initial.opStringIntDictAsync(p1)
    (p2, p3) = f.result()
    test(p2 == p1 and p3 == p1)

    try:
        (p1, p2, p3) = initial.opRequiredAfterOptional(1, 2, 3)
        test(p1 == 1)
        test(p2 == 2)
        test(p3 == 3)

        (p1, p2, p3) = initial.opOptionalAfterRequired(1)
        test(p1 == 1)
        test(p2 is None)
        test(p3 is None)

        (p1, p2, p3) = initial.opOptionalAfterRequired(1, p3=3)
        test(p1 == 1)
        test(p2 is None)
        test(p3 == 3)
    except Ice.OperationNotExistException:
        # Expected for language mappings that don't implement these operations when running cross tests
        pass
    print("ok")

    sys.stdout.write("testing exception optionals... ")
    sys.stdout.flush()

    try:
        initial.opOptionalException(None, None)
    except Test.OptionalException as ex:
        test(ex.a is None)
        test(ex.b is None)

    try:
        initial.opOptionalException(30, "test")
    except Test.OptionalException as ex:
        test(ex.a == 30)
        test(ex.b == "test")

    try:
        #
        # Use the 1.0 encoding with an exception whose only data members are optional.
        #
        initial.ice_encodingVersion(Ice.Encoding_1_0).opOptionalException(30, "test")
    except Test.OptionalException as ex:
        test(ex.a is None)
        test(ex.b is None)

    try:
        initial.opDerivedException(None, None)
    except Test.DerivedException as ex:
        test(ex.a is None)
        test(ex.b is None)
        test(ex.ss is None)
        test(ex.d1 == "d1")
        test(ex.d2 == "d2")

    try:
        initial.opDerivedException(30, "test2")
    except Test.DerivedException as ex:
        test(ex.a == 30)
        test(ex.b == "test2")
        test(ex.ss == "test2")
        test(ex.d1 == "d1")
        test(ex.d2 == "d2")

    try:
        initial.opRequiredException(None, None)
    except Test.RequiredException as ex:
        test(ex.a is None)
        test(ex.b is None)
        test(ex.ss == "test")

    try:
        initial.opRequiredException(30, "test2")
    except Test.RequiredException as ex:
        test(ex.a == 30)
        test(ex.b == "test2")
        test(ex.ss == "test2")

    print("ok")

    sys.stdout.write("testing optionals with marshaled results... ")
    sys.stdout.flush()

    test(initial.opMStruct1() is not None)
    test(initial.opMDict1() is not None)
    test(initial.opMSeq1() is not None)

    (p3, p2) = initial.opMStruct2(None)
    test(p2 is None and p3 is None)

    p1 = Test.SmallStruct()
    (p3, p2) = initial.opMStruct2(p1)
    test(p2 == p1 and p3 == p1)

    (p3, p2) = initial.opMSeq2(None)
    test(p2 is None and p3 is None)

    p1 = ["hello"]
    (p3, p2) = initial.opMSeq2(p1)
    test(p2[0] == "hello" and p3[0] == "hello")

    (p3, p2) = initial.opMDict2(None)
    test(p2 is None and p3 is None)

    p1 = {"test": 54}
    (p3, p2) = initial.opMDict2(p1)
    test(p2["test"] == 54 and p3["test"] == 54)

    print("ok")

    return initial
