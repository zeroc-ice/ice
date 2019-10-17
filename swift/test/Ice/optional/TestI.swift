//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import Ice
import TestCommon

class InitialI: Initial {
    func shutdown(current: Ice.Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }

    func pingPong(o: Ice.Value?, current _: Ice.Current) throws -> Ice.Value? {
        return o
    }

    func opOptionalException(a: Int32?,
                             b: String?,
                             o: OneOptional?,
                             current _: Ice.Current) throws {
        throw OptionalException(req: false, a: a, b: b, o: o)
    }

    func opDerivedException(a: Int32?,
                            b: String?,
                            o: OneOptional?,
                            current _: Ice.Current) throws {
        throw DerivedException(req: false, a: a, b: b, o: o, ss: b, o2: o)
    }

    func opRequiredException(a: Int32?,
                             b: String?,
                             o: OneOptional?,
                             current _: Ice.Current) throws {
        let e = RequiredException()
        e.a = a
        e.b = b
        e.o = o
        if let b = b {
            e.ss = b
        }
        e.o2 = o
        throw e
    }

    func opByte(p1: UInt8?,
                current _: Ice.Current) throws -> (returnValue: UInt8?, p3: UInt8?) {
        return (p1, p1)
    }

    func opBool(p1: Bool?, current _: Ice.Current) throws -> (returnValue: Bool?, p3: Bool?) {
        return (p1, p1)
    }

    func opShort(p1: Int16?, current _: Ice.Current) throws -> (returnValue: Int16?, p3: Int16?) {
        return (p1, p1)
    }

    func opInt(p1: Int32?, current _: Ice.Current) throws -> (returnValue: Int32?, p3: Int32?) {
        return (p1, p1)
    }

    func opLong(p1: Int64?, current _: Ice.Current) throws -> (returnValue: Int64?, p3: Int64?) {
        return (p1, p1)
    }

    func opFloat(p1: Float?, current _: Ice.Current) throws -> (returnValue: Float?, p3: Float?) {
        return (p1, p1)
    }

    func opDouble(p1: Double?, current _: Ice.Current) throws -> (returnValue: Double?, p3: Double?) {
        return (p1, p1)
    }

    func opString(p1: String?, current _: Ice.Current) throws -> (returnValue: String?, p3: String?) {
        return (p1, p1)
    }

    func opCustomString(p1: String?, current _: Current) throws -> (returnValue: String?, p3: String?) {
        return (p1, p1)
    }

    func opMyEnum(p1: MyEnum?, current _: Ice.Current) throws -> (returnValue: MyEnum?, p3: MyEnum?) {
        return (p1, p1)
    }

    func opSmallStruct(p1: SmallStruct?, current _: Ice.Current) throws -> (returnValue: SmallStruct?,
                                                                            p3: SmallStruct?) {
        return (p1, p1)
    }

    func opFixedStruct(p1: FixedStruct?, current _: Ice.Current) throws -> (returnValue: FixedStruct?,
                                                                            p3: FixedStruct?) {
        return (p1, p1)
    }

    func opVarStruct(p1: VarStruct?, current _: Ice.Current) throws -> (returnValue: VarStruct?, p3: VarStruct?) {
        return (p1, p1)
    }

    func opOneOptional(p1: OneOptional?, current _: Ice.Current) throws -> (returnValue: OneOptional?,
                                                                            p3: OneOptional?) {
        return (p1, p1)
    }

    func opOneOptionalProxy(p1: Ice.ObjectPrx?, current _: Ice.Current) throws -> (returnValue: Ice.ObjectPrx?,
                                                                                   p3: Ice.ObjectPrx?) {
        return (p1, p1)
    }

    func opByteSeq(p1: ByteSeq?, current _: Ice.Current) throws -> (returnValue: ByteSeq?, p3: ByteSeq?) {
        return (p1, p1)
    }

    func opBoolSeq(p1: BoolSeq?, current _: Ice.Current) throws -> (returnValue: BoolSeq?, p3: BoolSeq?) {
        return (p1, p1)
    }

    func opShortSeq(p1: ShortSeq?, current _: Ice.Current) throws -> (returnValue: ShortSeq?, p3: ShortSeq?) {
        return (p1, p1)
    }

    func opIntSeq(p1: IntSeq?, current _: Ice.Current) throws -> (returnValue: IntSeq?, p3: IntSeq?) {
        return (p1, p1)
    }

    func opLongSeq(p1: LongSeq?, current _: Ice.Current) throws -> (returnValue: LongSeq?, p3: LongSeq?) {
        return (p1, p1)
    }

