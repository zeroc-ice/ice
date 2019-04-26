//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import PromiseKit

class TestI: TestIntf {
    var _helper: TestHelper

    init(_ helper: TestHelper) {
        _helper = helper
    }

    func SBaseAsObject(current: Current) throws -> Value? {
        return SBase(sb: "SBase.sb")
    }

    func SBaseAsSBase(current: Current) throws -> SBase? {
        return SBase(sb: "SBase.sb")
    }

    func SBSKnownDerivedAsSBase(current: Current) throws -> SBase? {
        return SBSKnownDerived(sb: "SBSKnownDerived.sb", sbskd: "SBSKnownDerived.sbskd")
    }

    func SBSKnownDerivedAsSBSKnownDerived(current: Current) throws -> SBSKnownDerived? {
        return SBSKnownDerived(sb: "SBSKnownDerived.sb", sbskd: "SBSKnownDerived.sbskd")
    }

    func SBSUnknownDerivedAsSBase(current: Current) throws -> SBase? {
        return SBSUnknownDerived(sb: "SBSUnknownDerived.sb", sbsud: "SBSUnknownDerived.sbsud")
    }

    func SBSUnknownDerivedAsSBaseCompact(current: Current) throws -> SBase? {
        return SBSUnknownDerived(sb: "SBSUnknownDerived.sb", sbsud: "SBSUnknownDerived.sbsud")
    }

    func SUnknownAsObject(current: Current) throws -> Value? {
        let su = SUnknown()
        su.su = "SUnknown.su"
        su.cycle = su
        return su
    }

    func checkSUnknown(o: Value?, current: Current) throws {
        if current.encoding == Ice.Encoding_1_0 {
            try _helper.test(!(o is SUnknown))
        } else {
            try _helper.test((o as! SUnknown).su == "SUnknown.su")
        }
    }

    func oneElementCycle(current: Current) throws -> B? {
        let b = B()
        b.sb = "B1.sb"
        b.pb = b
        return b
    }

    func twoElementCycle(current: Current) throws -> B? {
        let b1 = B()
        b1.sb = "B1.sb"
        let b2 = B()
        b2.sb = "B2.sb"
        b2.pb = b1
        b1.pb = b2
        return b1
    }

    func D1AsB(current: Current) throws -> B? {
        let d1 = D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        let d2 = D2()
        d2.pb = d1
        d2.sb = "D2.sb"
        d2.sd2 = "D2.sd2"
        d2.pd2 = d1
        d1.pb = d2
        d1.pd1 = d2
        return d1
    }

    func D1AsD1(current: Current) throws -> D1? {
        let d1 = D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        let d2 = D2()
        d2.pb = d1
        d2.sb = "D2.sb"
        d2.sd2 = "D2.sd2"
        d2.pd2 = d1
        d1.pb = d2
        d1.pd1 = d2
        return d1
    }

    func D2AsB(current: Current) throws -> B? {
        let d2 = D2()
        d2.sb = "D2.sb"
        d2.sd2 = "D2.sd2"
        let d1 = D1()
        d1.pb = d2
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d1.pd1 = d2
        d2.pb = d1
        d2.pd2 = d1
        return d2
    }

    func paramTest1(current: Current) throws -> (p1: B?, p2: B?) {
        let d1 = D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        let d2 = D2()
        d2.pb = d1
        d2.sb = "D2.sb"
        d2.sd2 = "D2.sd2"
        d2.pd2 = d1
        d1.pb = d2
        d1.pd1 = d2
        return (d1, d2)
    }

    func paramTest2(current: Current) throws -> (p2: B?, p1: B?) {
        let ret = try paramTest1(current: current)
        return (ret.p2, ret.p1)
    }

