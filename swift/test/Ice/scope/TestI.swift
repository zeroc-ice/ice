// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

class MyInterface1: MyInterface {
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

    func opMyEnum(MyEnum: e1, current _: Current) async throws -> MyEnum {
        return e1
    }

    func opMyOtherStruct(MyOtherStruct: s1, current _: Current) async throws -> MyOtherStruct {
        return s1
    }

    func opMyOtherClass(MyOtherClass: c1?, current _: Current) async throws -> MyOtherClass? {
        return c1
    }

    func opMyOtherStructSeq(MyOtherStructSeq: myOtherStructSeq, current _: Current) async throws -> MyOtherStructSeq {
        return myOtherStructSeq
    }

    func opMyOtherStructMap(MyOtherStructMap: myOtherStructMap, current _: Current) async throws -> MyOtherStructMap {
        return myOtherStructMap
    }

    func shutdown(current: Current) async throws {
        current.adapter.getCommunicator().shutdown()
    }
}

class MyInterface2: InnerMyInterface {
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

class MyInterface3: InnerInner2MyInterface {
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

class MyInterface4: InnerTestInner2MyInterface {
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

class DPMyInterface: DifferentMyInterface {
    func opMyStruct(s1: InnerMyStruct, current _: Current) async throws -> (returnValue: InnerMyStruct, s2: InnerMyStruct) {
        return (s1, s1)
    }

    func opMyClass(c1: DifferentMyClass?, current _: Current) async throws -> (
        returnValue: DifferentMyClass?, c2: DifferentMyClass?
    ) {
        return (c1, c1)
    }
}

class NPMyInterface: NoPrefixMyInterface {
    func opMyStruct(s1: InnerMyStruct, current _: Current) async throws -> (returnValue: InnerMyStruct, s2: InnerMyStruct) {
        return (s1, s1)
    }

    func opMyClass(c1: NoPrefixMyClass?, current _: Current) async throws -> (
        returnValue: NoPrefixMyClass?, c2: NoPrefixMyClass?
    ) {
        return (c1, c1)
    }
}
