//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import PromiseKit
import TestCommon

class TestI: TestIntf {
    var _helper: TestHelper

    init(_ helper: TestHelper) {
        _helper = helper
    }

    func SBaseAsObjectAsync(current _: Current) -> Promise<Value?> {
        return Promise<Value?> { seal in
            seal.fulfill(SBase(sb: "SBase.sb"))
        }
    }

    func SBaseAsSBaseAsync(current _: Current) -> Promise<SBase?> {
        return Promise<SBase?> { seal in
            seal.fulfill(SBase(sb: "SBase.sb"))
        }
    }

    func SBSKnownDerivedAsSBaseAsync(current _: Current) -> Promise<SBase?> {
        return Promise<SBase?> { seal in
            seal.fulfill(SBSKnownDerived(sb: "SBSKnownDerived.sb", sbskd: "SBSKnownDerived.sbskd"))
        }
    }

    func SBSKnownDerivedAsSBSKnownDerivedAsync(current _: Current) -> Promise<SBSKnownDerived?> {
        return Promise<SBSKnownDerived?> { seal in
            seal.fulfill(SBSKnownDerived(sb: "SBSKnownDerived.sb", sbskd: "SBSKnownDerived.sbskd"))
        }
    }

    func SBSUnknownDerivedAsSBaseAsync(current _: Current) -> Promise<SBase?> {
        return Promise<SBase?> { seal in
            seal.fulfill(SBSUnknownDerived(sb: "SBSUnknownDerived.sb", sbsud: "SBSUnknownDerived.sbsud"))
        }
    }

    func SBSUnknownDerivedAsSBaseCompactAsync(current _: Current) -> Promise<SBase?> {
        return Promise<SBase?> { seal in
            seal.fulfill(SBSUnknownDerived(sb: "SBSUnknownDerived.sb", sbsud: "SBSUnknownDerived.sbsud"))
        }
    }

    func SUnknownAsObjectAsync(current _: Current) -> Promise<Value?> {
        return Promise<Value?> { seal in
            let su = SUnknown()
            su.su = "SUnknown.su"
            su.cycle = su
            seal.fulfill(su)
        }
    }

