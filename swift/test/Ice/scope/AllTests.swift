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
        let obj = try communicator.stringToProxy("i1:\(helper.getTestEndpoint())")!
        let i = try await checkedCast(prx: obj, type: IPrx.self)!

        let s1 = S(v: 0)
        let (s3, s2) = try await i.opS(s1)
        try test(s2 == s1)
        try test(s3 == s1)

        let sseq1 = [s1]
        let (sseq3, sseq2) = try await i.opSSeq(sseq1)
        try test(sseq2 == sseq1)
        try test(sseq3 == sseq1)

        let smap1 = ["a": s1]
        let (smap3, smap2) = try await i.opSMap(smap1)
        try test(smap2 == smap1)
        try test(smap3 == smap1)

        let c1 = C(s: s1)
        let (c3, c2) = try await i.opC(c1)
        try test(c2!.s == s1)
        try test(c3!.s == s1)

        let cseq1 = [c1]
        let (cseq3, cseq2) = try await i.opCSeq(cseq1)
        try test(cseq2[0]!.s == s1)
        try test(cseq3[0]!.s == s1)

        let cmap1 = ["a": c1]
        let (cmap3, cmap2) = try await i.opCMap(cmap1)
        try test(cmap2["a"]!!.s == s1)
        try test(cmap3["a"]!!.s == s1)

        let e = try await i.opE1(.v1)
        try test(e == .v1)

        let s = try await i.opS1(S1(s: "S1"))
        try test(s.s == "S1")

        let c = try await i.opC1(C1(s: "C1"))!
        try test(c.s == "C1")

        let ss = try await i.opS1Seq([S1(s: "S1")])
        try test(ss[0].s == "S1")

        let sm = try await i.opS1Map(["s1": S1(s: "S1")])
        try test(sm["s1"]!.s == "S1")
    }

    do {
        let obj = try communicator.stringToProxy("i1:\(helper.getTestEndpoint())")!
        let i = try await checkedCast(prx: obj, type: IPrx.self)!

        let s1 = S(v: 0)
        let (s3, s2) = try await i.opS(s1)
        try test(s2 == s1)
        try test(s3 == s1)

        let sseq1 = [s1]
        let (sseq3, sseq2) = try await i.opSSeq(sseq1)
        try test(sseq2 == sseq1)
        try test(sseq3 == sseq1)

        let smap1 = ["a": s1]
        let (smap3, smap2) = try await i.opSMap(smap1)
        try test(smap2 == smap1)
        try test(smap3 == smap1)

        let c1 = C(s: s1)
        let (c3, c2) = try await i.opC(c1)
        try test(c2!.s == s1)
        try test(c3!.s == s1)

        let cseq1 = [c1]
        let (cseq3, cseq2) = try await i.opCSeq(cseq1)
        try test(cseq2[0]!.s == s1)
        try test(cseq3[0]!.s == s1)

        let cmap1 = ["a": c1]
        let (cmap3, cmap2) = try await i.opCMap(cmap1)
        try test(cmap2["a"]!!.s == s1)
        try test(cmap3["a"]!!.s == s1)
    }

    do {
        let obj = try communicator.stringToProxy("i2:\(helper.getTestEndpoint())")!
        let i = try await checkedCast(prx: obj, type: InnerIPrx.self)!

        let s1 = InnerInner2S(v: 0)
        let (s3, s2) = try await i.opS(s1)
        try test(s2 == s1)
        try test(s3 == s1)

        let sseq1 = [s1]
        let (sseq3, sseq2) = try await i.opSSeq(sseq1)
        try test(sseq2 == sseq1)
        try test(sseq3 == sseq1)

        let smap1 = ["a": s1]
        let (smap3, smap2) = try await i.opSMap(smap1)
        try test(smap2 == smap1)
        try test(smap3 == smap1)

        let c1 = InnerInner2C(s: s1)
        let (c3, c2) = try await i.opC(c1)
        try test(c2!.s == c1.s)
        try test(c3!.s == c1.s)

        let cseq1 = [c1]
        let (cseq3, cseq2) = try await i.opCSeq(cseq1)
        try test(cseq2[0]!.s == s1)
        try test(cseq3[0]!.s == s1)

        let cmap1 = ["a": c1]
        let (cmap3, cmap2) = try await i.opCMap(cmap1)
        try test(cmap2["a"]!!.s == s1)
        try test(cmap3["a"]!!.s == s1)
    }

    do {
        let obj = try communicator.stringToProxy("i3:\(helper.getTestEndpoint())")!
        let i = try await checkedCast(prx: obj, type: InnerInner2IPrx.self)!

        let s1 = InnerInner2S(v: 0)
        let (s3, s2) = try await i.opS(s1)
        try test(s2 == s1)
        try test(s3 == s1)

        let sseq1 = [s1]
        let (sseq3, sseq2) = try await i.opSSeq(sseq1)
        try test(sseq2[0] == s1)
        try test(sseq3[0] == s1)

        let smap1 = ["a": s1]
        let (smap3, smap2) = try await i.opSMap(smap1)
        try test(smap2 == smap1)
        try test(smap3 == smap1)

        let c1 = InnerInner2C(s: s1)
        let (c3, c2) = try await i.opC(c1)
        try test(c2!.s == s1)
        try test(c3!.s == s1)

        let cseq1 = [c1]
        let (cseq3, cseq2) = try await i.opCSeq(cseq1)
        try test(cseq2[0]!.s == s1)
        try test(cseq3[0]!.s == s1)

        let cmap1 = ["a": c1]
        let (cmap3, cmap2) = try await i.opCMap(cmap1)
        try test(cmap2["a"]!!.s == s1)
        try test(cmap3["a"]!!.s == s1)
    }

    do {
        let obj = try communicator.stringToProxy("i4:\(helper.getTestEndpoint())")!
        let i = try await checkedCast(prx: obj, type: InnerTestInner2IPrx.self)!

        let s1 = S(v: 0)
        let (s3, s2) = try await i.opS(s1)
        try test(s2 == s1)
        try test(s3 == s1)

        let sseq1 = [s1]
        let (sseq3, sseq2) = try await i.opSSeq(sseq1)
        try test(sseq2[0] == s1)
        try test(sseq3[0] == s1)

        let smap1 = ["a": s1]
        let (smap3, smap2) = try await i.opSMap(smap1)
        try test(smap2 == smap1)
        try test(smap3 == smap1)

        let c1 = C(s: s1)
        let (c3, c2) = try await i.opC(c1)
        try test(c2!.s == s1)
        try test(c3!.s == s1)

        let cseq1 = [c1]
        let (cseq3, cseq2) = try await i.opCSeq(cseq1)
        try test(cseq2[0]!.s == s1)
        try test(cseq3[0]!.s == s1)

        let cmap1 = ["a": c1]
        let (cmap3, cmap2) = try await i.opCMap(cmap1)
        try test(cmap2["a"]!!.s == s1)
        try test(cmap3["a"]!!.s == s1)
    }

    do {
        let obj = try communicator.stringToProxy("dpi:\(helper.getTestEndpoint())")!
        let i = try await checkedCast(prx: obj, type: DifferentInnerIPrx.self)!

        let s1 = InnerS(v: 0)
        let (s3, s2) = try await i.opS(s1)
        try test(s2 == s1)
        try test(s3 == s1)

        let c1 = DifferentInnerClass1(l: 79);
        let (c3, c2) = try await i.opC(c1);
        try test(c2!.l == 79)
        try test(c3!.l == 79)
    }

    do {
        let obj = try communicator.stringToProxy("npi:\(helper.getTestEndpoint())")!
        let i = try await checkedCast(prx: obj, type: NoPrefixIPrx.self)!

        let s1 = InnerS(v: 0)
        let (s3, s2) = try await i.opS(s1)
        try test(s2 == s1)
        try test(s3 == s1)

        let c1 = NoPrefixClass1(l: 79);
        let (c3, c2) = try await i.opC(c1);
        try test(c2!.l == 79)
        try test(c3!.l == 79)
    }

    do {
        let obj = try communicator.stringToProxy("i1:\(helper.getTestEndpoint())")!
        let i = try await checkedCast(prx: obj, type: IPrx.self)!
        try await i.shutdown()
    }
    output.writeLine("ok")
}
