// Copyright (c) ZeroC, Inc.

import Foundation
import Ice
import TestCommon

final class InitialI: Initial {
    func shutdown(current: Ice.Current) async throws {
        current.adapter.getCommunicator().shutdown()
    }

    func pingPong(o: Ice.Value?, current _: Ice.Current) async throws -> Ice.Value? {
        return o
    }

    func opOptionalException(
        a: Int32?,
        b: String?,
        current _: Ice.Current
    ) throws {
        throw OptionalException(req: false, a: a, b: b)
    }

    func opDerivedException(
        a: Int32?,
        b: String?,
        current _: Ice.Current
    ) throws {
        throw DerivedException(req: false, a: a, b: b, d1: "d1", ss: b, d2: "d2")
    }

    func opRequiredException(
        a: Int32?,
        b: String?,
        current _: Ice.Current
    ) throws {
        let e = RequiredException()
        e.a = a
        e.b = b
        if let b = b {
            e.ss = b
        }
        throw e
    }

    func opByte(
        p1: UInt8?,
        current _: Ice.Current
    ) throws -> (returnValue: UInt8?, p3: UInt8?) {
        return (p1, p1)
    }

    func opBool(p1: Bool?, current _: Ice.Current) async throws -> (returnValue: Bool?, p3: Bool?) {
        return (p1, p1)
    }

    func opShort(p1: Int16?, current _: Ice.Current) async throws -> (returnValue: Int16?, p3: Int16?) {
        return (p1, p1)
    }

    func opInt(p1: Int32?, current _: Ice.Current) async throws -> (returnValue: Int32?, p3: Int32?) {
        return (p1, p1)
    }

    func opLong(p1: Int64?, current _: Ice.Current) async throws -> (returnValue: Int64?, p3: Int64?) {
        return (p1, p1)
    }

    func opFloat(p1: Float?, current _: Ice.Current) async throws -> (returnValue: Float?, p3: Float?) {
        return (p1, p1)
    }

    func opDouble(p1: Double?, current _: Ice.Current) async throws -> (
        returnValue: Double?, p3: Double?
    ) {
        return (p1, p1)
    }

    func opString(p1: String?, current _: Ice.Current) async throws -> (
        returnValue: String?, p3: String?
    ) {
        return (p1, p1)
    }

    func opCustomString(p1: String?, current _: Current) async throws -> (
        returnValue: String?, p3: String?
    ) {
        return (p1, p1)
    }

    func opMyEnum(p1: MyEnum?, current _: Ice.Current) async throws -> (
        returnValue: MyEnum?, p3: MyEnum?
    ) {
        return (p1, p1)
    }

    func opSmallStruct(p1: SmallStruct?, current _: Ice.Current) async throws -> (
        returnValue: SmallStruct?,
        p3: SmallStruct?
    ) {
        return (p1, p1)
    }

    func opFixedStruct(p1: FixedStruct?, current _: Ice.Current) async throws -> (
        returnValue: FixedStruct?,
        p3: FixedStruct?
    ) {
        return (p1, p1)
    }

    func opVarStruct(p1: VarStruct?, current _: Ice.Current) async throws -> (
        returnValue: VarStruct?, p3: VarStruct?
    ) {
        return (p1, p1)
    }

    func opOneOptional(p1: OneOptional?, current _: Ice.Current) async throws -> (
        returnValue: OneOptional?,
        p3: OneOptional?
    ) {
        return (p1, p1)
    }

    func opMyInterfaceProxy(p1: MyInterfacePrx?, current _: Ice.Current) async throws -> (
        returnValue: MyInterfacePrx?,
        p3: MyInterfacePrx?
    ) {
        return (p1, p1)
    }

    func opByteSeq(p1: ByteSeq?, current _: Ice.Current) async throws -> (
        returnValue: ByteSeq?, p3: ByteSeq?
    ) {
        return (p1, p1)
    }

    func opBoolSeq(p1: BoolSeq?, current _: Ice.Current) async throws -> (
        returnValue: BoolSeq?, p3: BoolSeq?
    ) {
        return (p1, p1)
    }

    func opShortSeq(p1: ShortSeq?, current _: Ice.Current) async throws -> (
        returnValue: ShortSeq?, p3: ShortSeq?
    ) {
        return (p1, p1)
    }

