// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

class I1: I {
    func opS(s1: S, current _: Current) async throws -> (returnValue: S, s2: S) {
        return (s1, s1)
    }

    func opSSeq(s1: SSeq, current _: Current) async throws -> (returnValue: SSeq, s2: SSeq) {
        return (s1, s1)
    }

    func opSMap(s1: SMap, current _: Current) async throws -> (returnValue: SMap, s2: SMap) {
        return (s1, s1)
    }

    func opC(c1: C?, current _: Current) async throws -> (returnValue: C?, c2: C?) {
        return (c1, c1)
    }

    func opCSeq(s1: CSeq, current _: Current) async throws -> (returnValue: CSeq, s2: CSeq) {
        return (s1, s1)
    }

    func opCMap(c1: CMap, current _: Current) async throws -> (returnValue: CMap, c2: CMap) {
        return (c1, c1)
    }

    func opE1(E1: E1, current _: Current) async throws -> E1 {
        return E1
    }

    func opS1(S1: S1, current _: Current) async throws -> S1 {
        return S1
    }

    func opC1(C1: C1?, current _: Current) async throws -> C1? {
        return C1
    }

    func opS1Seq(S1Seq: S1Seq, current _: Current) async throws -> S1Seq {
        return S1Seq
    }

    func opS1Map(S1Map: S1Map, current _: Current) async throws -> S1Map {
        return S1Map
    }

    func shutdown(current: Current) async throws {
        current.adapter.getCommunicator().shutdown()
    }
}

class I2: InnerI {
    func opS(s1: InnerInner2S, current _: Current) async throws -> (
        returnValue: InnerInner2S, s2: InnerInner2S
    ) {
        return (s1, s1)
    }

    func opSSeq(s1: InnerInner2SSeq, current _: Current) async throws -> (
        returnValue: InnerInner2SSeq, s2: InnerInner2SSeq
    ) {
        return (s1, s1)
    }

    func opSMap(s1: InnerInner2SMap, current _: Current) async throws -> (
        returnValue: InnerInner2SMap, s2: InnerInner2SMap
    ) {
        return (s1, s1)
    }

    func opC(c1: InnerInner2C?, current _: Current) async throws -> (
        returnValue: InnerInner2C?, c2: InnerInner2C?
    ) {
        return (c1, c1)
    }

    func opCSeq(c1: InnerInner2CSeq, current _: Current) async throws -> (
        returnValue: InnerInner2CSeq, c2: InnerInner2CSeq
    ) {
        return (c1, c1)
    }

    func opCMap(c1: InnerInner2CMap, current _: Current) async throws -> (
        returnValue: InnerInner2CMap, c2: InnerInner2CMap
    ) {
        return (c1, c1)
    }

    func shutdown(current: Current) async throws {
        current.adapter.getCommunicator().shutdown()
    }
}

class I3: InnerInner2I {
    func opS(s1: InnerInner2S, current _: Current) async throws -> (
        returnValue: InnerInner2S, s2: InnerInner2S
    ) {
        return (s1, s1)
    }

    func opSSeq(s1: InnerInner2SSeq, current _: Current) async throws -> (
        returnValue: InnerInner2SSeq, s2: InnerInner2SSeq
    ) {
        return (s1, s1)
    }

    func opSMap(s1: InnerInner2SMap, current _: Current) async throws -> (
        returnValue: InnerInner2SMap, s2: InnerInner2SMap
    ) {
        return (s1, s1)
    }

    func opC(c1: InnerInner2C?, current _: Current) async throws -> (
        returnValue: InnerInner2C?, c2: InnerInner2C?
    ) {
        return (c1, c1)
    }

    func opCSeq(c1: InnerInner2CSeq, current _: Current) async throws -> (
        returnValue: InnerInner2CSeq, c2: InnerInner2CSeq
    ) {
        return (c1, c1)
    }

    func opCMap(c1: InnerInner2CMap, current _: Current) async throws -> (
        returnValue: InnerInner2CMap, c2: InnerInner2CMap
    ) {
        return (c1, c1)
    }

    func shutdown(current: Current) async throws {
        current.adapter.getCommunicator().shutdown()
    }
}

class I4: InnerTestInner2I {
    func opS(s1: S, current _: Current) async throws -> (returnValue: S, s2: S) {
        return (s1, s1)
    }

    func opSSeq(s1: SSeq, current _: Current) async throws -> (returnValue: SSeq, s2: SSeq) {
        return (s1, s1)
    }

    func opSMap(s1: SMap, current _: Current) async throws -> (returnValue: SMap, s2: SMap) {
        return (s1, s1)
    }

    func opC(c1: C?, current _: Current) async throws -> (returnValue: C?, c2: C?) {
        return (c1, c1)
    }

    func opCSeq(c1: CSeq, current _: Current) async throws -> (returnValue: CSeq, c2: CSeq) {
        return (c1, c1)
    }

    func opCMap(c1: CMap, current _: Current) async throws -> (returnValue: CMap, c2: CMap) {
        return (c1, c1)
    }

    func shutdown(current: Current) async throws {
        current.adapter.getCommunicator().shutdown()
    }
}

class DPI: DifferentI {
    func opS(s1: InnerS, current _: Current) async throws -> (returnValue: InnerS, s2: InnerS) {
        return (s1, s1)
    }

    func opC(c1: DifferentClass1?, current _: Current) async throws -> (
        returnValue: DifferentClass1?, c2: DifferentClass1?
    ) {
        return (c1, c1)
    }
}

class NPI: NoPrefixI {
    func opS(s1: InnerS, current _: Current) async throws -> (returnValue: InnerS, s2: InnerS) {
        return (s1, s1)
    }

    func opC(c1: NoPrefixClass1?, current _: Current) async throws -> (
        returnValue: NoPrefixClass1?, c2: NoPrefixClass1?
    ) {
        return (c1, c1)
    }
}
