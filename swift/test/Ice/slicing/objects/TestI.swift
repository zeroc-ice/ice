// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

// TODO: Make this class an actor once https://github.com/swiftlang/swift/issues/76710 has shipped.
final class TestI: TestIntf {
    private let _helper: TestHelper
    private var _values = [Value]()

    init(_ helper: TestHelper) {
        _helper = helper
    }

    deinit {
        for value in _values {
            breakCycles(value)
        }
    }

    private nonisolated func breakCycles(_ value: Value) {
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
        if let d4 = value as? D4 {
            d4.p1 = nil
            d4.p2 = nil
        }
        if let b = value as? B {
            b.pb?.pb = nil
            b.pb = nil
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

    func SBaseAsObject(current _: Current) -> Value? {
        return SBase(sb: "SBase.sb")
    }

    func SBaseAsSBase(current _: Current) -> SBase? {
        return SBase(sb: "SBase.sb")
    }

    func SBSKnownDerivedAsSBase(current _: Current) -> SBase? {
        return SBSKnownDerived(sb: "SBSKnownDerived.sb", sbskd: "SBSKnownDerived.sbskd")
    }

    func SBSKnownDerivedAsSBSKnownDerived(current _: Current) -> SBSKnownDerived? {
        return SBSKnownDerived(sb: "SBSKnownDerived.sb", sbskd: "SBSKnownDerived.sbskd")
    }

    func SBSUnknownDerivedAsSBase(current _: Current) -> SBase? {
        return SBSUnknownDerived(sb: "SBSUnknownDerived.sb", sbsud: "SBSUnknownDerived.sbsud")
    }

    func SBSUnknownDerivedAsSBaseCompact(current _: Current) -> SBase? {
        return SBSUnknownDerived(sb: "SBSUnknownDerived.sb", sbsud: "SBSUnknownDerived.sbsud")
    }

    func SUnknownAsObject(current _: Current) -> Value? {
        let su = SUnknown()
        su.su = "SUnknown.su"
        su.cycle = su
        _values.append(su)
        return su
    }

    func checkSUnknown(o: Value?, current: Current) throws {
        let su = o as? SUnknown
        if current.encoding == Encoding_1_0 {
            try _helper.test(su == nil)
        } else {
            try _helper.test(su!.su == "SUnknown.su")
            su!.cycle = nil
        }
    }

    func oneElementCycle(current _: Current) -> B? {
        let b = B()
        b.sb = "B1.sb"
        b.pb = b
        _values.append(b)
        return b
    }

    func twoElementCycle(current _: Current) -> B? {
        let b1 = B()
        b1.sb = "B1.sb"
        let b2 = B()
        b2.sb = "B2.sb"
        b2.pb = b1
        b1.pb = b2
        _values.append(b1)
        return b1
    }

    func D1AsB(current _: Current) -> B? {
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
        _values.append(d1)
        return d1
    }

    func D1AsD1(current _: Current) -> D1? {
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
        _values.append(d1)
        return d1
    }

    func D2AsB(current _: Current) -> B? {
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
        _values.append(d1)
        return d2
    }

    func paramTest1(current _: Current) -> (p1: B?, p2: B?) {
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
        _values.append(d1)
        return (d1, d2)
    }

    func paramTest2(current: Current) -> (p2: B?, p1: B?) {
        let ret = paramTest1(current: current)
        return (ret.p2, ret.p1)
    }

    func paramTest3(current _: Current) -> (returnValue: B?, p1: B?, p2: B?) {
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

        _values.append(d1)
        _values.append(d2)
        _values.append(d3)
        _values.append(d4)

        return (d3, d2, d4)
    }

    func paramTest4(current _: Current) -> (returnValue: B?, p: B?) {
        let d4 = D4()
        d4.sb = "D4.sb (1)"
        d4.pb = nil
        d4.p1 = B()
        d4.p1!.sb = "B.sb (1)"
        d4.p2 = B()
        d4.p2!.sb = "B.sb (2)"
        _values.append(d4)
        return (d4.p2, d4)
    }

    func returnTest1(current: Current) -> (returnValue: B?, p1: B?, p2: B?) {
        let ret = paramTest1(current: current)
        return (ret.p1, ret.p1, ret.p2)
    }

    func returnTest2(current: Current) -> (returnValue: B?, p2: B?, p1: B?) {
        let ret = paramTest1(current: current)
        return (ret.p1, ret.p1, ret.p2)
    }

    func returnTest3(p1: B?, p2: B?, current _: Current) -> B? {
        _values.append(p1!)
        _values.append(p2!)
        return p1
    }

    func sequenceTest(p1: SS1?, p2: SS2?, current _: Current) -> SS3 {
        let ss = SS3()
        ss.c1 = p1
        ss.c2 = p2
        _values.append(p1!)
        _values.append(p2!)
        return ss
    }

    func dictionaryTest(bin: BDict, current _: Current) -> (
        returnValue: BDict, bout: BDict
    ) {
        var bout = [Int32: B?]()
        for i: Int32 in 0..<10 {
            let b = bin[i]!!
            let d2 = D2()
            d2.sb = b.sb
            d2.pb = b.pb
            d2.sd2 = "D2"
            d2.pd2 = d2
            _values.append(d2)
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
            _values.append(d1)
            r[i * 20] = d1
        }
        return (r, bout)
    }

    func exchangePBase(pb: PBase?, current _: Current) -> PBase? {
        _values.append(pb!)
        return pb
    }

    func PBSUnknownAsPreserved(current: Current) -> Preserved? {
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

    // TODO: Doesn't seem to be called
    func PBSUnknownAsPreservedWithGraph(current: Current) -> Preserved? {
        // This code requires a regular, non-colloc dispatch
        //TODO: there are not more dispatch queues
        // if let dq = try? current.adapter.getDispatchQueue() {
        //     dispatchPrecondition(condition: .onQueue(dq))
        // }

        // TODO: update this comment and make sure this is all true
        // .barrier to ensure we execute this code after Ice has called "done" on the promise
        // Otherwise the cycle breaking can occur before the result is marshaled by the
        // closure given to done.
        // return try await withCheckedThrowingContinuation { continuation in
        //     do {
        //         try current.adapter.getDispatchQueue().async(flags: .barrier) {
        //             let r = PSUnknown()
        //             r.pi = 5
        //             r.ps = "preserved"
        //             r.psu = "unknown"
        //             r.graph = PNode()
        //             r.graph!.next = PNode()
        //             r.graph!.next!.next = PNode()
        //             r.graph!.next!.next!.next = r.graph
        //             continuation.resume(returning: r)  // Ice marshals r now
        //             r.graph!.next!.next!.next = nil  // break the cycle
        //         }
        //     } catch {
        //         continuation.resume(throwing: error)
        //     }

        // }

        fatalError("not implemented")
    }

    func checkPBSUnknownWithGraph(p: Preserved?, current: Current) throws {
        if current.encoding == Encoding_1_0 {
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
            if pu.graph!.next!.next!.next == nil {
                try _helper.test(false)

            } else {
                try _helper.test(pu.graph!.next!.next!.next === pu.graph)
            }
            pu.graph!.next!.next!.next = nil  // break the cycle
        }
    }

    func PBSUnknown2AsPreservedWithGraph(current: Current) -> Preserved? {
        fatalError("not implemented")
        // TODO: verify this is correct
        // return try await withCheckedThrowingContinuation { continuation in
        //     do {
        //         try current.adapter.getDispatchQueue().async(flags: .barrier) {
        //             let r = PSUnknown2()
        //             r.pi = 5
        //             r.ps = "preserved"
        //             r.pb = r
        //             continuation.resume(returning: r)  // Ice marshals r immediately
        //             r.pb = nil  // break the cycle
        //         }
        //     } catch {
        //         continuation.resume(throwing: error)
        //     }
        // }
    }

    func checkPBSUnknown2WithGraph(p: Preserved?, current: Current) throws {
        if current.encoding == Encoding_1_0 {
            try _helper.test(!(p is PSUnknown2))
            try _helper.test(p!.pi == 5)
            try _helper.test(p!.ps == "preserved")
        } else {
            let pu = p as! PSUnknown2
            try _helper.test(pu.pi == 5)
            try _helper.test(pu.ps == "preserved")
            try _helper.test(pu.pb === pu)
            pu.pb = nil  // break the cycle
        }
    }

    func exchangePNode(pn: PNode?, current _: Current) -> PNode? {
        return pn
    }

    func throwBaseAsBase(current _: Current) throws {
        let be = BaseException()
        be.sbe = "sbe"
        be.pb = B()
        be.pb!.sb = "sb"
        be.pb!.pb = be.pb
        _values.append(be.pb!)
        throw be
    }

    func throwDerivedAsBase(current _: Current) throws {
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
        _values.append(de.pb!)
        _values.append(de.pd1!)
        throw de
    }

    func throwDerivedAsDerived(current _: Current) throws {
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
        _values.append(de.pb!)
        _values.append(de.pd1!)
        throw de
    }

    func throwUnknownDerivedAsBase(current _: Current) throws {
        let d2 = D2()
        d2.sb = "sb d2"
        d2.pb = d2
        d2.sd2 = "sd2 d2"
        d2.pd2 = d2
        _values.append(d2)

        let ude = UnknownDerivedException()
        ude.sbe = "sbe"
        ude.pb = d2
        ude.sude = "sude"
        ude.pd2 = d2
        throw ude
    }

    func useForward(current _: Current) -> Forward? {
        let f = Forward()
        f.h = Hidden()
        f.h!.f = f
        _values.append(f)
        return f
    }

    func shutdown(current: Current) {
        current.adapter.getCommunicator().shutdown()
    }
}