    func opIntSeq(p1: IntSeq?, current _: Ice.Current) async throws -> (
        returnValue: IntSeq?, p3: IntSeq?
    ) {
        return (p1, p1)
    }

    func opLongSeq(p1: LongSeq?, current _: Ice.Current) async throws -> (
        returnValue: LongSeq?, p3: LongSeq?
    ) {
        return (p1, p1)
    }

    func opFloatSeq(p1: FloatSeq?, current _: Ice.Current) async throws -> (
        returnValue: FloatSeq?, p3: FloatSeq?
    ) {
        return (p1, p1)
    }

    func opDoubleSeq(p1: DoubleSeq?, current _: Ice.Current) async throws -> (
        returnValue: DoubleSeq?, p3: DoubleSeq?
    ) {
        return (p1, p1)
    }

    func opStringSeq(p1: StringSeq?, current _: Ice.Current) async throws -> (
        returnValue: StringSeq?,
        p3: StringSeq?
    ) {
        return (p1, p1)
    }

    func opSmallStructSeq(p1: SmallStructSeq?, current _: Ice.Current) async throws -> (
        returnValue: SmallStructSeq?,
        p3: SmallStructSeq?
    ) {
        return (p1, p1)
    }

    func opSmallStructList(p1: SmallStructList?, current _: Ice.Current) async throws -> (
        returnValue: SmallStructList?,
        p3: SmallStructList?
    ) {
        return (p1, p1)
    }

    func opFixedStructSeq(p1: FixedStructSeq?, current _: Ice.Current) async throws -> (
        returnValue: FixedStructSeq?,
        p3: FixedStructSeq?
    ) {
        return (p1, p1)
    }

    func opFixedStructList(p1: FixedStructList?, current _: Ice.Current) async throws -> (
        returnValue: FixedStructList?,
        p3: FixedStructList?
    ) {
        return (p1, p1)
    }

    func opVarStructSeq(p1: VarStructSeq?, current _: Ice.Current) async throws -> (
        returnValue: VarStructSeq?,
        p3: VarStructSeq?
    ) {
        return (p1, p1)
    }

    func opSerializable(p1: Serializable?, current _: Current) async throws -> (
        returnValue: Serializable?,
        p3: Serializable?
    ) {
        return (p1, p1)
    }

    func opIntIntDict(p1: [Int32: Int32]?, current _: Ice.Current) async throws -> (
        returnValue: [Int32: Int32]?,
        p3: [Int32: Int32]?
    ) {
        return (p1, p1)
    }

    func opStringIntDict(p1: [String: Int32]?, current _: Ice.Current) async throws -> (
        returnValue: [String: Int32]?,
        p3: [String: Int32]?
    ) {
        return (p1, p1)
    }

    func opCustomIntStringDict(
        p1: IntStringDict?,
        current _: Current
    ) throws -> (returnValue: IntStringDict?, p3: IntStringDict?) {
        return (p1, p1)
    }

    func opClassAndUnknownOptional(p _: A?, current _: Ice.Current) async throws {}

    func opG(g: G?, current _: Ice.Current) async throws -> G? {
        return g
    }

    func opVoid(current _: Ice.Current) async throws {}

    func supportsJavaSerializable(current _: Ice.Current) async throws -> Bool {
        return false
    }

    func opMStruct1(current _: Current) async throws -> SmallStruct? {
        return SmallStruct()
    }

    func opMStruct2(p1: SmallStruct?, current _: Current) async throws -> (
        returnValue: SmallStruct?, p2: SmallStruct?
    ) {
        return (p1, p1)
    }

    func opMSeq1(current _: Current) async throws -> StringSeq? {
        return []
    }

    func opMSeq2(p1: StringSeq?, current _: Current) async throws -> (
        returnValue: StringSeq?, p2: StringSeq?
    ) {
        return (p1, p1)
    }

    func opMDict1(current _: Current) async throws -> StringIntDict? {
        return [:]
    }

    func opMDict2(p1: StringIntDict?, current _: Current) async throws -> (
        returnValue: StringIntDict?, p2: StringIntDict?
    ) {
        return (p1, p1)
    }
}
