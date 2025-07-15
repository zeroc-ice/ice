# Copyright (c) ZeroC, Inc.

import sys


import Test
from Inner.Test.Inner2 import MyInterfacePrx as Inner_Test_Inner2_MyInterfacePrx
from Test.Inner import MyInterfacePrx as Test_Inner_MyInterfacePrx, MyStruct as Test_Inner_MyStruct
from Test.Inner.Inner2 import (
    MyInterfacePrx as Test_Inner_Inner2_MyInterfacePrx,
    MyStruct as Test_Inner_Inner2_MyStruct,
    MyClass as Test_Inner_Inner2_MyClass,
)


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


def allTests(helper, communicator):
    sys.stdout.write("test using same type name in different Slice modules... ")
    sys.stdout.flush()
    i1 = Test.MyInterfacePrx(communicator, f"i1:{helper.getTestEndpoint()}")

    s1 = Test.MyStruct(0)

    s2, s3 = i1.opMyStruct(s1)

    test(s2 == s1)
    test(s3 == s1)

    sseq1 = [s1]

    sseq2, sseq3 = i1.opMyStructSeq(sseq1)

    test(sseq2[0] == s1)
    test(sseq3[0] == s1)

    smap1 = {"a": s1}
    smap2, smap3 = i1.opMyStructMap(smap1)
    test(smap2["a"] == s1)
    test(smap3["a"] == s1)

    c1 = Test.MyClass(s1)

    c2, c3 = i1.opMyClass(c1)

    test(c2.s == s1)
    test(c3.s == s1)

    cseq1 = [c1]
    cseq2, cseq3 = i1.opMyClassSeq(cseq1)

    test(cseq2[0].s == s1)
    test(cseq3[0].s == s1)

    cmap1 = {"a": c1}
    cmap2, cmap3 = i1.opMyClassMap(cmap1)
    test(cmap2["a"].s == s1)
    test(cmap3["a"].s == s1)

    e = i1.opMyEnum(Test.MyEnum.v1)
    test(e == Test.MyEnum.v1)

    s = i1.opMyOtherStruct(Test.MyOtherStruct("MyOtherStruct"))
    test(s.s == "MyOtherStruct")

    c = i1.opMyOtherClass(Test.MyOtherClass("MyOtherClass"))
    test(c.s == "MyOtherClass")

    i2 = Test_Inner_Inner2_MyInterfacePrx(communicator, f"i2:{helper.getTestEndpoint()}")

    s1 = Test_Inner_Inner2_MyStruct(0)

    s2, s3 = i2.opMyStruct(s1)

    test(s2 == s1)
    test(s3 == s1)

    sseq1 = [s1]

    sseq2, sseq3 = i2.opMyStructSeq(sseq1)

    test(sseq2[0] == s1)
    test(sseq3[0] == s1)

    smap1 = {"a": s1}
    smap2, smap3 = i2.opMyStructMap(smap1)
    test(smap2["a"] == s1)
    test(smap3["a"] == s1)

    c1 = Test_Inner_Inner2_MyClass(s1)

    c2, c3 = i2.opMyClass(c1)

    test(c2.s == s1)
    test(c3.s == s1)

    cseq1 = [c1]
    cseq2, cseq3 = i2.opMyClassSeq(cseq1)

    test(cseq2[0].s == s1)
    test(cseq3[0].s == s1)

    cmap1 = {"a": c1}
    cmap2, cmap3 = i2.opMyClassMap(cmap1)
    test(cmap2["a"].s == s1)
    test(cmap3["a"].s == s1)

    i3 = Test_Inner_MyInterfacePrx(communicator, f"i3:{helper.getTestEndpoint()}")

    s1 = Test_Inner_Inner2_MyStruct(0)

    s2, s3 = i3.opMyStruct(s1)

    test(s2 == s1)
    test(s3 == s1)

    sseq1 = [s1]

    sseq2, sseq3 = i3.opMyStructSeq(sseq1)

    test(sseq2[0] == s1)
    test(sseq3[0] == s1)

    smap1 = {"a": s1}
    smap2, smap3 = i3.opMyStructMap(smap1)
    test(smap2["a"] == s1)
    test(smap3["a"] == s1)

    c1 = Test_Inner_Inner2_MyClass(s1)

    c2, c3 = i3.opMyClass(c1)

    test(c2.s == s1)
    test(c3.s == s1)

    cseq1 = [c1]
    cseq2, cseq3 = i3.opMyClassSeq(cseq1)

    test(cseq2[0].s == s1)
    test(cseq3[0].s == s1)

    cmap1 = {"a": c1}
    cmap2, cmap3 = i3.opMyClassMap(cmap1)
    test(cmap2["a"].s == s1)
    test(cmap3["a"].s == s1)

    i4 = Inner_Test_Inner2_MyInterfacePrx(communicator, f"i4:{helper.getTestEndpoint()}")

    s1 = Test.MyStruct(0)

    s2, s3 = i4.opMyStruct(s1)

    test(s2 == s1)
    test(s3 == s1)

    sseq1 = [s1]

    sseq2, sseq3 = i4.opMyStructSeq(sseq1)

    test(sseq2[0] == s1)
    test(sseq3[0] == s1)

    smap1 = {"a": s1}
    smap2, smap3 = i4.opMyStructMap(smap1)
    test(smap2["a"] == s1)
    test(smap3["a"] == s1)

    c1 = Test.MyClass(s1)

    c2, c3 = i4.opMyClass(c1)

    test(c2.s == s1)
    test(c3.s == s1)

    cseq1 = [c1]
    cseq2, cseq3 = i4.opMyClassSeq(cseq1)

    test(cseq2[0].s == s1)
    test(cseq3[0].s == s1)

    cmap1 = {"a": c1}
    cmap2, cmap3 = i4.opMyClassMap(cmap1)
    test(cmap2["a"].s == s1)
    test(cmap3["a"].s == s1)

    i1.shutdown()
    print("ok")
