//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import Ice
import PromiseKit
import TestCommon

class BI: MBOperations {
    func opBAsync(current _: Ice.Current) -> Promise<Void> {
        return Promise.value(())
    }

    func opIntfAsync(current _: Ice.Current) -> Promise<Void> {
        return Promise.value(())
    }
}

class MyDerivedClassI: ObjectI<MyDerivedClassTraits>, MyDerivedClass {
    var _helper: TestHelper
    var _opByteSOnewayCallCount: Int32 = 0
    var _lock = os_unfair_lock()

    init(_ helper: TestHelper) {
        _helper = helper
    }

    //
    // Override the Object "pseudo" operations to verify the operation mode.
    //
    override func ice_isA(id: String, current: Ice.Current) throws -> Bool {
        try _helper.test(current.mode == .Nonmutating)
        return try super.ice_isA(id: id, current: current)
    }

    override func ice_ping(current: Ice.Current) throws {
        try _helper.test(current.mode == .Nonmutating)
    }

    override func ice_ids(current: Ice.Current) throws -> [String] {
        try _helper.test(current.mode == .Nonmutating)
        return try super.ice_ids(current: current)
    }

    override func ice_id(current: Ice.Current) throws -> String {
        try _helper.test(current.mode == .Nonmutating)
        return try super.ice_id(current: current)
    }

    func opDerivedAsync(current _: Current) -> Promise<Void> {
        return Promise.value(())
    }

    func opMyClass1Async(opMyClass1: MyClass1?, current _: Current) -> Promise<MyClass1?> {
        return Promise.value(opMyClass1)
    }

    func opMyStruct1Async(opMyStruct1: MyStruct1, current _: Current) -> Promise<MyStruct1> {
        return Promise.value(opMyStruct1)
    }

