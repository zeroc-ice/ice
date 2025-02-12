// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

class I1: I {
    func opMyStruct(s1: MyStruct, current _: Current) async throws -> (returnValue: MyStruct, s2: MyStruct) {
        return (s1, s1)
    }

    func opMyStructSeq(s1: MyStructSeq, current _: Current) async throws -> (returnValue: MyStructSeq, s2: MyStructSeq) {
        return (s1, s1)
    }

    func opMyStructMap(s1: MyStructMap, current _: Current) async throws -> (returnValue: MyStructMap, s2: MyStructMap) {
        return (s1, s1)
    }

    func opMyClass(c1: MyClass?, current _: Current) async throws -> (returnValue: MyClass?, c2: MyClass?) {
        return (c1, c1)
    }

    func opMyClassSeq(s1: MyClassSeq, current _: Current) async throws -> (returnValue: MyClassSeq, s2: MyClassSeq) {
        return (s1, s1)
    }

    func opMyClassMap(c1: MyClassMap, current _: Current) async throws -> (returnValue: MyClassMap, c2: MyClassMap) {
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
    func opMyStruct(s1: InnerInner2MyStruct, current _: Current) async throws -> (
        returnValue: InnerInner2MyStruct, s2: InnerInner2MyStruct
    ) {
        return (s1, s1)
    }

    func opMyStructSeq(s1: InnerInner2MyStructSeq, current _: Current) async throws -> (
        returnValue: InnerInner2MyStructSeq, s2: InnerInner2MyStructSeq
    ) {
        return (s1, s1)
    }

    func opMyStructMap(s1: InnerInner2MyStructMap, current _: Current) async throws -> (
        returnValue: InnerInner2MyStructMap, s2: InnerInner2MyStructMap
    ) {
        return (s1, s1)
    }

    func opMyClass(c1: InnerInner2MyClass?, current _: Current) async throws -> (
        returnValue: InnerInner2MyClass?, c2: InnerInner2MyClass?
    ) {
        return (c1, c1)
    }

    func opMyClassSeq(c1: InnerInner2MyClassSeq, current _: Current) async throws -> (
        returnValue: InnerInner2MyClassSeq, c2: InnerInner2MyClassSeq
    ) {
        return (c1, c1)
    }

    func opMyClassMap(c1: InnerInner2MyClassMap, current _: Current) async throws -> (
        returnValue: InnerInner2MyClassMap, c2: InnerInner2MyClassMap
    ) {
        return (c1, c1)
    }

    func shutdown(current: Current) async throws {
        current.adapter.getCommunicator().shutdown()
    }
}

class I3: InnerInner2I {
    func opMyStruct(s1: InnerInner2MyStruct, current _: Current) async throws -> (
        returnValue: InnerInner2MyStruct, s2: InnerInner2MyStruct
    ) {
        return (s1, s1)
    }

    func opMyStructSeq(s1: InnerInner2MyStructSeq, current _: Current) async throws -> (
        returnValue: InnerInner2MyStructSeq, s2: InnerInner2MyStructSeq
    ) {
        return (s1, s1)
    }

    func opMyStructMap(s1: InnerInner2MyStructMap, current _: Current) async throws -> (
        returnValue: InnerInner2MyStructMap, s2: InnerInner2MyStructMap
    ) {
        return (s1, s1)
    }

    func opMyClass(c1: InnerInner2MyClass?, current _: Current) async throws -> (
        returnValue: InnerInner2MyClass?, c2: InnerInner2MyClass?
    ) {
        return (c1, c1)
    }

    func opMyClassSeq(c1: InnerInner2MyClassSeq, current _: Current) async throws -> (
        returnValue: InnerInner2MyClassSeq, c2: InnerInner2MyClassSeq
    ) {
        return (c1, c1)
    }

    func opMyClassMap(c1: InnerInner2MyClassMap, current _: Current) async throws -> (
        returnValue: InnerInner2MyClassMap, c2: InnerInner2MyClassMap
    ) {
        return (c1, c1)
    }

    func shutdown(current: Current) async throws {
        current.adapter.getCommunicator().shutdown()
    }
}

class I4: InnerTestInner2I {
    func opMyStruct(s1: MyStruct, current _: Current) async throws -> (returnValue: MyStruct, s2: MyStruct) {
        return (s1, s1)
    }

    func opMyStructSeq(s1: MyStructSeq, current _: Current) async throws -> (returnValue: MyStructSeq, s2: MyStructSeq) {
        return (s1, s1)
    }

    func opMyStructMap(s1: MyStructMap, current _: Current) async throws -> (returnValue: MyStructMap, s2: MyStructMap) {
        return (s1, s1)
    }

    func opMyClass(c1: MyClass?, current _: Current) async throws -> (returnValue: MyClass?, c2: MyClass?) {
        return (c1, c1)
    }

    func opMyClassSeq(c1: MyClassSeq, current _: Current) async throws -> (returnValue: MyClassSeq, c2: MyClassSeq) {
        return (c1, c1)
    }

    func opMyClassMap(c1: MyClassMap, current _: Current) async throws -> (returnValue: MyClassMap, c2: MyClassMap) {
        return (c1, c1)
    }

    func shutdown(current: Current) async throws {
        current.adapter.getCommunicator().shutdown()
    }
}

class DPI: DifferentI {
    func opMyStruct(s1: InnerMyStruct, current _: Current) async throws -> (returnValue: InnerMyStruct, s2: InnerMyStruct) {
        return (s1, s1)
    }

    func opMyClass(c1: DifferentMyClass?, current _: Current) async throws -> (
        returnValue: DifferentMyClass?, c2: DifferentMyClass?
    ) {
        return (c1, c1)
    }
}

class NPI: NoPrefixI {
    func opMyStruct(s1: InnerMyStruct, current _: Current) async throws -> (returnValue: InnerMyStruct, s2: InnerMyStruct) {
        return (s1, s1)
    }

    func opMyClass(c1: NoPrefixMyClass?, current _: Current) async throws -> (
        returnValue: NoPrefixMyClass?, c2: NoPrefixMyClass?
    ) {
        return (c1, c1)
    }
}
