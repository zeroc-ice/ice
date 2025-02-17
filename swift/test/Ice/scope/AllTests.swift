// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

func allTests(helper: TestHelper) async throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }
    let output = helper.getWriter()
    output.write("test using same type name in different Slice modules... ")
    let communicator = helper.communicator()

    do {
        let i = try makeProxy(
            communicator: communicator, proxyString: "i1:\(helper.getTestEndpoint())", type: MyInterfacePrx.self)

        let s1 = MyStruct(v: 0)
        let (s3, s2) = try await i.opMyStruct(s1)
        try test(s2 == s1)
        try test(s3 == s1)

        let sseq1 = [s1]
        let (sseq3, sseq2) = try await i.opMyStructSeq(sseq1)
        try test(sseq2 == sseq1)
        try test(sseq3 == sseq1)

        let smap1 = ["a": s1]
        let (smap3, smap2) = try await i.opMyStructMap(smap1)
        try test(smap2 == smap1)
        try test(smap3 == smap1)

        let c1 = MyClass(s: s1)
        let (c3, c2) = try await i.opMyClass(c1)
        try test(c2!.s == s1)
        try test(c3!.s == s1)

        let cseq1 = [c1]
        let (cseq3, cseq2) = try await i.opMyClassSeq(cseq1)
        try test(cseq2[0]!.s == s1)
        try test(cseq3[0]!.s == s1)

        let cmap1 = ["a": c1]
        let (cmap3, cmap2) = try await i.opMyClassMap(cmap1)
        try test(cmap2["a"]!!.s == s1)
        try test(cmap3["a"]!!.s == s1)

        let e = try await i.opMyEnum(.v1)
        try test(e == .v1)

        let s = try await i.opMyOtherStruct(MyOtherStruct(s: "MyOtherStruct"))
        try test(s.s == "MyOtherStruct")

        let c = try await i.opMyOtherClass(MyOtherClass(s: "MyOtherClass"))!
        try test(c.s == "MyOtherClass")

        let ss = try await i.opMyOtherStructSeq([MyOtherStruct(s: "MyOtherStruct")])
        try test(ss[0].s == "MyOtherStruct")

        let sm = try await i.opMyOtherStructMap(["s1": MyOtherStruct(s: "MyOtherStruct")])
        try test(sm["s1"]!.s == "MyOtherStruct")
    }

    do {
        let i = try makeProxy(
            communicator: communicator, proxyString: "i1:\(helper.getTestEndpoint())", type: MyInterfacePrx.self)

        let s1 = MyStruct(v: 0)
        let (s3, s2) = try await i.opMyStruct(s1)
        try test(s2 == s1)
        try test(s3 == s1)

        let sseq1 = [s1]
        let (sseq3, sseq2) = try await i.opMyStructSeq(sseq1)
        try test(sseq2 == sseq1)
        try test(sseq3 == sseq1)

        let smap1 = ["a": s1]
        let (smap3, smap2) = try await i.opMyStructMap(smap1)
        try test(smap2 == smap1)
        try test(smap3 == smap1)

        let c1 = MyClass(s: s1)
        let (c3, c2) = try await i.opMyClass(c1)
        try test(c2!.s == s1)
        try test(c3!.s == s1)

        let cseq1 = [c1]
        let (cseq3, cseq2) = try await i.opMyClassSeq(cseq1)
        try test(cseq2[0]!.s == s1)
        try test(cseq3[0]!.s == s1)

        let cmap1 = ["a": c1]
        let (cmap3, cmap2) = try await i.opMyClassMap(cmap1)
        try test(cmap2["a"]!!.s == s1)
        try test(cmap3["a"]!!.s == s1)
    }

    do {
        let i = try makeProxy(
            communicator: communicator, proxyString: "i2:\(helper.getTestEndpoint())", type: InnerMyInterfacePrx.self)

        let s1 = InnerInner2MyStruct(v: 0)
        let (s3, s2) = try await i.opMyStruct(s1)
        try test(s2 == s1)
        try test(s3 == s1)

        let sseq1 = [s1]
        let (sseq3, sseq2) = try await i.opMyStructSeq(sseq1)
        try test(sseq2 == sseq1)
        try test(sseq3 == sseq1)

        let smap1 = ["a": s1]
        let (smap3, smap2) = try await i.opMyStructMap(smap1)
        try test(smap2 == smap1)
        try test(smap3 == smap1)

        let c1 = InnerInner2MyClass(s: s1)
        let (c3, c2) = try await i.opMyClass(c1)
        try test(c2!.s == c1.s)
        try test(c3!.s == c1.s)

        let cseq1 = [c1]
        let (cseq3, cseq2) = try await i.opMyClassSeq(cseq1)
        try test(cseq2[0]!.s == s1)
        try test(cseq3[0]!.s == s1)

        let cmap1 = ["a": c1]
        let (cmap3, cmap2) = try await i.opMyClassMap(cmap1)
        try test(cmap2["a"]!!.s == s1)
        try test(cmap3["a"]!!.s == s1)
    }

    do {
        let i = try makeProxy(
            communicator: communicator, proxyString: "i3:\(helper.getTestEndpoint())",
            type: InnerInner2MyInterfacePrx.self)

        let s1 = InnerInner2MyStruct(v: 0)
        let (s3, s2) = try await i.opMyStruct(s1)
        try test(s2 == s1)
        try test(s3 == s1)

        let sseq1 = [s1]
        let (sseq3, sseq2) = try await i.opMyStructSeq(sseq1)
        try test(sseq2[0] == s1)
        try test(sseq3[0] == s1)

        let smap1 = ["a": s1]
        let (smap3, smap2) = try await i.opMyStructMap(smap1)
        try test(smap2 == smap1)
        try test(smap3 == smap1)

        let c1 = InnerInner2MyClass(s: s1)
        let (c3, c2) = try await i.opMyClass(c1)
        try test(c2!.s == s1)
        try test(c3!.s == s1)

        let cseq1 = [c1]
        let (cseq3, cseq2) = try await i.opMyClassSeq(cseq1)
        try test(cseq2[0]!.s == s1)
        try test(cseq3[0]!.s == s1)

        let cmap1 = ["a": c1]
        let (cmap3, cmap2) = try await i.opMyClassMap(cmap1)
        try test(cmap2["a"]!!.s == s1)
        try test(cmap3["a"]!!.s == s1)
    }

    do {
        let i = try makeProxy(
            communicator: communicator, proxyString: "i4:\(helper.getTestEndpoint())",
            type: InnerTestInner2MyInterfacePrx.self)

        let s1 = MyStruct(v: 0)
        let (s3, s2) = try await i.opMyStruct(s1)
        try test(s2 == s1)
        try test(s3 == s1)

        let sseq1 = [s1]
        let (sseq3, sseq2) = try await i.opMyStructSeq(sseq1)
        try test(sseq2[0] == s1)
        try test(sseq3[0] == s1)

        let smap1 = ["a": s1]
        let (smap3, smap2) = try await i.opMyStructMap(smap1)
        try test(smap2 == smap1)
        try test(smap3 == smap1)

        let c1 = MyClass(s: s1)
        let (c3, c2) = try await i.opMyClass(c1)
        try test(c2!.s == s1)
        try test(c3!.s == s1)

        let cseq1 = [c1]
        let (cseq3, cseq2) = try await i.opMyClassSeq(cseq1)
        try test(cseq2[0]!.s == s1)
        try test(cseq3[0]!.s == s1)

        let cmap1 = ["a": c1]
        let (cmap3, cmap2) = try await i.opMyClassMap(cmap1)
        try test(cmap2["a"]!!.s == s1)
        try test(cmap3["a"]!!.s == s1)
    }

    do {
        let i = try makeProxy(
            communicator: communicator, proxyString: "dpi:\(helper.getTestEndpoint())",
            type: DifferentMyInterfacePrx.self)

        let s1 = InnerMyStruct(v: 0)
        let (s3, s2) = try await i.opMyStruct(s1)
        try test(s2 == s1)
        try test(s3 == s1)

        let c1 = DifferentMyClass(value: 79)
        let (c3, c2) = try await i.opMyClass(c1)
        try test(c2!.value == 79)
        try test(c3!.value == 79)
    }

    do {
        let i = try makeProxy(
            communicator: communicator, proxyString: "npi:\(helper.getTestEndpoint())",
            type: NoPrefixMyInterfacePrx.self)

        let s1 = InnerMyStruct(v: 0)
        let (s3, s2) = try await i.opMyStruct(s1)
        try test(s2 == s1)
        try test(s3 == s1)

        let c1 = NoPrefixMyClass(value: 79)
        let (c3, c2) = try await i.opMyClass(c1)
        try test(c2!.value == 79)
        try test(c3!.value == 79)
    }

    do {
        let i = try makeProxy(
            communicator: communicator, proxyString: "i1:\(helper.getTestEndpoint())", type: MyInterfacePrx.self)
        try await i.shutdown()
    }
    output.writeLine("ok")
}
