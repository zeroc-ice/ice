// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

final class MyInterface1: MyInterface {
    func opMyStruct(s1: MyStruct, current _: Current) -> (returnValue: MyStruct, s2: MyStruct) {
        return (s1, s1)
    }

    func opMyStructSeq(s1: MyStructSeq, current _: Current) -> (returnValue: MyStructSeq, s2: MyStructSeq) {
        return (s1, s1)
    }

    func opMyStructMap(s1: MyStructMap, current _: Current) -> (returnValue: MyStructMap, s2: MyStructMap) {
        return (s1, s1)
    }

    func opMyClass(c1: MyClass?, current _: Current) -> (returnValue: MyClass?, c2: MyClass?) {
        return (c1, c1)
    }

    func opMyClassSeq(s1: MyClassSeq, current _: Current) -> (
        returnValue: MyClassSeq, s2: MyClassSeq
    ) {
        return (s1, s1)
    }

    func opMyClassMap(c1: MyClassMap, current _: Current) -> (
        returnValue: MyClassMap, c2: MyClassMap
    ) {
        return (c1, c1)
    }

    func opMyEnum(e1: MyEnum, current _: Current) -> MyEnum {
        return e1
    }

    func opMyOtherStruct(s1: MyOtherStruct, current _: Current) -> MyOtherStruct {
        return s1
    }

    func opMyOtherClass(c1: MyOtherClass?, current _: Current) -> MyOtherClass? {
        return c1
    }

    func opMyOtherStructSeq(myOtherStructSeq: MyOtherStructSeq, current _: Current) -> MyOtherStructSeq {
        return myOtherStructSeq
    }

    func opMyOtherStructMap(myOtherStructMap: MyOtherStructMap, current _: Current) -> MyOtherStructMap {
        return myOtherStructMap
    }

    func shutdown(current: Current) {
        current.adapter.getCommunicator().shutdown()
    }
}

final class MyInterface2: InnerMyInterface {
    func opMyStruct(s1: InnerInner2MyStruct, current _: Current) -> (
        returnValue: InnerInner2MyStruct, s2: InnerInner2MyStruct
    ) {
        return (s1, s1)
    }

    func opMyStructSeq(s1: InnerInner2MyStructSeq, current _: Current) -> (
        returnValue: InnerInner2MyStructSeq, s2: InnerInner2MyStructSeq
    ) {
        return (s1, s1)
    }

    func opMyStructMap(s1: InnerInner2MyStructMap, current _: Current) -> (
        returnValue: InnerInner2MyStructMap, s2: InnerInner2MyStructMap
    ) {
        return (s1, s1)
    }

    func opMyClass(c1: InnerInner2MyClass?, current _: Current) -> (
        returnValue: InnerInner2MyClass?, c2: InnerInner2MyClass?
    ) {
        return (c1, c1)
    }

    func opMyClassSeq(c1: InnerInner2MyClassSeq, current _: Current) -> (
        returnValue: InnerInner2MyClassSeq, c2: InnerInner2MyClassSeq
    ) {
        return (c1, c1)
    }

    func opMyClassMap(c1: InnerInner2MyClassMap, current _: Current) -> (
        returnValue: InnerInner2MyClassMap, c2: InnerInner2MyClassMap
    ) {
        return (c1, c1)
    }

    func shutdown(current: Current) {
        current.adapter.getCommunicator().shutdown()
    }
}

final class MyInterface3: InnerInner2MyInterface {
    func opMyStruct(s1: InnerInner2MyStruct, current _: Current) -> (
        returnValue: InnerInner2MyStruct, s2: InnerInner2MyStruct
    ) {
        return (s1, s1)
    }

    func opMyStructSeq(s1: InnerInner2MyStructSeq, current _: Current) -> (
        returnValue: InnerInner2MyStructSeq, s2: InnerInner2MyStructSeq
    ) {
        return (s1, s1)
    }

    func opMyStructMap(s1: InnerInner2MyStructMap, current _: Current) -> (
        returnValue: InnerInner2MyStructMap, s2: InnerInner2MyStructMap
    ) {
        return (s1, s1)
    }

    func opMyClass(c1: InnerInner2MyClass?, current _: Current) -> (
        returnValue: InnerInner2MyClass?, c2: InnerInner2MyClass?
    ) {
        return (c1, c1)
    }

    func opMyClassSeq(c1: InnerInner2MyClassSeq, current _: Current) -> (
        returnValue: InnerInner2MyClassSeq, c2: InnerInner2MyClassSeq
    ) {
        return (c1, c1)
    }

    func opMyClassMap(c1: InnerInner2MyClassMap, current _: Current) -> (
        returnValue: InnerInner2MyClassMap, c2: InnerInner2MyClassMap
    ) {
        return (c1, c1)
    }

    func shutdown(current: Current) {
        current.adapter.getCommunicator().shutdown()
    }
}

final class MyInterface4: InnerTestInner2MyInterface {
    func opMyStruct(s1: MyStruct, current _: Current) -> (returnValue: MyStruct, s2: MyStruct) {
        return (s1, s1)
    }

    func opMyStructSeq(s1: MyStructSeq, current _: Current) -> (returnValue: MyStructSeq, s2: MyStructSeq) {
        return (s1, s1)
    }

    func opMyStructMap(s1: MyStructMap, current _: Current) -> (returnValue: MyStructMap, s2: MyStructMap) {
        return (s1, s1)
    }

    func opMyClass(c1: MyClass?, current _: Current) -> (
        returnValue: MyClass?, c2: MyClass?
    ) {
        return (c1, c1)
    }

    func opMyClassSeq(c1: MyClassSeq, current _: Current) -> (
        returnValue: MyClassSeq, c2: MyClassSeq
    ) {
        return (c1, c1)
    }

    func opMyClassMap(c1: MyClassMap, current _: Current) -> (
        returnValue: MyClassMap, c2: MyClassMap
    ) {
        return (c1, c1)
    }

    func shutdown(current: Current) {
        current.adapter.getCommunicator().shutdown()
    }
}

final class DPMyInterface: DifferentMyInterface {
    func opMyStruct(s1: InnerMyStruct, current _: Current) -> (
        returnValue: InnerMyStruct, s2: InnerMyStruct
    ) {
        return (s1, s1)
    }

    func opMyClass(c1: DifferentMyClass?, current _: Current) -> (
        returnValue: DifferentMyClass?, c2: DifferentMyClass?
    ) {
        return (c1, c1)
    }
}

final class NPMyInterface: NoPrefixMyInterface {
    func opMyStruct(s1: InnerMyStruct, current _: Current) -> (
        returnValue: InnerMyStruct, s2: InnerMyStruct
    ) {
        return (s1, s1)
    }

    func opMyClass(c1: NoPrefixMyClass?, current _: Current) -> (
        returnValue: NoPrefixMyClass?, c2: NoPrefixMyClass?
    ) {
        return (c1, c1)
    }
}