    func paramTest3(current: Current) throws -> (returnValue: B?, p1: B?, p2: B?) {
        let d2 = D2()
        d2.sb = "D2.sb (p1 1)"
        d2.pb = nil
        d2.sd2 = "D2.sd2 (p1 1)"

        let d1 = D1()
        d1.sb = "D1.sb (p1 2)"
        d1.pb = nil
        d1.sd1 = "D1.sd2 (p1 2)"
        d1.pd1 = nil
        d2.pd2 = d1

        let d4 = D2()
        d4.sb = "D2.sb (p2 1)"
        d4.pb = nil
        d4.sd2 = "D2.sd2 (p2 1)"

        let d3 = D1()
        d3.sb = "D1.sb (p2 2)"
        d3.pb = nil
        d3.sd1 = "D1.sd2 (p2 2)"
        d3.pd1 = nil
        d4.pd2 = d3

        return (d3, d2, d1)
    }

    func paramTest4(current: Current) throws -> (returnValue: B?, p: B?) {
        let d4 = D4()
        d4.sb = "D4.sb (1)"
        d4.pb = nil
        d4.p1 = B()
        d4.p1!.sb = "B.sb (1)"
        d4.p2 = B()
        d4.p2!.sb = "B.sb (2)"
        return (d4.p2, d4)
    }

    func returnTest1(current: Current) throws -> (returnValue: B?, p1: B?, p2: B?) {
        let ret = try paramTest1(current: current)
        return (ret.p1, ret.p2, ret.p1)
    }

    func returnTest2(current: Current) throws -> (returnValue: B?, p2: B?, p1: B?) {
        let ret = try paramTest1(current: current)
        return (ret.p1, ret.p2, ret.p1)
    }

    func returnTest3(p1: B?, p2: B?, current: Current) throws -> B? {
        return p1
    }

    func sequenceTest(p1: SS1?, p2: SS2?, current: Current) throws -> SS3 {
        let ss = SS3()
        ss.c1 = p1
        ss.c2 = p2
        return ss
    }

    func dictionaryTest(bin: BDict, current: Current) throws -> (returnValue: BDict, bout: BDict) {
        var bout = [Int32: B?]()
        for i: Int32 in 0..<10 {
            let b = bin[i]!!
            let d2 = D2()
            d2.sb = b.sb
            d2.pb = b.pb
            d2.sd2 = "D2"
            d2.pd2 = d2
            bout[i * 10] = d2
        }

        var r = [Int32: B]()
        for i: Int32 in 0..<10 {
            let s = "D1.\(i * 20)"
            let d1 = D1()
            d1.sb = s
            d1.pb = i == 0 ? nil : r[(i - 1) * 20]
            d1.sd1 = s
            d1.pd1 = d1
            r[i * 20] = d1
        }
        return (r, bout)
    }

    func exchangePBase(pb: PBase?, current: Current) throws -> PBase? {
        return pb
    }

    func PBSUnknownAsPreserved(current: Current) throws -> Preserved? {
        let r = PSUnknown()
        r.pi = 5
        r.ps = "preserved"
        r.psu = "unknown"
        r.graph = nil
        if current.encoding != Ice.Encoding_1_0 {
            //
            // 1.0 encoding doesn't support unmarshaling unknown classes even if referenced
            // from unread slice.
            //
            r.cl = MyClass(i: 15)
        }
        return r
    }

    func checkPBSUnknown(p: Preserved?, current: Current) throws {
        if current.encoding == Ice.Encoding_1_0 {
            try _helper.test(!(p is PSUnknown))
            try _helper.test(p!.pi == 5)
            try _helper.test(p!.ps == "preserved")
        } else {
            let pu = p as! PSUnknown
            try _helper.test(pu.pi == 5)
            try _helper.test(pu.ps == "preserved")
            try _helper.test(pu.psu == "unknown")
            try _helper.test(pu.graph == nil)
            try _helper.test(pu.cl != nil && pu.cl!.i == 15)
        }
    }

