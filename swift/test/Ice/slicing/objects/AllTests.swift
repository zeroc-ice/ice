// Copyright (c) ZeroC, Inc.

import Dispatch
import Foundation
import Ice
import TestCommon

private func breakCycles(_ value: Value) {
    if let d1 = value as? D1 {
        let tmp = d1.pd1
        d1.pd1 = nil
        if tmp != nil, tmp! !== d1 {
            breakCycles(tmp!)
        }
    }
    if let d2 = value as? D2 {
        d2.pd2 = nil
    }
    if let d3 = value as? D3 {
        let tmp = d3.pd3
        d3.pd3 = nil
        if tmp != nil, tmp! !== d3 {
            breakCycles(tmp!)
        }
    }
    if let d4 = value as? D4 {
        d4.p1 = nil
        d4.p2 = nil
    }
    if let b = value as? B {
        b.pb?.pb = nil
        b.pb = nil
        let tmp = b.pb
        b.pb = nil
        if tmp != nil, tmp! !== b {
            breakCycles(tmp!)
        }
        if b.ice_getSlicedData() != nil {
            b.ice_getSlicedData()?.clear()
        }
    }
    if let p = value as? Preserved {
        p.ice_getSlicedData()?.clear()
    }
    if let p = value as? PDerived {
        p.pb = nil
    }
    if let p = value as? CompactPDerived {
        p.pb = nil
    }
    if let p = value as? PCDerived {
        let a = p.pbs
        p.pbs = []
        for e in a {
            if let elt = e {
                breakCycles(elt)
            }
        }
    }
    if let p = value as? CompactPCDerived {
        let a = p.pbs
        p.pbs = []
        for e in a {
            if let elt = e {
                breakCycles(elt)
            }
        }
    }
    if var curr = value as? PNode {
        while curr.next != nil, value !== curr.next {
            curr = curr.next!
        }
        curr.next = nil
    }
    if let p = value as? PSUnknown, p.graph != nil {
        breakCycles(p.graph!)
    }
    if let p = value as? PSUnknown2 {
        p.pb = nil
    }
    if let s = value as? SS1 {
        for b in s.s where b != nil {
            breakCycles(b!)
        }
    }
    if let s = value as? SS2 {
        for b in s.s where b != nil {
            breakCycles(b!)
        }
    }
    if let f = value as? Forward {
        f.h = nil
    }
    if let u = value as? SUnknown {
        u.cycle = nil
    }
}