    func shutdownAsync(current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            current.adapter!.getCommunicator().shutdown()
            seal.fulfill(())
        }
    }

    func supportsCompressAsync(current _: Current) -> Promise<Bool> {
        return Promise.value(true)
    }

    func opVoidAsync(current _: Current) -> Promise<Void> {
        return Promise.value(())
    }

    func opByteAsync(p1: UInt8, p2: UInt8, current _: Current) -> Promise<(returnValue: UInt8, p3: UInt8)> {
        return Promise.value((p1, p1 ^ p2))
    }

    func opBoolAsync(p1: Bool, p2: Bool, current _: Current) -> Promise<(returnValue: Bool, p3: Bool)> {
        return Promise.value((p2, p1))
    }

    func opShortIntLongAsync(p1: Int16,
                             p2: Int32,
                             p3: Int64,
                             current _: Current) -> Promise<(returnValue: Int64, p4: Int16, p5: Int32, p6: Int64)> {
        return Promise.value((p3, p1, p2, p3))
    }

    func opFloatDoubleAsync(p1: Float,
                            p2: Double,
                            current _: Current) -> Promise<(returnValue: Double, p3: Float, p4: Double)> {
        return Promise.value((p2, p1, p2))
    }

    func opStringAsync(p1: String, p2: String, current _: Current) -> Promise<(returnValue: String, p3: String)> {
        return Promise.value(("\(p1) \(p2)", "\(p2) \(p1)"))
    }

    func opMyEnumAsync(p1: MyEnum, current _: Current) -> Promise<(returnValue: MyEnum, p2: MyEnum)> {
        return Promise.value((MyEnum.enum3, p1))
    }

    func opMyClassAsync(p1: MyClassPrx?,
                        current: Current) -> Promise<(returnValue: MyClassPrx?, p2: MyClassPrx?, p3: MyClassPrx?)> {
        guard let adapter = current.adapter else {
            fatalError()
        }
        return Promise { seal in
            do {
                seal.fulfill(
                    (try uncheckedCast(prx: adapter.createProxy(current.id), type: MyClassPrx.self),
                     p1,
                     try uncheckedCast(prx: adapter.createProxy(Ice.stringToIdentity("noSuchIdentity")),
                                       type: MyClassPrx.self)))
            } catch {
                seal.reject(error)
            }
        }
    }

    func opStructAsync(p1: Structure,
                       p2: Structure,
                       current _: Current) -> Promise<(returnValue: Structure, p3: Structure)> {
        var p3 = p1
        p3.s.s = "a new string"
        return Promise.value((p2, p3))
    }

    func opByteSAsync(p1: ByteS, p2: ByteS, current _: Current) -> Promise<(returnValue: ByteS, p3: ByteS)> {
        return Promise.value((p1 + p2, ByteSeq(p1.reversed())))
    }

    func opBoolSAsync(p1: BoolS, p2: BoolS, current _: Current) -> Promise<(returnValue: BoolS, p3: BoolS)> {
        return Promise.value((p1.reversed(), p1 + p2))
    }

    func opShortIntLongSAsync(p1: ShortS,
                              p2: IntS,
                              p3: LongS,
                              current _: Current) -> Promise<(returnValue: LongS, p4: ShortS, p5: IntS, p6: LongS)> {
        return Promise.value((p3, p1, p2.reversed(), p3 + p3))
    }

    func opFloatDoubleSAsync(p1: FloatS,
                             p2: DoubleS,
                             current _: Current) -> Promise<(returnValue: DoubleS, p3: FloatS, p4: DoubleS)> {
        return Promise.value((p2 + p1.map { Double($0) }, p1, p2.reversed()))
    }

    func opStringSAsync(p1: StringS, p2: StringS, current _: Current) -> Promise<(returnValue: StringS, p3: StringS)> {
        return Promise.value((p1.reversed(), p1 + p2))
    }

    func opByteSSAsync(p1: ByteSS, p2: ByteSS, current _: Current) -> Promise<(returnValue: ByteSS, p3: ByteSS)> {
        return Promise.value((p1 + p2, p1.reversed()))
    }

    func opBoolSSAsync(p1: BoolSS, p2: BoolSS, current _: Current) -> Promise<(returnValue: BoolSS, p3: BoolSS)> {
        return Promise.value((p1.reversed(), p1 + p2))
    }

    func opShortIntLongSSAsync(p1: ShortSS,
                               p2: IntSS,
                               p3: LongSS,
                               current _: Current) -> Promise<(returnValue: LongSS,
                                                               p4: ShortSS,
                                                               p5: IntSS,
                                                               p6: LongSS)> {
        return Promise.value((p3, p1, p2.reversed(), p3 + p3))
    }

    func opFloatDoubleSSAsync(p1: FloatSS,
                              p2: DoubleSS,
                              current _: Current) -> Promise<(returnValue: DoubleSS, p3: FloatSS, p4: DoubleSS)> {
        return Promise.value((p2 + p2, p1, p2.reversed()))
    }

    func opStringSSAsync(p1: StringSS,
                         p2: StringSS,
                         current _: Current) -> Promise<(returnValue: StringSS, p3: StringSS)> {
        return Promise.value((p2.reversed(), p1 + p2))
    }

    func opStringSSSAsync(p1: StringSSS,
                          p2: StringSSS,
                          current _: Current) -> Promise<(returnValue: StringSSS, p3: StringSSS)> {
        return Promise.value((p2.reversed(), p1 + p2))
    }

    func opByteBoolDAsync(p1: ByteBoolD,
                          p2: ByteBoolD,
                          current _: Current) -> Promise<(returnValue: ByteBoolD, p3: ByteBoolD)> {
        return Promise.value((p1.merging(p2) { _, new in new }, p1))
    }

    func opShortIntDAsync(p1: ShortIntD,
                          p2: ShortIntD,
                          current _: Current) -> Promise<(returnValue: ShortIntD, p3: ShortIntD)> {
        return Promise.value((p1.merging(p2) { _, new in new }, p1))
    }

    func opLongFloatDAsync(p1: LongFloatD,
                           p2: LongFloatD,
                           current _: Current) -> Promise<(returnValue: LongFloatD, p3: LongFloatD)> {
        return Promise.value((p1.merging(p2) { _, new in new }, p1))
    }

    func opStringStringDAsync(p1: StringStringD,
                              p2: StringStringD,
                              current _: Current) -> Promise<(returnValue: StringStringD, p3: StringStringD)> {
        return Promise.value((p1.merging(p2) { _, new in new }, p1))
    }

    func opStringMyEnumDAsync(p1: StringMyEnumD,
                              p2: StringMyEnumD,
                              current _: Current) -> Promise<(returnValue: StringMyEnumD, p3: StringMyEnumD)> {
        return Promise.value((p1.merging(p2) { _, new in new }, p1))
    }

    func opMyEnumStringDAsync(p1: MyEnumStringD,
                              p2: MyEnumStringD,
                              current _: Current) -> Promise<(returnValue: MyEnumStringD, p3: MyEnumStringD)> {
        return Promise.value((p1.merging(p2) { _, new in new }, p1))
    }

    func opMyStructMyEnumDAsync(p1: MyStructMyEnumD,
                                p2: MyStructMyEnumD,
                                current _: Current) -> Promise<(returnValue: MyStructMyEnumD, p3: MyStructMyEnumD)> {
        return Promise.value((p1.merging(p2) { _, new in new }, p1))
    }

    func opByteBoolDSAsync(p1: ByteBoolDS,
                           p2: ByteBoolDS,
                           current _: Current) -> Promise<(returnValue: ByteBoolDS, p3: ByteBoolDS)> {
        return Promise.value((p1.reversed(), p2 + p1))
    }

    func opShortIntDSAsync(p1: ShortIntDS,
                           p2: ShortIntDS,
                           current _: Current) -> Promise<(returnValue: ShortIntDS, p3: ShortIntDS)> {
        return Promise.value((p1.reversed(), p2 + p1))
    }

    func opLongFloatDSAsync(p1: LongFloatDS,
                            p2: LongFloatDS,
                            current _: Current) -> Promise<(returnValue: LongFloatDS, p3: LongFloatDS)> {
        return Promise.value((p1.reversed(), p2 + p1))
    }

    func opStringStringDSAsync(p1: StringStringDS,
                               p2: StringStringDS,
                               current _: Current) -> Promise<(returnValue: StringStringDS, p3: StringStringDS)> {
        return Promise.value((p1.reversed(), p2 + p1))
    }

    func opStringMyEnumDSAsync(p1: StringMyEnumDS,
                               p2: StringMyEnumDS,
                               current _: Current) -> Promise<(returnValue: StringMyEnumDS, p3: StringMyEnumDS)> {
        return Promise.value((p1.reversed(), p2 + p1))
    }

    func opMyEnumStringDSAsync(p1: MyEnumStringDS,
                               p2: MyEnumStringDS,
                               current _: Current) -> Promise<(returnValue: MyEnumStringDS, p3: MyEnumStringDS)> {
        return Promise.value((p1.reversed(), p2 + p1))
    }

    func opMyStructMyEnumDSAsync(p1: MyStructMyEnumDS,
                                 p2: MyStructMyEnumDS,
                                 current _: Current) -> Promise<(returnValue: MyStructMyEnumDS, p3: MyStructMyEnumDS)> {
        return Promise.value((p1.reversed(), p2 + p1))
    }

    func opByteByteSDAsync(p1: ByteByteSD,
                           p2: ByteByteSD,
                           current _: Current) -> Promise<(returnValue: ByteByteSD, p3: ByteByteSD)> {
        return Promise.value((p1.merging(p2) { _, new in new }, p2))
    }

    func opBoolBoolSDAsync(p1: BoolBoolSD,
                           p2: BoolBoolSD,
                           current _: Current) -> Promise<(returnValue: BoolBoolSD, p3: BoolBoolSD)> {
        return Promise.value((p1.merging(p2) { _, new in new }, p2))
    }

    func opShortShortSDAsync(p1: ShortShortSD,
                             p2: ShortShortSD,
                             current _: Current) -> Promise<(returnValue: ShortShortSD, p3: ShortShortSD)> {
        return Promise.value((p1.merging(p2) { _, new in new }, p2))
    }

    func opIntIntSDAsync(p1: IntIntSD,
                         p2: IntIntSD,
                         current _: Current) -> Promise<(returnValue: IntIntSD, p3: IntIntSD)> {
        return Promise.value((p1.merging(p2) { _, new in new }, p2))
    }

    func opLongLongSDAsync(p1: LongLongSD,
                           p2: LongLongSD,
                           current _: Current) -> Promise<(returnValue: LongLongSD, p3: LongLongSD)> {
        return Promise.value((p1.merging(p2) { _, new in new }, p2))
    }

    func opStringFloatSDAsync(p1: StringFloatSD,
                              p2: StringFloatSD,
                              current _: Current) -> Promise<(returnValue: StringFloatSD, p3: StringFloatSD)> {
        return Promise.value((p1.merging(p2) { _, new in new }, p2))
    }

    func opStringDoubleSDAsync(p1: StringDoubleSD,
                               p2: StringDoubleSD,
                               current _: Current) -> Promise<(returnValue: StringDoubleSD, p3: StringDoubleSD)> {
        return Promise.value((p1.merging(p2) { _, new in new }, p2))
    }

    func opStringStringSDAsync(p1: StringStringSD,
                               p2: StringStringSD,
                               current _: Current) -> Promise<(returnValue: StringStringSD, p3: StringStringSD)> {
        return Promise.value((p1.merging(p2) { _, new in new }, p2))
    }

    func opMyEnumMyEnumSDAsync(p1: MyEnumMyEnumSD,
                               p2: MyEnumMyEnumSD,
                               current _: Current) -> Promise<(returnValue: MyEnumMyEnumSD, p3: MyEnumMyEnumSD)> {
        return Promise.value((p1.merging(p2) { _, new in new }, p2))
    }

    func opIntSAsync(s: IntS, current _: Current) -> Promise<IntS> {
        return Promise.value(s.map { -$0 })
    }

    func opByteSOnewayAsync(s _: ByteS, current _: Current) -> Promise<Void> {
        withLock(&_lock) {
            _opByteSOnewayCallCount += 1
        }
        return Promise.value(())
    }

    func opByteSOnewayCallCountAsync(current _: Current) -> Promise<Int32> {
        return withLock(&_lock) {
            let count = _opByteSOnewayCallCount
            _opByteSOnewayCallCount = 0
            return Promise<Int32>.value(count)
        }
    }

    func opContextAsync(current: Current) -> Promise<Context> {
        return Promise.value(current.ctx)
    }

    func opDoubleMarshalingAsync(p1: Double, p2: DoubleS, current _: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            do {
                let d = Double(1_278_312_346.0 / 13.0)
                try _helper.test(p1 == d)
                for p in p2 {
                    try _helper.test(p == d)
                }
                seal.fulfill(())
            } catch {
                seal.reject(error)
            }
        }
    }

    func opIdempotentAsync(current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            do {
                try _helper.test(current.mode == .Idempotent)
                seal.fulfill(())
            } catch {
                seal.reject(error)
            }
        }
    }

    func opNonmutatingAsync(current: Current) -> Promise<Void> {
        return Promise<Void> { seal in
            do {
                try _helper.test(current.mode == .Nonmutating)
                seal.fulfill(())
            } catch {
                seal.reject(error)
            }
        }
    }

    func opByte1Async(opByte1: UInt8, current _: Current) -> Promise<UInt8> {
        return Promise.value(opByte1)
    }

    func opShort1Async(opShort1: Int16, current _: Current) -> Promise<Int16> {
        return Promise.value(opShort1)
    }

    func opInt1Async(opInt1: Int32, current _: Current) -> Promise<Int32> {
        return Promise.value(opInt1)
    }

    func opLong1Async(opLong1: Int64, current _: Current) -> Promise<Int64> {
        return Promise.value(opLong1)
    }

    func opFloat1Async(opFloat1: Float, current _: Current) -> Promise<Float> {
        return Promise.value(opFloat1)
    }

    func opDouble1Async(opDouble1: Double, current _: Current) -> Promise<Double> {
        return Promise.value(opDouble1)
    }

    func opString1Async(opString1: String, current _: Current) -> Promise<String> {
        return Promise.value(opString1)
    }

    func opStringS1Async(opStringS1: StringS, current _: Current) -> Promise<StringS> {
        return Promise.value(opStringS1)
    }

    func opByteBoolD1Async(opByteBoolD1: ByteBoolD, current _: Current) -> Promise<ByteBoolD> {
        return Promise.value(opByteBoolD1)
    }

    func opStringS2Async(stringS: StringS, current _: Current) -> Promise<StringS> {
        return Promise.value(stringS)
    }

    func opByteBoolD2Async(byteBoolD: ByteBoolD, current _: Current) -> Promise<ByteBoolD> {
        return Promise.value(byteBoolD)
    }

    func opStringLiteralsAsync(current _: Current) -> Promise<StringS> {
        return Promise.value([s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10,
                              sw0, sw1, sw2, sw3, sw4, sw5, sw6, sw7, sw8, sw9, sw10,
                              ss0, ss1, ss2, ss3, ss4, ss5,
                              su0, su1, su2])
    }

    func opWStringLiteralsAsync(current: Current) -> Promise<StringS> {
        return opStringLiteralsAsync(current: current)
    }
}
