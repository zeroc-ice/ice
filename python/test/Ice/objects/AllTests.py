# Copyright (c) ZeroC, Inc.

import sys
from typing import cast

from generated.test.Ice.objects import Test
from generated.test.Ice.objects.client_private.Test import UnexpectedObjectExceptionTestPrx
from TestHelper import TestHelper, test

import Ice


def allTests(helper: TestHelper, communicator: Ice.Communicator) -> Test.InitialPrx:
    initial = Test.InitialPrx(communicator, f"initial:{helper.getTestEndpoint()}")

    sys.stdout.write("getting B1... ")
    sys.stdout.flush()
    b1 = initial.getB1()
    test(b1)
    print("ok")

    sys.stdout.write("getting B2... ")
    sys.stdout.flush()
    b2 = initial.getB2()
    test(b2)
    print("ok")

    sys.stdout.write("getting C... ")
    sys.stdout.flush()
    c = initial.getC()
    test(c)
    print("ok")

    sys.stdout.write("getting D... ")
    sys.stdout.flush()
    d = initial.getD()
    test(d)
    print("ok")

    sys.stdout.write("getting K... ")
    sys.stdout.flush()
    k = initial.getK()
    assert k is not None
    assert isinstance(k.value, Test.L)
    test(k.value.data == "l")
    print("ok")

    sys.stdout.write("testing Value as parameter... ")
    sys.stdout.flush()
    v1, v2 = initial.opValue(Test.L("l"))
    assert isinstance(v1, Test.L)
    assert isinstance(v2, Test.L)
    test(v1.data == "l")
    test(v2.data == "l")

    v1, v2 = initial.opValueSeq([Test.L("l")])
    assert isinstance(v1[0], Test.L)
    assert isinstance(v2[0], Test.L)
    test(v1[0].data == "l")
    test(v2[0].data == "l")

    v1, v2 = initial.opValueMap({"l": Test.L("l")})
    assert isinstance(v1["l"], Test.L)
    assert isinstance(v2["l"], Test.L)
    test(v1["l"].data == "l")
    test(v2["l"].data == "l")
    print("ok")

    sys.stdout.write("getting D1... ")
    sys.stdout.flush()
    d1 = initial.getD1(Test.D1(Test.A1("a1"), Test.A1("a2"), Test.A1("a3"), Test.A1("a4")))
    assert d1 is not None
    assert d1.a1 is not None
    assert d1.a2 is not None
    assert d1.a3 is not None
    assert d1.a4 is not None
    test(d1.a1.name == "a1")
    test(d1.a2.name == "a2")
    test(d1.a3.name == "a3")
    test(d1.a4.name == "a4")
    print("ok")

    sys.stdout.write("throw EDerived... ")
    sys.stdout.flush()
    try:
        initial.throwEDerived()
        test(False)
    except Test.EDerived as e:
        assert e.a1 is not None
        assert e.a2 is not None
        assert e.a3 is not None
        assert e.a4 is not None
        test(e.a1.name == "a1")
        test(e.a2.name == "a2")
        test(e.a3.name == "a3")
        test(e.a4.name == "a4")
    print("ok")

    sys.stdout.write("setting G... ")
    sys.stdout.flush()
    try:
        initial.setG(Test.G(Test.S("hello"), "g"))
    except Ice.OperationNotExistException:
        pass
    print("ok")

    sys.stdout.write("checking consistency... ")
    sys.stdout.flush()
    test(b1 != b2)
    test(b1 != c)
    test(b1 != d)
    test(b2 != c)
    test(b2 != d)
    test(c != d)
    assert b1 is not None
    test(b1.theB == b1)
    test(b1.theC is None)
    assert isinstance(b1.theA, Test.B)
    test(b1.theA.theA == b1.theA)
    test(b1.theA.theB == b1)
    assert b1.theA.theC is not None
    test(b1.theA.theC.theB == b1.theA)
    test(b1.preMarshalInvoked)
    test(b1.postUnmarshalInvoked)
    test(b1.theA.preMarshalInvoked)
    test(b1.theA.postUnmarshalInvoked)
    test(b1.theA.theC.preMarshalInvoked)
    test(b1.theA.theC.postUnmarshalInvoked)
    # More tests possible for b2 and d, but I think this is already sufficient.
    assert b2 is not None
    test(b2.theA == b2)
    assert d is not None
    test(d.theC is None)
    print("ok")

    sys.stdout.write("getting B1, B2, C, and D all at once... ")
    sys.stdout.flush()
    b1, b2, c, d = initial.getAll()
    assert b1 is not None
    assert b2 is not None
    assert c is not None
    assert d is not None
    print("ok")

    sys.stdout.write("checking renamed classes... ")
    sys.stdout.flush()
    renamed = Test.PyClass("renamed")
    r = initial.opRenamedClass(renamed)
    assert r is not None
    test(isinstance(r, Test.PyClass))
    test(r.data == "renamed")

    renamed = Test.PyDerivedClass("renamed", "derived")
    r = initial.opRenamedClass(renamed)
    assert r is not None
    test(isinstance(r, Test.PyDerivedClass))
    r = cast(Test.PyDerivedClass, r)
    test(r.data == "renamed")
    test(r.moreData == "derived")
    print("ok")

    sys.stdout.write("checking consistency... ")
    sys.stdout.flush()
    test(b1 != b2)
    test(b1.theA == b2)
    test(b1.theB == b1)
    test(b1.theC is None)
    test(b2.theA == b2)
    test(b2.theB == b1)
    test(b2.theC == c)
    test(c.theB == b2)
    test(d.theA == b1)
    test(d.theB == b2)
    test(d.theC is None)
    test(d.preMarshalInvoked)
    test(d.postUnmarshalInvoked)
    assert d.theA is not None
    assert d.theB is not None
    assert d.theB.theC is not None
    test(d.theA.preMarshalInvoked)
    test(d.theA.postUnmarshalInvoked)
    test(d.theB.preMarshalInvoked)
    test(d.theB.postUnmarshalInvoked)
    test(d.theB.theC.preMarshalInvoked)
    test(d.theB.theC.postUnmarshalInvoked)
    print("ok")

    sys.stdout.write("testing sequences... ")
    try:
        sys.stdout.flush()
        initial.opBaseSeq([])

        retS, outS = initial.opBaseSeq([Test.Base()])
        test(len(retS) == 1 and len(outS) == 1)
    except Ice.OperationNotExistException:
        pass
    print("ok")

    sys.stdout.write("testing recursive type... ")
    sys.stdout.flush()
    top = Test.Recursive()
    bottom = top

    for _ in range(1, 10):
        bottom.v = Test.Recursive()
        bottom = bottom.v
    initial.setRecursive(top)

    # Adding one more level would exceed the max class graph depth
    bottom.v = Test.Recursive()
    bottom = bottom.v

    try:
        initial.setRecursive(top)
        test(False)
    except Ice.UnknownLocalException:
        # Expected marshal exception from the server (max class graph depth reached)
        pass
    initial.setRecursive(Test.Recursive())
    print("ok")

    sys.stdout.write("testing compact ID... ")
    sys.stdout.flush()
    try:
        r = initial.getCompact()
        test(r)
    except Ice.OperationNotExistException:
        pass
    print("ok")

    sys.stdout.write("testing marshaled results...")
    sys.stdout.flush()
    b1 = initial.getMB()
    test(b1 is not None and b1.theB == b1)
    b1 = cast(Ice.Future, initial.getAMDMBAsync()).result()
    test(b1 is not None and b1.theB == b1)
    print("ok")

    # Don't run this test with collocation, this should work with collocation
    # but the test isn't written to support it (we'd need support for the
    # streaming interface)
    if initial.ice_getConnection():
        sys.stdout.write("testing UnexpectedObjectException... ")
        sys.stdout.flush()
        uoet = UnexpectedObjectExceptionTestPrx(communicator, f"uoet:{helper.getTestEndpoint()}")
        test(uoet)
        try:
            uoet.op()
            test(False)
        except Ice.MarshalException as ex:
            test("::Test::AlsoEmpty" in str(ex))
            test("::Test::Empty" in str(ex))
        except Ice.Exception as ex:
            print(ex)
            test(False)
        except Exception:
            print(sys.exc_info())
            test(False)
        print("ok")

    sys.stdout.write("testing class containing complex dictionary... ")
    sys.stdout.flush()
    m = Test.M()
    m.v = {}
    k1 = Test.StructKey(1, "1")
    m.v[k1] = Test.L("one")
    k2 = Test.StructKey(2, "2")
    m.v[k2] = Test.L("two")
    m1, m2 = initial.opM(m)
    assert m1 is not None
    assert m2 is not None
    test(len(m1.v) == 2)
    test(len(m2.v) == 2)

    test(cast(Test.L, m1.v[k1]).data == "one")
    test(cast(Test.L, m2.v[k1]).data == "one")

    test(cast(Test.L, m1.v[k2]).data == "two")
    test(cast(Test.L, m2.v[k2]).data == "two")

    print("ok")

    sys.stdout.write("testing forward declarations... ")
    sys.stdout.flush()
    f11, f12 = initial.opF1(Test.F1("F11"))
    assert f11 is not None
    assert f12 is not None
    test(f11.name == "F11")
    test(f12.name == "F12")

    f21, f22 = initial.opF2(Test.F2Prx(communicator, f"F21:{helper.getTestEndpoint()}"))
    assert f21 is not None
    assert f22 is not None
    test(f21.ice_getIdentity().name == "F21")
    f21.op()
    test(f22.ice_getIdentity().name == "F22")

    if initial.hasF3():
        f31, f32 = initial.opF3(Test.F3(f11, f21))
        assert f31 is not None
        assert f31.f1 is not None
        assert f31.f2 is not None
        assert f32 is not None
        assert f32.f1 is not None
        assert f32.f2 is not None

        test(f31.f1.name == "F11")
        test(f31.f2.ice_getIdentity().name == "F21")

        test(f32.f1.name == "F12")
        test(f32.f2.ice_getIdentity().name == "F22")
    print("ok")

    sys.stdout.write("testing sending class cycle... ")
    sys.stdout.flush()
    rec = Test.Recursive()
    rec.v = rec
    acceptsCycles = initial.acceptsClassCycles()
    try:
        initial.setCycle(rec)
        test(acceptsCycles)
    except Ice.UnknownLocalException:
        test(not acceptsCycles)
    print("ok")

    return initial