    func checkSUnknownAsync(o: Value?, current: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            if current.encoding == Ice.Encoding_1_0 {
                try _helper.test(!(o is SUnknown))
            } else {
                try _helper.test((o as! SUnknown).su == "SUnknown.su")
            }
        }
    }

    func oneElementCycleAsync(current _: Current) -> Promise<B?> {
        return Promise<B?> { seal in
            let b = B()
            b.sb = "B1.sb"
            b.pb = b
            seal.fulfill(b)
        }
    }

    func twoElementCycleAsync(current _: Current) -> Promise<B?> {
        return Promise<B?> { seal in
            let b1 = B()
            b1.sb = "B1.sb"
            let b2 = B()
            b2.sb = "B2.sb"
            b2.pb = b1
            b1.pb = b2
            seal.fulfill(b1)
        }
    }

    func D1AsBAsync(current _: Current) -> Promise<B?> {
        return Promise<B?> { seal in
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
            seal.fulfill(d1)
        }
    }

    func D1AsD1Async(current _: Current) -> Promise<D1?> {
        return Promise<D1?> { seal in
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
            seal.fulfill(d1)
        }
    }

    func D2AsBAsync(current _: Current) -> Promise<B?> {
        return Promise<B?> { seal in
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
            seal.fulfill(d2)
        }
    }

    func paramTest1Async(current _: Current) -> Promise<(p1: B?, p2: B?)> {
        return Promise<(p1: B?, p2: B?)> { seal in
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
            seal.fulfill((d1, d2))
        }
    }

    func paramTest2Async(current _: Current) -> Promise<(p2: B?, p1: B?)> {
        return Promise<(p2: B?, p1: B?)> { seal in
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
            seal.fulfill((d2, d1))
        }
    }

    func paramTest3Async(current _: Current) -> Promise<(returnValue: B?, p1: B?, p2: B?)> {
        return Promise<(returnValue: B?, p1: B?, p2: B?)> { seal in
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

            seal.fulfill((d3, d2, d4))
        }
    }

    func paramTest4Async(current _: Current) -> Promise<(returnValue: B?, p: B?)> {
        return Promise<(returnValue: B?, p: B?)> { seal in
            let d4 = D4()
            d4.sb = "D4.sb (1)"
            d4.pb = nil
            d4.p1 = B()
            d4.p1!.sb = "B.sb (1)"
            d4.p2 = B()
            d4.p2!.sb = "B.sb (2)"

            seal.fulfill((d4.p2, d4))
        }
    }

    func returnTest1Async(current _: Current) -> Promise<(returnValue: B?, p1: B?, p2: B?)> {
        return Promise<(returnValue: B?, p1: B?, p2: B?)> { seal in
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

            seal.fulfill((d2, d2, d1))
        }
    }

    func returnTest2Async(current _: Current) -> Promise<(returnValue: B?, p2: B?, p1: B?)> {
        return Promise<(returnValue: B?, p2: B?, p1: B?)> { seal in
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

            seal.fulfill((d1, d1, d2))
        }
    }

    func returnTest3Async(p1: B?, p2 _: B?, current _: Current) -> Promise<B?> {
        return Promise<B?> { seal in
            seal.fulfill(p1)
        }
    }

    func sequenceTestAsync(p1: SS1?, p2: SS2?, current _: Current) -> Promise<SS3> {
        return Promise<SS3> { seal in
            seal.fulfill(SS3(c1: p1, c2: p2))
        }
    }

    func dictionaryTestAsync(bin: BDict, current _: Current) -> Promise<(returnValue: BDict, bout: BDict)> {
        return Promise<(returnValue: BDict, bout: BDict)> { seal in
            var bout = [Int32: B?]()
            for i: Int32 in 0 ..< 10 {
                let b = bin[i]!!
                let d2 = D2()
                d2.sb = b.sb
                d2.pb = b.pb
                d2.sd2 = "D2"
                d2.pd2 = d2
                bout[i * 10] = d2
            }

            var r = [Int32: B]()
            for i: Int32 in 0 ..< 10 {
                let s = "D1.\(i * 20)"
                let d1 = D1()
                d1.sb = s
                d1.pb = i == 0 ? nil : r[(i - 1) * 20]
                d1.sd1 = s
                d1.pd1 = d1
                r[i * 20] = d1
            }
            return seal.fulfill((r, bout))
        }
    }

    func exchangePBaseAsync(pb: PBase?, current _: Current) -> Promise<PBase?> {
        return Promise<PBase?> { seal in
            seal.fulfill(pb)
        }
    }

    func PBSUnknownAsPreservedAsync(current: Current) -> Promise<Preserved?> {
        return Promise<Preserved?> { seal in
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
            seal.fulfill(r)
        }
    }

    func checkPBSUnknownAsync(p: Preserved?, current: Current) -> Promise<Void> {
        return Promise<Void> { _ in
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
    }

    func PBSUnknownAsPreservedWithGraphAsync(current _: Current) -> Promise<Preserved?> {
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

    func checkPBSUnknownWithGraphAsync(p: Preserved?, current: Current) -> Promise<Void> {
        return Promise<Void> { _ in
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
    }

    func PBSUnknown2AsPreservedWithGraphAsync(current _: Current) -> Promise<Preserved?> {
        return Promise<Preserved?> { seal in
            let r = PSUnknown2()
            r.pi = 5
            r.ps = "preserved"
            r.pb = r
            seal.fulfill(r)
        }
    }

    func checkPBSUnknown2WithGraphAsync(p: Preserved?, current: Current) -> Promise<Void> {
        return Promise<Void> { _ in
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
    }

    func exchangePNodeAsync(pn: PNode?, current _: Current) -> Promise<PNode?> {
        return Promise<PNode?> { seal in
            seal.fulfill(pn)
        }
    }

    func throwBaseAsBaseAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            let be = BaseException()
            be.sbe = "sbe"
            be.pb = B()
            be.pb!.sb = "sb"
            be.pb!.pb = be.pb
            throw be
        }
    }

    func throwDerivedAsBaseAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
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
    }

    func throwDerivedAsDerivedAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
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
    }

    func throwUnknownDerivedAsBaseAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
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
    }

    func throwPreservedExceptionAsync(current _: Current) -> Promise<Void> {
        return Promise<Void> { _ in
            let ue = PSUnknownException()
            ue.p = PSUnknown2()
            ue.p!.pi = 5
            ue.p!.ps = "preserved"
            ue.p!.pb = ue.p
            throw ue
        }
    }

    func useForwardAsync(current _: Current) -> Promise<Forward?> {
        return Promise<Forward?> { seal in
            let f = Forward()
            f.h = Hidden()
            f.h!.f = f
            seal.fulfill(f)
        }
    }

    func shutdownAsync(current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            current.adapter!.getCommunicator().shutdown()
            seal.fulfill(())
        }
    }
}