    func PBSUnknownAsPreservedWithGraphAsync(current: Current) -> Promise<Preserved?> {
        return Promise<Preserved?> { seal in
            let r = PSUnknown()
            r.pi = 5
            r.ps = "preserved"
            r.psu = "unknown"
            r.graph = PNode()
            r.graph!.next = PNode()
            r.graph!.next!.next = PNode()
            r.graph!.next!.next!.next = r.graph
            seal.fulfill(r)
        }
    }

    func checkPBSUnknownWithGraph(p: Preserved?, current: Current) throws {
        if current.encoding == Ice.Encoding_1_0 {
            try _helper.test(!(p is PSUnknown))
            try _helper.test(p!.pi == 5)
            try _helper.test(p!.ps == "preserved")
        } else {
            let pu = p as! PSUnknown
            try _helper.test(pu.pi == 5)
            try _helper.test(pu.ps == "preserved")
            try _helper.test(pu.psu == "unknown")
            try _helper.test(pu.graph !== pu.graph!.next)
            try _helper.test(pu.graph!.next !== pu.graph!.next!.next)
            try _helper.test(pu.graph!.next!.next!.next === pu.graph)
        }
    }

    func PBSUnknown2AsPreservedWithGraphAsync(current: Current) -> Promise<Preserved?> {
        return Promise<Preserved?> { seal in
            let r = PSUnknown2()
            r.pi = 5
            r.ps = "preserved"
            r.pb = r
            seal.fulfill(r)
        }
    }

    func checkPBSUnknown2WithGraph(p: Preserved?, current: Current) throws {
        if current.encoding == Ice.Encoding_1_0 {
            try _helper.test(!(p is PSUnknown2))
            try _helper.test(p!.pi == 5)
            try _helper.test(p!.ps == "preserved")
        } else {
            let pu = p as! PSUnknown2
            try _helper.test(pu.pi == 5)
            try _helper.test(pu.ps == "preserved")
            try _helper.test(pu.pb === pu)
        }
    }

    func exchangePNode(pn: PNode?, current: Current) throws -> PNode? {
        return pn
    }

    func throwBaseAsBase(current: Current) throws {
        let be = BaseException()
        be.sbe = "sbe"
        be.pb = B()
        be.pb!.sb = "sb"
        be.pb!.pb = be.pb
        throw be
    }

    func throwDerivedAsBase(current: Current) throws {
        let de = DerivedException()
        de.sbe = "sbe"
        de.pb = B()
        de.pb!.sb = "sb1"
        de.pb!.pb = de.pb
        de.sde = "sde1"
        de.pd1 = D1()
        de.pd1!.sb = "sb2"
        de.pd1!.pb = de.pd1
        de.pd1!.sd1 = "sd2"
        de.pd1!.pd1 = de.pd1
        throw de
    }

    func throwDerivedAsDerived(current: Current) throws {
        let de = DerivedException()
        de.sbe = "sbe"
        de.pb = B()
        de.pb!.sb = "sb1"
        de.pb!.pb = de.pb
        de.sde = "sde1"
        de.pd1 = D1()
        de.pd1!.sb = "sb2"
        de.pd1!.pb = de.pd1
        de.pd1!.sd1 = "sd2"
        de.pd1!.pd1 = de.pd1
        throw de
    }

    func throwUnknownDerivedAsBase(current: Current) throws {
        let d2 = D2()
        d2.sb = "sb d2"
        d2.pb = d2
        d2.sd2 = "sd2 d2"
        d2.pd2 = d2

        let ude = UnknownDerivedException()
        ude.sbe = "sbe"
        ude.pb = d2
        ude.sude = "sude"
        ude.pd2 = d2
        throw ude
    }

    func throwPreservedExceptionAsync(current: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            let ue = PSUnknownException()
            ue.p = PSUnknown2()
            ue.p!.pi = 5
            ue.p!.ps = "preserved"
            ue.p!.pb = ue.p
            throw ue
        }
    }

    func useForward(current: Current) throws -> Forward? {
        let f = Forward()
        f.h = Hidden()
        f.h!.f = f
        return f
    }

    func shutdown(current: Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }
}
