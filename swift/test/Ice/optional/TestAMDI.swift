//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import Ice
import PromiseKit
import TestCommon

class InitialI: Initial {
    func shutdownAsync(current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            current.adapter!.getCommunicator().shutdown()
            seal.fulfill(())
        }
    }

    func pingPongAsync(o: Value?, current _: Current) -> Promise<Value?> {
        return Promise.value(o)
    }

    func opOptionalExceptionAsync(a: Int32?, b: String?, o: OneOptional?, current _: Current) -> Promise<Void> {
        return Promise { seal in
            seal.reject(OptionalException(req: false, a: a, b: b, o: o))
        }
    }

    func opDerivedExceptionAsync(a: Int32?, b: String?, o: OneOptional?, current _: Current) -> Promise<Void> {
        return Promise { seal in
            seal.reject(DerivedException(req: false, a: a, b: b, o: o, ss: b, o2: o))
        }
    }

    func opRequiredExceptionAsync(a: Int32?, b: String?, o: OneOptional?, current _: Current) -> Promise<Void> {
        return Promise { seal in
            let e = RequiredException()
            e.a = a
            e.b = b
            e.o = o
            if let b = b {
                e.ss = b
            }
            e.o2 = o

            seal.reject(e)
        }
    }

    func opByteAsync(p1: UInt8?, current _: Current) -> Promise<(returnValue: UInt8?, p3: UInt8?)> {
        return Promise.value((p1, p1))
    }

    func opBoolAsync(p1: Bool?, current _: Current) -> Promise<(returnValue: Bool?, p3: Bool?)> {
        return Promise.value((p1, p1))
    }

    func opShortAsync(p1: Int16?, current _: Current) -> Promise<(returnValue: Int16?, p3: Int16?)> {
        return Promise.value((p1, p1))
    }

    func opIntAsync(p1: Int32?, current _: Current) -> Promise<(returnValue: Int32?, p3: Int32?)> {
        return Promise.value((p1, p1))
    }

    func opLongAsync(p1: Int64?, current _: Current) -> Promise<(returnValue: Int64?, p3: Int64?)> {
        return Promise.value((p1, p1))
    }

    func opFloatAsync(p1: Float?, current _: Current) -> Promise<(returnValue: Float?, p3: Float?)> {
        return Promise.value((p1, p1))
    }

    func opDoubleAsync(p1: Double?, current _: Current) -> Promise<(returnValue: Double?, p3: Double?)> {
        return Promise.value((p1, p1))
    }

    func opStringAsync(p1: String?, current _: Current) -> Promise<(returnValue: String?, p3: String?)> {
        return Promise.value((p1, p1))
    }

    func opMyEnumAsync(p1: MyEnum?, current _: Current) -> Promise<(returnValue: MyEnum?, p3: MyEnum?)> {
        return Promise.value((p1, p1))
    }

    func opSmallStructAsync(p1: SmallStruct?,
                            current _: Current) -> Promise<(returnValue: SmallStruct?, p3: SmallStruct?)> {
        return Promise.value((p1, p1))
    }

    func opFixedStructAsync(p1: FixedStruct?,
                            current _: Current) -> Promise<(returnValue: FixedStruct?, p3: FixedStruct?)> {
        return Promise.value((p1, p1))
    }

    func opVarStructAsync(p1: VarStruct?, current _: Current) -> Promise<(returnValue: VarStruct?, p3: VarStruct?)> {
        return Promise.value((p1, p1))
    }

    func opOneOptionalAsync(p1: OneOptional?,
                            current _: Current) -> Promise<(returnValue: OneOptional?, p3: OneOptional?)> {
        return Promise.value((p1, p1))
    }

    func opOneOptionalProxyAsync(p1: ObjectPrx?,
                                 current _: Current) -> Promise<(returnValue: ObjectPrx?, p3: ObjectPrx?)> {
        return Promise.value((p1, p1))
    }

    func opByteSeqAsync(p1: ByteSeq?, current _: Current) -> Promise<(returnValue: ByteSeq?, p3: ByteSeq?)> {
        return Promise.value((p1, p1))
    }

    func opBoolSeqAsync(p1: BoolSeq?, current _: Current) -> Promise<(returnValue: BoolSeq?, p3: BoolSeq?)> {
        return Promise.value((p1, p1))
    }

    func opShortSeqAsync(p1: ShortSeq?, current _: Current) -> Promise<(returnValue: ShortSeq?, p3: ShortSeq?)> {
        return Promise.value((p1, p1))
    }

    func opIntSeqAsync(p1: IntSeq?, current _: Current) -> Promise<(returnValue: IntSeq?, p3: IntSeq?)> {
        return Promise.value((p1, p1))
    }

    func opLongSeqAsync(p1: LongSeq?, current _: Current) -> Promise<(returnValue: LongSeq?, p3: LongSeq?)> {
        return Promise.value((p1, p1))
    }

    func opFloatSeqAsync(p1: FloatSeq?, current _: Current) -> Promise<(returnValue: FloatSeq?, p3: FloatSeq?)> {
        return Promise.value((p1, p1))
    }

    func opDoubleSeqAsync(p1: DoubleSeq?, current _: Current) -> Promise<(returnValue: DoubleSeq?, p3: DoubleSeq?)> {
        return Promise.value((p1, p1))
    }

    func opStringSeqAsync(p1: StringSeq?, current _: Current) -> Promise<(returnValue: StringSeq?, p3: StringSeq?)> {
        return Promise.value((p1, p1))
    }

    func opSmallStructSeqAsync(p1: SmallStructSeq?,
                               current _: Current) -> Promise<(returnValue: SmallStructSeq?, p3: SmallStructSeq?)> {
        return Promise.value((p1, p1))
    }

    func opSmallStructListAsync(p1: SmallStructList?,
                                current _: Current) -> Promise<(returnValue: SmallStructList?, p3: SmallStructList?)> {
        return Promise.value((p1, p1))
    }

    func opFixedStructSeqAsync(p1: FixedStructSeq?,
                               current _: Current) -> Promise<(returnValue: FixedStructSeq?, p3: FixedStructSeq?)> {
        return Promise.value((p1, p1))
    }

    func opFixedStructListAsync(p1: FixedStructList?,
                                current _: Current) -> Promise<(returnValue: FixedStructList?, p3: FixedStructList?)> {
        return Promise.value((p1, p1))
    }

    func opVarStructSeqAsync(p1: VarStructSeq?,
                             current _: Current) -> Promise<(returnValue: VarStructSeq?, p3: VarStructSeq?)> {
        return Promise.value((p1, p1))
    }

    func opSerializableAsync(p1: Serializable?,
                             current _: Current) -> Promise<(returnValue: Serializable?, p3: Serializable?)> {
        return Promise.value((p1, p1))
    }

    func opIntIntDictAsync(p1: IntIntDict?, current _: Current) -> Promise<(returnValue: IntIntDict?,
                                                                            p3: IntIntDict?)> {
        return Promise.value((p1, p1))
    }

    func opStringIntDictAsync(p1: StringIntDict?,
                              current _: Current) -> Promise<(returnValue: StringIntDict?, p3: StringIntDict?)> {
        return Promise.value((p1, p1))
    }

    func opIntOneOptionalDictAsync(p1: IntOneOptionalDict?,
                                   current _: Current) -> Promise<(returnValue: IntOneOptionalDict?,
                                                                   p3: IntOneOptionalDict?)> {
        return Promise.value((p1, p1))
    }

    func opClassAndUnknownOptionalAsync(p _: A?, current _: Current) -> Promise<Void> {
        return Promise.value(())
    }

    func sendOptionalClassAsync(req _: Bool, o _: OneOptional?, current _: Current) -> Promise<Void> {
        return Promise.value(())
    }

    func returnOptionalClassAsync(req _: Bool, current _: Current) -> Promise<OneOptional?> {
        return Promise.value(OneOptional())
    }

    func opGAsync(g: G?, current _: Current) -> Promise<G?> {
        return Promise.value(g)
    }

    func opVoidAsync(current _: Current) -> Promise<Void> {
        return Promise.value(())
    }

    func opMStruct1Async(current _: Current) -> Promise<SmallStruct?> {
        return Promise.value(SmallStruct())
    }

    func opMStruct2Async(p1: SmallStruct?, current _: Current) -> Promise<(returnValue: SmallStruct?,
                                                                           p2: SmallStruct?)> {
        return Promise.value((p1, p1))
    }

    func opMSeq1Async(current _: Current) -> Promise<StringSeq?> {
        return Promise.value([])
    }

    func opMSeq2Async(p1: StringSeq?, current _: Current) -> Promise<(returnValue: StringSeq?, p2: StringSeq?)> {
        return Promise.value((p1, p1))
    }

    func opMDict1Async(current _: Current) -> Promise<StringIntDict?> {
        return Promise.value([:])
    }

    func opMDict2Async(p1: StringIntDict?,
                       current _: Current) -> Promise<(returnValue: StringIntDict?, p2: StringIntDict?)> {
        return Promise.value((p1, p1))
    }

    func opMG1Async(current _: Current) -> Promise<G?> {
        return Promise.value(G())
    }

    func opMG2Async(p1: G?, current _: Current) -> Promise<(returnValue: G?, p2: G?)> {
        return Promise.value((p1, p1))
    }

    func supportsRequiredParamsAsync(current _: Current) -> Promise<Bool> {
        return Promise.value(false)
    }

    func supportsJavaSerializableAsync(current _: Current) -> Promise<Bool> {
        return Promise.value(false)
    }

    func supportsCsharpSerializableAsync(current _: Current) -> Promise<Bool> {
        return Promise.value(false)
    }

    func supportsCppStringViewAsync(current _: Current) -> Promise<Bool> {
        return Promise.value(false)
    }

    func supportsNullOptionalAsync(current _: Current) -> Promise<Bool> {
        return Promise.value(false)
    }
}
