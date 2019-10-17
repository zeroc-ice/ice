//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import Ice
import TestCommon

class I1: I {
    func opS(s1: S, current _: Current) throws -> (returnValue: S, s2: S) {
        return (s1, s1)
    }

    func opSSeq(s1: SSeq, current _: Current) throws -> (returnValue: SSeq, s2: SSeq) {
        return (s1, s1)
    }

    func opSMap(s1: SMap, current _: Current) throws -> (returnValue: SMap, s2: SMap) {
        return (s1, s1)
    }

    func opC(c1: C?, current _: Current) throws -> (returnValue: C?, c2: C?) {
        return (c1, c1)
    }

    func opCSeq(s1: CSeq, current _: Current) throws -> (returnValue: CSeq, s2: CSeq) {
        return (s1, s1)
    }

    func opCMap(c1: CMap, current _: Current) throws -> (returnValue: CMap, c2: CMap) {
        return (c1, c1)
    }

    func opE1(E1: E1, current: Current) throws -> E1 {
        return E1
    }

    func opS1(S1: S1, current: Current) throws -> S1 {
        return S1
    }

    func opC1(C1: C1?, current: Current) throws -> C1? {
        return C1
    }

    func opS1Seq(S1Seq: S1Seq, current: Current) throws -> S1Seq {
        return S1Seq
    }

    func opS1Map(S1Map: S1Map, current: Current) throws -> S1Map {
        return S1Map
    }

    func shutdown(current: Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }
}

class I2: InnerI {
    func opS(s1: InnerInner2S, current _: Current) throws -> (returnValue: InnerInner2S, s2: InnerInner2S) {
        return (s1, s1)
    }

    func opSSeq(s1: InnerInner2SSeq, current _: Current) throws -> (returnValue: InnerInner2SSeq, s2: InnerInner2SSeq) {
        return (s1, s1)
    }

    func opSMap(s1: InnerInner2SMap, current _: Current) throws -> (returnValue: InnerInner2SMap, s2: InnerInner2SMap) {
        return (s1, s1)
    }

    func opC(c1: InnerInner2C?, current _: Current) throws -> (returnValue: InnerInner2C?, c2: InnerInner2C?) {
        return (c1, c1)
    }

    func opCSeq(c1: InnerInner2CSeq, current _: Current) throws -> (returnValue: InnerInner2CSeq, c2: InnerInner2CSeq) {
        return (c1, c1)
    }

    func opCMap(c1: InnerInner2CMap, current _: Current) throws -> (returnValue: InnerInner2CMap, c2: InnerInner2CMap) {
        return (c1, c1)
    }

    func shutdown(current: Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }
}

class I3: InnerInner2I {
    func opS(s1: InnerInner2S, current _: Current) throws -> (returnValue: InnerInner2S, s2: InnerInner2S) {
        return (s1, s1)
    }

    func opSSeq(s1: InnerInner2SSeq, current _: Current) throws -> (returnValue: InnerInner2SSeq, s2: InnerInner2SSeq) {
        return (s1, s1)
    }

    func opSMap(s1: InnerInner2SMap, current _: Current) throws -> (returnValue: InnerInner2SMap, s2: InnerInner2SMap) {
        return (s1, s1)
    }

    func opC(c1: InnerInner2C?, current _: Current) throws -> (returnValue: InnerInner2C?, c2: InnerInner2C?) {
        return (c1, c1)
    }

    func opCSeq(c1: InnerInner2CSeq, current _: Current) throws -> (returnValue: InnerInner2CSeq, c2: InnerInner2CSeq) {
        return (c1, c1)
    }

    func opCMap(c1: InnerInner2CMap, current _: Current) throws -> (returnValue: InnerInner2CMap, c2: InnerInner2CMap) {
        return (c1, c1)
    }

    func shutdown(current: Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }
}

class I4: InnerTestInner2I {
    func opS(s1: S, current _: Current) throws -> (returnValue: S, s2: S) {
        return (s1, s1)
    }

    func opSSeq(s1: SSeq, current _: Current) throws -> (returnValue: SSeq, s2: SSeq) {
        return (s1, s1)
    }

    func opSMap(s1: SMap, current _: Current) throws -> (returnValue: SMap, s2: SMap) {
        return (s1, s1)
    }

    func opC(c1: C?, current _: Current) throws -> (returnValue: C?, c2: C?) {
        return (c1, c1)
    }

    func opCSeq(c1: CSeq, current _: Current) throws -> (returnValue: CSeq, c2: CSeq) {
        return (c1, c1)
    }

    func opCMap(c1: CMap, current _: Current) throws -> (returnValue: CMap, c2: CMap) {
        return (c1, c1)
    }

    func shutdown(current: Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }
}