    func opFloatSeq(p1: FloatSeq?, current _: Ice.Current) throws -> (returnValue: FloatSeq?, p3: FloatSeq?) {
        return (p1, p1)
    }

    func opDoubleSeq(p1: DoubleSeq?, current _: Ice.Current) throws -> (returnValue: DoubleSeq?, p3: DoubleSeq?) {
        return (p1, p1)
    }

    func opStringSeq(p1: StringSeq?, current _: Ice.Current) throws -> (returnValue: StringSeq?,
                                                                        p3: StringSeq?) {
        return (p1, p1)
    }

    func opSmallStructSeq(p1: SmallStructSeq?, current _: Ice.Current) throws -> (returnValue: SmallStructSeq?,
                                                                                  p3: SmallStructSeq?) {
        return (p1, p1)
    }

    func opSmallStructList(p1: SmallStructList?, current _: Ice.Current) throws -> (returnValue: SmallStructList?,
                                                                                    p3: SmallStructList?) {
        return (p1, p1)
    }

    func opFixedStructSeq(p1: FixedStructSeq?, current _: Ice.Current) throws -> (returnValue: FixedStructSeq?,
                                                                                  p3: FixedStructSeq?) {
        return (p1, p1)
    }

    func opFixedStructList(p1: FixedStructList?, current _: Ice.Current) throws -> (returnValue: FixedStructList?,
                                                                                    p3: FixedStructList?) {
        return (p1, p1)
    }

    func opVarStructSeq(p1: VarStructSeq?, current _: Ice.Current) throws -> (returnValue: VarStructSeq?,
                                                                              p3: VarStructSeq?) {
        return (p1, p1)
    }

    func opSerializable(p1: Serializable?, current _: Current) throws -> (returnValue: Serializable?,
                                                                          p3: Serializable?) {
        return (p1, p1)
    }

    func opIntIntDict(p1: [Int32: Int32]?, current _: Ice.Current) throws -> (returnValue: [Int32: Int32]?,
                                                                              p3: [Int32: Int32]?) {
        return (p1, p1)
    }

    func opStringIntDict(p1: [String: Int32]?, current _: Ice.Current) throws -> (returnValue: [String: Int32]?,
                                                                                  p3: [String: Int32]?) {
        return (p1, p1)
    }

    func opCustomIntStringDict(p1: IntStringDict?,
                               current _: Current) throws -> (returnValue: IntStringDict?, p3: IntStringDict?) {
        return (p1, p1)
    }

    func opIntOneOptionalDict(p1: [Int32: OneOptional?]?,
                              current _: Ice.Current) throws -> (returnValue: [Int32: OneOptional?]?,
                                                                 p3: [Int32: OneOptional?]?) {
        return (p1, p1)
    }

    func opClassAndUnknownOptional(p _: A?, current _: Ice.Current) throws {}

    func sendOptionalClass(req _: Bool, o _: OneOptional?, current _: Ice.Current) throws {}

    func returnOptionalClass(req _: Bool, current _: Ice.Current) throws -> OneOptional? {
        return OneOptional(a: 53)
    }

    func opG(g: G?, current _: Ice.Current) throws -> G? {
        return g
    }

    func opVoid(current _: Ice.Current) throws {}

    func supportsRequiredParams(current _: Ice.Current) throws -> Bool {
        return false
    }

    func supportsJavaSerializable(current _: Ice.Current) throws -> Bool {
        return false
    }

    func supportsCsharpSerializable(current _: Ice.Current) throws -> Bool {
        return false
    }

    func supportsCppStringView(current _: Ice.Current) throws -> Bool {
        return false
    }

    func supportsNullOptional(current _: Ice.Current) throws -> Bool {
        return false
    }

    func opMStruct1(current _: Current) throws -> SmallStruct? {
        return SmallStruct()
    }

    func opMStruct2(p1: SmallStruct?, current _: Current) throws -> (returnValue: SmallStruct?, p2: SmallStruct?) {
        return (p1, p1)
    }

    func opMSeq1(current _: Current) throws -> StringSeq? {
        return []
    }

    func opMSeq2(p1: StringSeq?, current _: Current) throws -> (returnValue: StringSeq?, p2: StringSeq?) {
        return (p1, p1)
    }

    func opMDict1(current _: Current) throws -> StringIntDict? {
        return [:]
    }

    func opMDict2(p1: StringIntDict?, current _: Current) throws -> (returnValue: StringIntDict?, p2: StringIntDict?) {
        return (p1, p1)
    }

    func opMG1(current _: Current) throws -> G? {
        return G()
    }

    func opMG2(p1: G?, current _: Current) throws -> (returnValue: G?, p2: G?) {
        return (p1, p1)
    }
}
