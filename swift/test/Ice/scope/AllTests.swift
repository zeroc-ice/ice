//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon

func allTests(helper: TestHelper) throws {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }
    let output = helper.getWriter()
    output.write("test same Slice type name in different scopes... ");
    let communicator = helper.communicator()

    do {
        let obj = try communicator.stringToProxy("i1:\(helper.getTestEndpoint())")!
        let i = try checkedCast(prx: obj, type: IPrx.self)!

        let s1 = S(v: 0)
        let (s3, s2) = try i.opS(s1)
        try test(s2 == s1)
        try test(s3 == s1)

        let sseq1 = [s1]
        let (sseq3, sseq2) = try i.opSSeq(sseq1)
        try test(sseq2 == sseq1)
        try test(sseq3 == sseq1)

        let smap1 = ["a": s1]
        let (smap3, smap2) = try i.opSMap(smap1)
        try test(smap2 == smap1)
        try test(smap3 == smap1)

        let c1 = C(s: s1)
        let (c3, c2) = try i.opC(c1)
        try test(c2!.s == s1)
        try test(c3!.s == s1)

        let cseq1 = [c1]
        var (cseq3, cseq2) = try i.opCSeq(cseq1)
        try test(cseq2[0]!.s == s1)
        try test(cseq3[0]!.s == s1)

        let cmap1 = ["a": c1]
        let (cmap3, cmap2) = try i.opCMap(cmap1)
        try test(cmap2["a"]!!.s == s1)
        try test(cmap3["a"]!!.s == s1)
    }

    do {
        let obj = try communicator.stringToProxy("i1:\(helper.getTestEndpoint())")!
        let i = try checkedCast(prx: obj, type: IPrx.self)!

        let s1 = S(v: 0)
        let (s3, s2) = try i.opSAsync(s1).wait()
        try test(s2 == s1)
        try test(s3 == s1)

        let sseq1 = [s1]
        let (sseq3, sseq2) = try i.opSSeqAsync(sseq1).wait()
        try test(sseq2 == sseq1)
        try test(sseq3 == sseq1)

        let smap1 = ["a": s1]
        let (smap3, smap2) = try i.opSMapAsync(smap1).wait()
        try test(smap2 == smap1)
        try test(smap3 == smap1)

        let c1 = C(s: s1)
        let (c3, c2) = try i.opCAsync(c1).wait()
        try test(c2!.s == s1)
        try test(c3!.s == s1)

        let cseq1 = [c1]
        var (cseq3, cseq2) = try i.opCSeqAsync(cseq1).wait()
        try test(cseq2[0]!.s == s1)
        try test(cseq3[0]!.s == s1)

        let cmap1 = ["a": c1]
        let (cmap3, cmap2) = try i.opCMapAsync(cmap1).wait()
        try test(cmap2["a"]!!.s == s1)
        try test(cmap3["a"]!!.s == s1)
    }

    do {
        let obj = try communicator.stringToProxy("i2:\(helper.getTestEndpoint())")!
        let i = try checkedCast(prx: obj, type: InnerIPrx.self)!

        let s1 = InnerInner2S(v: 0)
        let (s3, s2) = try i.opS(s1)
        try test(s2 == s1)
        try test(s3 == s1)

        let sseq1 = [s1]
        let (sseq3, sseq2) = try i.opSSeq(sseq1)
        try test(sseq2 == sseq1)
        try test(sseq3 == sseq1)

        let smap1 = ["a": s1]
        let (smap3, smap2) = try i.opSMap(smap1)
        try test(smap2 == smap1)
        try test(smap3 == smap1)

        let c1 = InnerInner2C(s: s1)
        let (c3, c2) = try i.opC(c1)
        try test(c2!.s == c1.s)
        try test(c3!.s == c1.s)

        let cseq1 = [c1]
        let (cseq3, cseq2) = try i.opCSeq(cseq1)
        try test(cseq2[0]!.s == s1)
        try test(cseq3[0]!.s == s1)

        let cmap1 = ["a": c1]
        let (cmap3, cmap2) = try i.opCMap(cmap1)
        try test(cmap2["a"]!!.s == s1)
        try test(cmap3["a"]!!.s == s1)
    }

    do {
        let obj = try communicator.stringToProxy("i2:\(helper.getTestEndpoint())")!
        let i = try checkedCast(prx: obj, type: InnerIPrx.self)!

        let s1 = InnerInner2S(v: 0)
        let (s3, s2) = try i.opSAsync(s1).wait()
        try test(s2 == s1)
        try test(s3 == s1)

        let sseq1 = [s1]
        let (sseq3, sseq2) = try i.opSSeqAsync(sseq1).wait()
        try test(sseq2 == sseq1)
        try test(sseq3 == sseq1)

        let smap1 = ["a": s1]
        let (smap3, smap2) = try i.opSMapAsync(smap1).wait()
        try test(smap2 == smap1)
        try test(smap3 == smap1)

        let c1 = InnerInner2C(s: s1)
        let (c3, c2) = try i.opCAsync(c1).wait()
        try test(c2!.s == c1.s)
        try test(c3!.s == c1.s)

        let cseq1 = [c1]
        let (cseq3, cseq2) = try i.opCSeqAsync(cseq1).wait()
        try test(cseq2[0]!.s == s1)
        try test(cseq3[0]!.s == s1)

        let cmap1 = ["a": c1]
        let (cmap3, cmap2) = try i.opCMapAsync(cmap1).wait()
        try test(cmap2["a"]!!.s == s1)
        try test(cmap3["a"]!!.s == s1)
    }

    do {
        let obj = try communicator.stringToProxy("i3:\(helper.getTestEndpoint())")!
        let i = try checkedCast(prx: obj, type: InnerInner2IPrx.self)!

        let s1 = InnerInner2S(v: 0)
        let (s3, s2) = try i.opS(s1)
        try test(s2 == s1)
        try test(s3 == s1)

        let sseq1 = [s1]
        let (sseq3, sseq2) = try i.opSSeq(sseq1)
        try test(sseq2[0] == s1)
        try test(sseq3[0] == s1)

        let smap1 = ["a": s1]
        let (smap3, smap2) = try i.opSMap(smap1)
        try test(smap2 == smap1)
        try test(smap3 == smap1)

        let c1 = InnerInner2C(s: s1)
        let (c3, c2) = try i.opC(c1)
        try test(c2!.s == s1)
        try test(c3!.s == s1)

        let cseq1 = [c1]
        let (cseq3, cseq2) = try i.opCSeq(cseq1)
        try test(cseq2[0]!.s == s1)
        try test(cseq3[0]!.s == s1)

        let cmap1 = ["a": c1]
        let (cmap3, cmap2) = try i.opCMap(cmap1)
        try test(cmap2["a"]!!.s == s1)
        try test(cmap3["a"]!!.s == s1)
    }

    do {
        let obj = try communicator.stringToProxy("i3:\(helper.getTestEndpoint())")!
        let i = try checkedCast(prx: obj, type: InnerInner2IPrx.self)!

        let s1 = InnerInner2S(v: 0)
        let (s3, s2) = try i.opSAsync(s1).wait()
        try test(s2 == s1)
        try test(s3 == s1)

        let sseq1 = [s1]
        let (sseq3, sseq2) = try i.opSSeqAsync(sseq1).wait()
        try test(sseq2[0] == s1)
        try test(sseq3[0] == s1)

        let smap1 = ["a": s1]
        let (smap3, smap2) = try i.opSMapAsync(smap1).wait()
        try test(smap2 == smap1)
        try test(smap3 == smap1)

        let c1 = InnerInner2C(s: s1)
        let (c3, c2) = try i.opCAsync(c1).wait()
        try test(c2!.s == s1)
        try test(c3!.s == s1)

        let cseq1 = [c1]
        let (cseq3, cseq2) = try i.opCSeqAsync(cseq1).wait()
        try test(cseq2[0]!.s == s1)
        try test(cseq3[0]!.s == s1)

        let cmap1 = ["a": c1]
        let (cmap3, cmap2) = try i.opCMapAsync(cmap1).wait()
        try test(cmap2["a"]!!.s == s1)
        try test(cmap3["a"]!!.s == s1)
    }

    do {
        let obj = try communicator.stringToProxy("i4:\(helper.getTestEndpoint())")!
        let i = try checkedCast(prx: obj, type: InnerTestInner2IPrx.self)!

        let s1 = S(v: 0)
        let (s3, s2) = try i.opS(s1)
        try test(s2 == s1)
        try test(s3 == s1)

        let sseq1 = [s1]
        let (sseq3, sseq2) = try i.opSSeq(sseq1)
        try test(sseq2[0] == s1)
        try test(sseq3[0] == s1)

        let smap1 = ["a": s1]
        let (smap3, smap2) = try i.opSMap(smap1)
        try test(smap2 == smap1)
        try test(smap3 == smap1)

        let c1 = C(s: s1)
        let (c3, c2) = try i.opC(c1)
        try test(c2!.s == s1)
        try test(c3!.s == s1)

        let cseq1 = [c1]
        let (cseq3, cseq2) = try i.opCSeq(cseq1)
        try test(cseq2[0]!.s == s1)
        try test(cseq3[0]!.s == s1)

        let cmap1 = ["a": c1]
        let (cmap3, cmap2) = try i.opCMap(cmap1)
        try test(cmap2["a"]!!.s == s1)
        try test(cmap3["a"]!!.s == s1)
    }

    do {
        let obj = try communicator.stringToProxy("i4:\(helper.getTestEndpoint())")!
        let i = try checkedCast(prx: obj, type: InnerTestInner2IPrx.self)!

        let s1 = S(v: 0)
        let (s3, s2) = try i.opSAsync(s1).wait()
        try test(s2 == s1)
        try test(s3 == s1)

        let sseq1 = [s1]
        let (sseq3, sseq2) = try i.opSSeqAsync(sseq1).wait()
        try test(sseq2[0] == s1)
        try test(sseq3[0] == s1)

        let smap1 = ["a": s1]
        let (smap3, smap2) = try i.opSMapAsync(smap1).wait()
        try test(smap2 == smap1)
        try test(smap3 == smap1)

        let c1 = C(s: s1)
        let (c3, c2) = try i.opC(c1)
        try test(c2!.s == s1)
        try test(c3!.s == s1)

        let cseq1 = [c1]
        let (cseq3, cseq2) = try i.opCSeqAsync(cseq1).wait()
        try test(cseq2[0]!.s == s1)
        try test(cseq3[0]!.s == s1)

        let cmap1 = ["a": c1]
        let (cmap3, cmap2) = try i.opCMapAsync(cmap1).wait()
        try test(cmap2["a"]!!.s == s1)
        try test(cmap3["a"]!!.s == s1)
    }

    do {
        let obj = try communicator.stringToProxy("i1:\(helper.getTestEndpoint())")!
        let  i = try checkedCast(prx: obj, type: IPrx.self)!
        try i.shutdown()
    }
    output.writeLine("ok")
}