public func allTests(_ helper: TestHelper) async throws -> TestIntfPrx {
    func test(_ value: Bool, file: String = #file, line: Int = #line) throws {
        try helper.test(value, file: file, line: line)
    }

    let output = helper.getWriter()
    let communicator = helper.communicator()

    output.write("testing stringToProxy... ")
    let basePrx = try communicator.stringToProxy("Test:\(helper.getTestEndpoint(num: 0)) -t 2000")!
    output.writeLine("ok")

    output.write("testing checked cast... ")
    let testPrx = try await checkedCast(prx: basePrx, type: TestIntfPrx.self)!
    try test(testPrx == basePrx)
    output.writeLine("ok")

    output.write("base as Object... ")
    do {
        let o = try await testPrx.SBaseAsObject()
        let sb = o as! SBase
        try test(sb.ice_id() == "::Test::SBase")
        try test(sb.sb == "SBase.sb")
        breakCycles(sb)
    }
    output.writeLine("ok")

    output.write("base as base... ")
    do {
        let sb = try await testPrx.SBaseAsSBase()!
        try test(sb.sb == "SBase.sb")
        breakCycles(sb)
    }
    output.writeLine("ok")

    output.write("base with known derived as base... ")
    do {
        let sb = try await testPrx.SBSKnownDerivedAsSBase()!
        try test(sb.sb == "SBSKnownDerived.sb")
        let sbskd = sb as! SBSKnownDerived
        try test(sbskd.sbskd == "SBSKnownDerived.sbskd")
        breakCycles(sbskd)
    }
    output.writeLine("ok")

    output.write("base with known derived as known derived... ")
    do {
        let sbskd = try await testPrx.SBSKnownDerivedAsSBSKnownDerived()!
        try test(sbskd.sbskd == "SBSKnownDerived.sbskd")
        breakCycles(sbskd)
    }
    output.writeLine("ok")

    output.write("base with unknown derived as base... ")
    do {
        let sb = try await testPrx.SBSUnknownDerivedAsSBase()!
        try test(sb.sb == "SBSUnknownDerived.sb")
        breakCycles(sb)
    }

    if testPrx.ice_getEncodingVersion() == Ice.Encoding_1_0 {
        do {
            let sb = try await testPrx.SBSUnknownDerivedAsSBaseCompact()!
            try test(sb.sb == "SBSUnknownDerived.sb")
            breakCycles(sb)
        }
    } else {
        do {
            //
            // This test fails when using the compact format because the instance cannot
            // be sliced to a known type.
            //
            _ = try await testPrx.SBSUnknownDerivedAsSBaseCompact()
            try test(false)
        } catch is Ice.MarshalException {  // Expected.
        }
    }
    output.writeLine("ok")

    output.write("unknown with Object as Object... ")
    do {
        let o = try await testPrx.SUnknownAsObject()!
        try test(testPrx.ice_getEncodingVersion() != Ice.Encoding_1_0)
        try test(o is Ice.UnknownSlicedValue)
        try test((o as! Ice.UnknownSlicedValue).ice_id() == "::Test::SUnknown")
        try test((o as! Ice.UnknownSlicedValue).ice_getSlicedData() != nil)
        try await testPrx.checkSUnknown(o)
        (o as! Ice.UnknownSlicedValue).ice_getSlicedData()!.clear()
    } catch is Ice.MarshalException {
        try test(testPrx.ice_getEncodingVersion() == Ice.Encoding_1_0)
    }
    output.writeLine("ok")

    output.write("one-element cycle... ")
    do {
        let b = try await testPrx.oneElementCycle()!
        try test(b.ice_id() == "::Test::B")
        try test(b.sb == "B1.sb")
        try test(b.pb === b)
        breakCycles(b)
    }
    output.writeLine("ok")

    output.write("two-element cycle... ")
    do {
        let b1 = try await testPrx.twoElementCycle()!
        try test(b1.ice_id() == "::Test::B")
        try test(b1.sb == "B1.sb")
        let b2 = b1.pb!
        try test(b2.ice_id() == "::Test::B")
        try test(b2.sb == "B2.sb")
        try test(b2.pb === b1)
        breakCycles(b1)
    }
    output.writeLine("ok")

    output.write("known derived pointer slicing as base... ")
    do {
        let b1 = try await testPrx.D1AsB()!
        try test(b1.ice_id() == "::Test::D1")
        try test(b1.sb == "D1.sb")
        try test(b1.pb !== nil)
        try test(b1.pb !== b1)
        if let d1 = b1 as? D1 {
            try test(d1.sd1 == "D1.sd1")
            try test(d1.pd1 !== nil)
            try test(d1.pd1 !== b1)
            try test(b1.pb === d1.pd1)
        } else {
            try test(false)
        }

        let b2 = b1.pb!
        try test(b2.pb === b1)
        try test(b2.sb == "D2.sb")
        try test(b2.ice_id() == "::Test::B")
        breakCycles(b1)
    }
    output.writeLine("ok")

    output.write("known derived pointer slicing as derived... ")
    do {
        let d1 = try await testPrx.D1AsD1()!
        try test(d1.ice_id() == "::Test::D1")
        try test(d1.sb == "D1.sb")
        try test(d1.pb !== nil)
        try test(d1.pb !== d1)

        let b2 = d1.pb!
        try test(b2.ice_id() == "::Test::B")
        try test(b2.sb == "D2.sb")
        try test(b2.pb === d1)
        breakCycles(d1)
    }
    output.writeLine("ok")

    output.write("unknown derived pointer slicing as base... ")
    do {
        let b2 = try await testPrx.D2AsB()!
        try test(b2.ice_id() == "::Test::B")
        try test(b2.sb == "D2.sb")
        try test(b2.pb !== nil)
        try test(b2.pb !== b2)

        let b1 = b2.pb!
        try test(b1.ice_id() == "::Test::D1")
        try test(b1.sb == "D1.sb")
        try test(b1.pb === b2)
        if let d1 = b1 as? D1 {
            try test(d1.sd1 == "D1.sd1")
            try test(d1.pd1 === b2)
            breakCycles(b2)
        } else {
            try test(false)
        }
    }
    output.writeLine("ok")

    output.write("param ptr slicing with known first... ")
    do {
        let (b1, b2) = try await testPrx.paramTest1()
        try test(b1 !== nil)
        try test(b1!.ice_id() == "::Test::D1")
        try test(b1!.sb == "D1.sb")
        try test(b1!.pb === b2)
        let d1 = b1 as! D1
        try test(d1.sd1 == "D1.sd1")
        try test(d1.pd1 === b2)

        try test(b2 !== nil)
        // No factory, must be sliced
        try test(b2!.ice_id() == "::Test::B")
        try test(b2!.sb == "D2.sb")
        try test(b2!.pb === b1)
        breakCycles(b1!)
        breakCycles(b2!)
    }
    output.writeLine("ok")

    output.write("param ptr slicing with unknown first... ")
    do {
        let (o2, o1) = try await testPrx.paramTest2()
        try test(o1 != nil && o2 != nil)

        let b1 = o1!
        let b2 = o2!

        try test(b1.ice_id() == "::Test::D1")
        try test(b1.sb == "D1.sb")
        try test(b1.pb === b2)

        if let d1 = b1 as? D1 {
            try test(d1.sd1 == "D1.sd1")
            try test(d1.pd1 === b2)
        } else {
            try test(false)
        }
        // No factory, must be sliced
        try test(b2.ice_id() == "::Test::B")
        try test(b2.sb == "D2.sb")
        try test(b2.pb === b1)
        breakCycles(b1)
        breakCycles(b2)
    }
    output.writeLine("ok")

    output.write("return value identity with known first... ")
    do {
        let (ret, p1, p2) = try await testPrx.returnTest1()
        try test(ret === p1)
        breakCycles(ret!)
        breakCycles(p2!)
    }
    output.writeLine("ok")

    output.write("return value identity with unknown first... ")
    do {
        let (ret, p1, _) = try await testPrx.returnTest2()
        try test(ret === p1)
        breakCycles(ret!)
    }
    output.writeLine("ok")

    output.write("return value identity for input params known first... ")
    do {
        let d1 = D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        let d3 = D3()
        d3.pb = d1
        d3.sb = "D3.sb"
        d3.sd3 = "D3.sd3"
        d3.pd3 = d1
        d1.pb = d3
        d1.pd1 = d3

        let b1 = try await testPrx.returnTest3(p1: d1, p2: d3)!
        try test(b1.sb == "D1.sb")
        try test(b1.ice_id() == "::Test::D1")

        let p1 = b1 as? D1
        if p1 !== nil {
            try test(p1!.sd1 == "D1.sd1")
            try test(p1!.pd1 === b1.pb)
        } else {
            try test(false)
        }

        let b2 = b1.pb!
        try test(b2.sb == "D3.sb")
        try test(b2.pb === b1)

        if testPrx.ice_getEncodingVersion() == Ice.Encoding_1_0 {
            try test(!(b2 is D3))
        } else {
            if let p3 = b2 as? D3 {
                try test(p3.pd3 === p1!)
                try test(p3.sd3 == "D3.sd3")
            } else {
                try test(false)
            }
        }

        try test(b1 !== d1)
        try test(b1 !== d3)
        try test(b2 !== d1)
        try test(b2 !== d3)
        breakCycles(b1)
        breakCycles(d1)
        breakCycles(d3)
    }
    output.writeLine("ok")

    output.write("return value identity for input params unknown first... ")
    do {
        let d1 = D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        let d3 = D3()
        d3.pb = d1
        d3.sb = "D3.sb"
        d3.sd3 = "D3.sd3"
        d3.pd3 = d1
        d1.pb = d3
        d1.pd1 = d3

        let b1 = try await testPrx.returnTest3(p1: d3, p2: d1)!

        try test(b1.sb == "D3.sb")

        let b2 = b1.pb!
        try test(b2.sb == "D1.sb")
        try test(b2.ice_id() == "::Test::D1")
        try test(b2.pb === b1)
        if let p3 = b2 as? D1 {
            try test(p3.sd1 == "D1.sd1")
            try test(p3.pd1 === b1)
        } else {
            try test(false)
        }

        if testPrx.ice_getEncodingVersion() == Ice.Encoding_1_0 {
            try test(!(b1 is D3))
        } else {
            if let p1 = b1 as? D3 {
                try test(p1.sd3 == "D3.sd3")
                try test(p1.pd3 === b2)
            } else {
                try test(false)
            }
        }

        try test(b1 !== d1)
        try test(b1 !== d3)
        try test(b2 !== d1)
        try test(b2 !== d3)
        breakCycles(b1)
        breakCycles(d1)
        breakCycles(d3)
    }
    output.writeLine("ok")

    output.write("remainder unmarshaling (3 instances)... ")
    do {
        let (ret1, o1, o2) = try await testPrx.paramTest3()
        try test(o1 != nil)
        let p1 = o1!
        try test(p1.sb == "D2.sb (p1 1)")
        try test(p1.pb == nil)
        try test(p1.ice_id() == "::Test::B")

        try test(o2 != nil)
        let p2 = o2!
        try test(p2.sb == "D2.sb (p2 1)")
        try test(p2.pb == nil)
        try test(p2.ice_id() == "::Test::B")

        try test(ret1 != nil)
        let ret = ret1!
        try test(ret.sb == "D1.sb (p2 2)")
        try test(ret.pb == nil)
        try test(ret.ice_id() == "::Test::D1")
        breakCycles(ret)
        breakCycles(p1)
        breakCycles(p2)
    }
    output.writeLine("ok")

    output.write("remainder unmarshaling (4 instances)... ")
    do {
        let (ret1, b1) = try await testPrx.paramTest4()

        try test(b1 != nil)
        let b = b1!
        try test(b.sb == "D4.sb (1)")
        try test(b.pb == nil)
        try test(b.ice_id() == "::Test::B")

        try test(ret1 != nil)
        let ret = ret1!
        try test(ret.sb == "B.sb (2)")
        try test(ret.pb == nil)
        try test(ret.ice_id() == "::Test::B")
        breakCycles(ret)
        breakCycles(b)
    }
    output.writeLine("ok")

    output.write("param ptr slicing, instance marshaled in unknown derived as base... ")
    do {
        let b1 = B()
        b1.sb = "B.sb(1)"
        b1.pb = b1

        let d3 = D3()
        d3.sb = "D3.sb"
        d3.pb = d3
        d3.sd3 = "D3.sd3"
        d3.pd3 = b1

        let b2 = B()
        b2.sb = "B.sb(2)"
        b2.pb = b1

        let ret = try await testPrx.returnTest3(p1: d3, p2: b2)!

        try test(ret.sb == "D3.sb")
        try test(ret.pb === ret)

        if testPrx.ice_getEncodingVersion() == Ice.Encoding_1_0 {
            try test(!(ret is D3))
        } else {
            if let p3 = ret as? D3 {
                try test(p3.sd3 == "D3.sd3")
                try test(p3.pd3!.ice_id() == "::Test::B")
                try test(p3.pd3!.sb == "B.sb(1)")
                try test(p3.pd3!.pb === p3.pd3)
            } else {
                try test(false)
            }
        }

        breakCycles(ret)
        breakCycles(b1)
        breakCycles(d3)
    }
    output.writeLine("ok")

    output.write("param ptr slicing, instance marshaled in unknown derived as derived... ")
    do {
        let d11 = D1()
        d11.sb = "D1.sb(1)"
        d11.pb = d11
        d11.sd1 = "D1.sd1(1)"

        let d3 = D3()
        d3.sb = "D3.sb"
        d3.pb = d3
        d3.sd3 = "D3.sd3"
        d3.pd3 = d11

        let d12 = D1()
        d12.sb = "D1.sb(2)"
        d12.pb = d12
        d12.sd1 = "D1.sd1(2)"
        d12.pd1 = d11

        let ret = try await testPrx.returnTest3(p1: d3, p2: d12)!
        try test(ret.sb == "D3.sb")
        try test(ret.pb === ret)
        breakCycles(d3)
        breakCycles(d11)
        breakCycles(d12)
        breakCycles(ret)
    }
    output.writeLine("ok")

    output.write("sequence slicing... ")
    do {
        let ss1b = B()
        ss1b.sb = "B.sb"
        ss1b.pb = ss1b

        let ss1d1 = D1()
        ss1d1.sb = "D1.sb"
        ss1d1.sd1 = "D1.sd1"
        ss1d1.pb = ss1b

        let ss1d3 = D3()
        ss1d3.sb = "D3.sb"
        ss1d3.sd3 = "D3.sd3"
        ss1d3.pb = ss1b

        let ss2b = B()
        ss2b.sb = "B.sb"
        ss2b.pb = ss1b

        let ss2d1 = D1()
        ss2d1.sb = "D1.sb"
        ss2d1.sd1 = "D1.sd1"
        ss2d1.pb = ss2b

        let ss2d3 = D3()
        ss2d3.sb = "D3.sb"
        ss2d3.sd3 = "D3.sd3"
        ss2d3.pb = ss2b

        ss1d1.pd1 = ss2b
        ss1d3.pd3 = ss2d1

        ss2d1.pd1 = ss1d3
        ss2d3.pd3 = ss1d1

        let ss1 = SS1()
        ss1.s = [ss1b, ss1d1, ss1d3]

        let ss2 = SS2()
        ss2.s = [ss2b, ss2d1, ss2d3]

        let ss = try await testPrx.sequenceTest(p1: ss1, p2: ss2)
        breakCycles(ss1)
        breakCycles(ss2)

        try test(ss.c1 != nil)
        let ss1b2 = ss.c1!.s[0]
        let ss1d2 = ss.c1!.s[1]
        try test(ss.c2 != nil)
        let ss1d4 = ss.c1!.s[2]

        try test(ss.c2 != nil)
        let ss2b2 = ss.c2!.s[0]
        let ss2d2 = ss.c2!.s[1]
        let ss2d4 = ss.c2!.s[2]

        try test(ss1b2!.pb === ss1b2)
        try test(ss1d2!.pb === ss1b2)
        try test(ss1d4!.pb === ss1b2)

        try test(ss2b2!.pb === ss1b2)
        try test(ss2d2!.pb === ss2b2)
        try test(ss2d4!.pb === ss2b2)

        try test(ss1b2!.ice_id() == "::Test::B")
        try test(ss1d2!.ice_id() == "::Test::D1")

        try test(ss2b2!.ice_id() == "::Test::B")
        try test(ss2d2!.ice_id() == "::Test::D1")

        if testPrx.ice_getEncodingVersion() == Ice.Encoding_1_0 {
            try test(ss1d4!.ice_id() == "::Test::B")
            try test(ss2d4!.ice_id() == "::Test::B")
        } else {
            try test(ss1d4!.ice_id() == "::Test::D3")
            try test(ss2d4!.ice_id() == "::Test::D3")
        }

        breakCycles(ss.c1!)
        breakCycles(ss.c2!)
    }
    output.writeLine("ok")

    output.write("dictionary slicing... ")
    do {
        var bin = [Int32: B]()

        for i: Int32 in 0..<10 {
            let s = "D1.\(i)"
            let d1 = D1()
            d1.sb = s
            d1.pb = d1
            d1.sd1 = s
            bin[i] = d1
        }

        let (ret, bout) = try await testPrx.dictionaryTest(bin)

        try test(bout.count == 10)
        for i: Int32 in 0..<10 {
            let b = bout[i * 10]!!
            let s = "D1.\(i)"
            try test(b.sb == s)
            try test(b.pb !== nil)
            try test(b.pb !== b)
            try test(b.pb!.sb == s)
            try test(b.pb!.pb === b.pb)
        }

        try test(ret.count == 10)
        for i: Int32 in 0..<10 {
            let b = ret[i * 20]!!
            let s = "D1.\(i * 20)"
            try test(b.sb == s)

            if i == 0 {
                try test(b.pb == nil)
            } else {
                try test(b.pb === ret[(i - 1) * 20]!)
            }

            if let d1 = b as? D1 {
                try test(d1.sd1 == s)
                try test(d1.pd1 === d1)
            } else {
                try test(false)
            }
        }

        for i in bin {
            breakCycles(i.value)
        }
        for i in bout where i.value != nil {
            breakCycles(i.value!)
        }
        for i in ret where i.value != nil {
            breakCycles(i.value!)
        }
    }
    output.writeLine("ok")

    output.write("base exception thrown as base exception... ")
    do {
        try await testPrx.throwBaseAsBase()
        try test(false)
    } catch let e as BaseException {
        try test(e.sbe == "sbe")
        try test(e.pb != nil)
        try test(e.pb!.sb == "sb")
        try test(e.pb!.pb === e.pb)
        breakCycles(e.pb!)
    }
    output.writeLine("ok")

    output.write("derived exception thrown as base exception... ")
    do {
        try await testPrx.throwDerivedAsBase()
        try test(false)
    } catch let e as DerivedException {
        try test(e.sbe == "sbe")
        try test(e.pb != nil)
        try test(e.pb!.sb == "sb1")
        try test(e.pb!.pb === e.pb)
        try test(e.sde == "sde1")
        try test(e.pd1 != nil)
        try test(e.pd1!.sb == "sb2")
        try test(e.pd1!.pb === e.pd1)
        try test(e.pd1!.sd1 == "sd2")
        try test(e.pd1!.pd1 === e.pd1)
        breakCycles(e.pb!)
        breakCycles(e.pd1!)
    }
    output.writeLine("ok")

    output.write("derived exception thrown as derived exception... ")
    do {
        try await testPrx.throwDerivedAsDerived()
        try test(false)
    } catch let e as DerivedException {
        try test(e.sbe == "sbe")
        try test(e.pb != nil)
        try test(e.pb!.sb == "sb1")
        try test(e.pb!.pb === e.pb)
        try test(e.sde == "sde1")
        try test(e.pd1 != nil)
        try test(e.pd1!.sb == "sb2")
        try test(e.pd1!.pb === e.pd1)
        try test(e.pd1!.sd1 == "sd2")
        try test(e.pd1!.pd1 === e.pd1)
        breakCycles(e.pb!)
        breakCycles(e.pd1!)
    }
    output.writeLine("ok")

    output.write("unknown derived exception thrown as base exception... ")
    do {
        try await testPrx.throwUnknownDerivedAsBase()
        try test(false)
    } catch let e as BaseException {
        try test(e.sbe == "sbe")
        try test(e.pb != nil)
        try test(e.pb!.sb == "sb d2")
        try test(e.pb!.pb === e.pb)
        breakCycles(e.pb!)
    }
    output.writeLine("ok")

    output.write("forward-declared class... ")
    do {
        let f = try await testPrx.useForward()
        try test(f != nil)
        breakCycles(f!)
    }
    output.writeLine("ok")

    output.write("preserved classes... ")
    do {
        //
        // Server knows the most-derived class PDerived.
        //
        let pd = PDerived()
        pd.pi = 3
        pd.ps = "preserved"
        pd.pb = pd

        let r = try await testPrx.exchangePBase(pd)
        if let p2 = r as? PDerived {
            try test(p2.pi == 3)
            try test(p2.ps == "preserved")
            try test(p2.pb === p2)
            breakCycles(r!)
            breakCycles(pd)
        } else {
            try test(false)
        }
    } catch is Ice.OperationNotExistException {}

    do {
        //
        // Server only knows the base (non-preserved) type, so the object is sliced.
        //
        let pu = PCUnknown()
        pu.pi = 3
        pu.pu = "preserved"

        let r = try await testPrx.exchangePBase(pu)!
        try test(r.pi == 3)

        if testPrx.ice_getEncodingVersion() == Ice.Encoding_1_0 {
            try test(!(r is PCUnknown))
        } else {
            if let p2 = r as? PCUnknown {
                try test(p2.pu == "preserved")
            } else {
                try test(false)
            }
        }

        breakCycles(r)
    } catch is Ice.OperationNotExistException {}

    do {
        //
        // Server only knows the intermediate type Preserved. The object will be sliced to
        // Preserved for the 1.0 encoding; otherwise it should be returned intact.
        //
        let pcd = PCDerived()
        pcd.pi = 3
        pcd.pbs = [pcd]

        let r = try await testPrx.exchangePBase(pcd)!
        if testPrx.ice_getEncodingVersion() == Ice.Encoding_1_0 {
            try test(!(r is PCDerived))
            try test(r.pi == 3)
        } else {
            if let p2 = r as? PCDerived {
                try test(p2.pi == 3)
                try test(p2.pbs[0] === p2)
            } else {
                try test(false)
            }
        }
        breakCycles(r)
        breakCycles(pcd)
    } catch is Ice.OperationNotExistException {}

    do {
        //
        // Server only knows the intermediate type Preserved. The object will be sliced to
        // Preserved for the 1.0 encoding; otherwise it should be returned intact.
        //
        let pcd = CompactPCDerived()
        pcd.pi = 3
        pcd.pbs = [pcd]

        let r = try await testPrx.exchangePBase(pcd)!
        if testPrx.ice_getEncodingVersion() == Ice.Encoding_1_0 {
            try test(!(r is CompactPCDerived))
            try test(r.pi == 3)
        } else {
            if let p2 = r as? CompactPCDerived {
                try test(p2.pi == 3)
                try test(p2.pbs[0] === p2)
            } else {
                try test(false)
            }
        }

        breakCycles(r)
        breakCycles(pcd)
    } catch is Ice.OperationNotExistException {}

    do {
        //
        // Send an object that will have multiple preserved slices in the server.
        // The object will be sliced to Preserved for the 1.0 encoding.
        //
        let pcd = PCDerived3()
        pcd.pi = 3
        //
        // Sending more than 254 objects exercises the encoding for object ids.
        //
        pcd.pbs = [PBase]()
        for i: Int32 in 0..<300 {
            let p2 = PCDerived2()
            p2.pi = i
            p2.pbs = [nil]  // Nil reference. This slice should not have an indirection table.
            p2.pcd2 = i
            pcd.pbs.append(p2)
        }
        pcd.pcd2 = pcd.pi
        pcd.pcd3 = pcd.pbs[10]

        let r = try await testPrx.exchangePBase(pcd)!
        if testPrx.ice_getEncodingVersion() == Ice.Encoding_1_0 {
            try test(!(r is PCDerived3))
            try test(r is Preserved)
            try test(r.pi == 3)
        } else {
            if let p3 = r as? PCDerived3 {
                try test(p3.pi == 3)
                for i in 0..<300 {
                    if let p2 = p3.pbs[i] as? PCDerived2 {
                        try test(p2.pi == i)
                        try test(p2.pbs.count == 1)
                        try test(p2.pbs[0] == nil)
                        try test(p2.pcd2 == i)
                    } else {
                        try test(false)
                    }
                }
                try test(p3.pcd2 == p3.pi)
                try test(p3.pcd3 === p3.pbs[10])
            } else {
                try test(false)
            }
        }
        breakCycles(r)
        breakCycles(pcd)
    } catch is Ice.OperationNotExistException {}

    do {
        //
        // Obtain an object with preserved slices and send it back to the server.
        // The preserved slices should be excluded for the 1.0 encoding, otherwise
        // they should be included.
        //
        let p = try await testPrx.PBSUnknownAsPreserved()!
        try await testPrx.checkPBSUnknown(p)
        if testPrx.ice_getEncodingVersion() != Ice.Encoding_1_0 {
            let slicedData = p.ice_getSlicedData()!
            try test(slicedData.slices.count == 1)
            try await testPrx.ice_encodingVersion(Ice.Encoding_1_0).checkPBSUnknown(p)
        } else {
            try test(p.ice_getSlicedData() == nil)
        }
        breakCycles(p)
    } catch is Ice.OperationNotExistException {}
    output.writeLine("ok")

    return testPrx
}
