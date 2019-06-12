//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import Ice
import TestCommon

class BI: MBOperations {
    func opB(current _: Ice.Current) throws {}

    func opIntf(current _: Ice.Current) throws {}
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

    func shutdown(current: Ice.Current) throws {
        current.adapter!.getCommunicator().shutdown()
    }

    func supportsCompress(current _: Ice.Current) throws -> Bool {
        return true
    }

    func opVoid(current: Ice.Current) throws {
        try _helper.test(current.mode == .Normal)
    }

    func opBool(p1: Bool, p2: Bool, current _: Ice.Current) throws -> (returnValue: Bool, p3: Bool) {
        return (p2, p1)
    }

    func opBoolS(p1: [Bool], p2: [Bool], current _: Ice.Current) throws -> (returnValue: [Bool], p3: [Bool]) {
        return (p1.reversed(), p1 + p2)
    }

    func opBoolSS(p1: [[Bool]], p2: [[Bool]], current _: Ice.Current) throws -> (returnValue: [[Bool]], p3: [[Bool]]) {
        return (p1.reversed(), p1 + p2)
    }

    func opByte(p1: UInt8, p2: UInt8, current _: Ice.Current) throws -> (returnValue: UInt8, p3: UInt8) {
        return (p1, p1 ^ p2)
    }

    func opByteBoolD(p1: [UInt8: Bool],
                     p2: [UInt8: Bool],
                     current _: Ice.Current) throws -> (returnValue: [UInt8: Bool], p3: [UInt8: Bool]) {
        return (p1.merging(p2) { _, new in new }, p1)
    }

    func opByteS(p1: ByteSeq,
                 p2: ByteSeq, current _: Ice.Current) throws -> (returnValue: ByteSeq, p3: ByteSeq) {
        return (p1 + p2, ByteSeq(p1.reversed()))
    }

    func opByteSS(p1: [ByteSeq],
                  p2: [ByteSeq], current _: Ice.Current) throws -> (returnValue: [ByteSeq], p3: [ByteSeq]) {
        return (p1 + p2, p1.reversed())
    }

    func opFloatDouble(p1: Float,
                       p2: Double,
                       current _: Ice.Current) throws -> (returnValue: Double, p3: Float, p4: Double) {
        return (p2, p1, p2)
    }

    func opFloatDoubleS(p1: [Float],
                        p2: [Double],
                        current _: Ice.Current) throws -> (returnValue: [Double], p3: [Float], p4: [Double]) {
        return (p2 + p1.map { Double($0) }, p1, p2.reversed())
    }

    func opFloatDoubleSS(p1: [[Float]],
                         p2: [[Double]],
                         current _: Ice.Current) throws -> (returnValue: [[Double]], p3: [[Float]], p4: [[Double]]) {
        return (p2 + p2, p1, p2.reversed())
    }

    func opLongFloatD(p1: [Int64: Float],
                      p2: [Int64: Float],
                      current _: Ice.Current) throws -> (returnValue: [Int64: Float], p3: [Int64: Float]) {
        return (p1.merging(p2) { _, new in new }, p1)
    }

    func opMyClass(p1: MyClassPrx?,
                   current: Ice.Current) throws -> (returnValue: MyClassPrx?, p2: MyClassPrx?, p3: MyClassPrx?) {
        guard let adapter = current.adapter else {
            fatalError()
        }
        return (try uncheckedCast(prx: adapter.createProxy(current.id), type: MyClassPrx.self),
                p1,
                try uncheckedCast(prx: adapter.createProxy(Ice.stringToIdentity("noSuchIdentity")),
                                  type: MyClassPrx.self))
    }

    func opMyEnum(p1: MyEnum, current _: Ice.Current) throws -> (returnValue: MyEnum, p2: MyEnum) {
        return (MyEnum.enum3, p1)
    }

    func opShortIntD(p1: [Int16: Int32],
                     p2: [Int16: Int32],
                     current _: Ice.Current) throws -> (returnValue: [Int16: Int32], p3: [Int16: Int32]) {
        return (p1.merging(p2) { _, new in new }, p1)
    }

    func opShortIntLong(p1: Int16,
                        p2: Int32,
                        p3: Int64,
                        current _: Ice.Current) throws -> (returnValue: Int64, p4: Int16, p5: Int32, p6: Int64) {
        return (p3, p1, p2, p3)
    }

    func opShortIntLongS(p1: [Int16],
                         p2: [Int32],
                         p3: [Int64],
                         current _: Ice.Current) throws -> (returnValue: [Int64],
                                                            p4: [Int16],
                                                            p5: [Int32],
                                                            p6: [Int64]) {
        return (p3, p1, p2.reversed(), p3 + p3)
    }

    func opShortIntLongSS(p1: [[Int16]],
                          p2: [[Int32]],
                          p3: [[Int64]],
                          current _: Ice.Current) throws -> (returnValue: [[Int64]],
                                                             p4: [[Int16]],
                                                             p5: [[Int32]],
                                                             p6: [[Int64]]) {
        return (p3, p1, p2.reversed(), p3 + p3)
    }

    func opString(p1: String, p2: String, current _: Ice.Current) throws -> (returnValue: String, p3: String) {
        return ("\(p1) \(p2)", "\(p2) \(p1)")
    }

    func opStringMyEnumD(p1: [String: MyEnum],
                         p2: [String: MyEnum],
                         current _: Ice.Current) throws -> (returnValue: [String: MyEnum], p3: [String: MyEnum]) {
        return (p1.merging(p2) { _, new in new }, p1)
    }

    func opMyEnumStringD(p1: [MyEnum: String],
                         p2: [MyEnum: String],
                         current _: Ice.Current) throws -> (returnValue: [MyEnum: String], p3: [MyEnum: String]) {
        return (p1.merging(p2) { _, new in new }, p1)
    }

    func opMyStructMyEnumD(p1: [MyStruct: MyEnum],
                           p2: [MyStruct: MyEnum],
                           current _: Ice.Current) throws -> (returnValue: [MyStruct: MyEnum], p3: [MyStruct: MyEnum]) {
        return (p1.merging(p2) { _, new in new }, p1)
    }

    func opByteBoolDS(p1: [[UInt8: Bool]],
                      p2: [[UInt8: Bool]],
                      current _: Ice.Current) throws -> (returnValue: [[UInt8: Bool]], p3: [[UInt8: Bool]]) {
        return (p1.reversed(), p2 + p1)
    }

    func opShortIntDS(p1: [[Int16: Int32]],
                      p2: [[Int16: Int32]],
                      current _: Ice.Current) throws -> (returnValue: [[Int16: Int32]], p3: [[Int16: Int32]]) {
        return (p1.reversed(), p2 + p1)
    }

    func opLongFloatDS(p1: [[Int64: Float]],
                       p2: [[Int64: Float]],
                       current _: Ice.Current) throws -> (returnValue: [[Int64: Float]], p3: [[Int64: Float]]) {
        return (p1.reversed(), p2 + p1)
    }

    func opStringStringDS(p1: [[String: String]],
                          p2: [[String: String]],
                          current _: Ice.Current) throws -> (returnValue: [[String: String]], p3: [[String: String]]) {
        return (p1.reversed(), p2 + p1)
    }

    func opStringMyEnumDS(p1: [[String: MyEnum]],
                          p2: [[String: MyEnum]],
                          current _: Ice.Current) throws -> (returnValue: [[String: MyEnum]], p3: [[String: MyEnum]]) {
        return (p1.reversed(), p2 + p1)
    }

    func opMyEnumStringDS(p1: [[MyEnum: String]],
                          p2: [[MyEnum: String]],
                          current _: Ice.Current) throws -> (returnValue: [[MyEnum: String]], p3: [[MyEnum: String]]) {
        return (p1.reversed(), p2 + p1)
    }

    func opMyStructMyEnumDS(p1: [[MyStruct: MyEnum]],
                            p2: [[MyStruct: MyEnum]],
                            current _: Ice.Current) throws -> (returnValue: [[MyStruct: MyEnum]],
                                                               p3: [[MyStruct: MyEnum]]) {
        return (p1.reversed(), p2 + p1)
    }

    func opByteByteSD(p1: [UInt8: ByteSeq],
                      p2: [UInt8: ByteSeq],
                      current _: Ice.Current) throws -> (returnValue: [UInt8: ByteSeq], p3: [UInt8: ByteSeq]) {
        return (p1.merging(p2) { _, new in new }, p2)
    }

    func opBoolBoolSD(p1: [Bool: [Bool]],
                      p2: [Bool: [Bool]],
                      current _: Ice.Current) throws -> (returnValue: [Bool: [Bool]], p3: [Bool: [Bool]]) {
        return (p1.merging(p2) { _, new in new }, p2)
    }

    func opShortShortSD(p1: [Int16: [Int16]],
                        p2: [Int16: [Int16]],
                        current _: Ice.Current) throws -> (returnValue: [Int16: [Int16]], p3: [Int16: [Int16]]) {
        return (p1.merging(p2) { _, new in new }, p2)
    }

    func opIntIntSD(p1: [Int32: [Int32]],
                    p2: [Int32: [Int32]],
                    current _: Ice.Current) throws -> (returnValue: [Int32: [Int32]], p3: [Int32: [Int32]]) {
        return (p1.merging(p2) { _, new in new }, p2)
    }

    func opLongLongSD(p1: [Int64: [Int64]],
                      p2: [Int64: [Int64]],
                      current _: Ice.Current) throws -> (returnValue: [Int64: [Int64]], p3: [Int64: [Int64]]) {
        return (p1.merging(p2) { _, new in new }, p2)
    }

    func opStringFloatSD(p1: [String: [Float]],
                         p2: [String: [Float]],
                         current _: Ice.Current) throws -> (returnValue: [String: [Float]], p3: [String: [Float]]) {
        return (p1.merging(p2) { _, new in new }, p2)
    }

    func opStringDoubleSD(p1: [String: [Double]],
                          p2: [String: [Double]],
                          current _: Ice.Current) throws -> (returnValue: [String: [Double]], p3: [String: [Double]]) {
        return (p1.merging(p2) { _, new in new }, p2)
    }

    func opStringStringSD(p1: [String: [String]],
                          p2: [String: [String]],
                          current _: Ice.Current) throws -> (returnValue: [String: [String]], p3: [String: [String]]) {
        return (p1.merging(p2) { _, new in new }, p2)
    }

    func opMyEnumMyEnumSD(p1: [MyEnum: [MyEnum]],
                          p2: [MyEnum: [MyEnum]],
                          current _: Ice.Current) throws -> (returnValue: [MyEnum: [MyEnum]], p3: [MyEnum: [MyEnum]]) {
        return (p1.merging(p2) { _, new in new }, p2)
    }

    func opIntS(s: [Int32], current _: Ice.Current) throws -> [Int32] {
        return s.map { -$0 }
    }

    func opByteSOneway(s _: ByteSeq, current _: Ice.Current) throws {
        withLock(&_lock) {
            _opByteSOnewayCallCount += 1
        }
    }

    func opByteSOnewayCallCount(current _: Ice.Current) throws -> Int32 {
        return withLock(&_lock) {
            let count = _opByteSOnewayCallCount
            _opByteSOnewayCallCount = 0
            return count
        }
    }

    func opContext(current: Ice.Current) throws -> Ice.Context {
        return current.ctx
    }

    func opDoubleMarshaling(p1: Double, p2: [Double], current _: Ice.Current) throws {
        let d = Double(1_278_312_346.0 / 13.0)
        try _helper.test(p1 == d)
        for p in p2 {
            try _helper.test(p == d)
        }
    }

    func opStringS(p1: [String], p2: [String], current _: Ice.Current) throws -> (returnValue: [String], p3: [String]) {
        return (p1.reversed(), p1 + p2)
    }

    func opStringSS(p1: [[String]],
                    p2: [[String]],
                    current _: Ice.Current) throws -> (returnValue: [[String]], p3: [[String]]) {
        return (p2.reversed(), p1 + p2)
    }

    func opStringSSS(p1: [[[String]]],
                     p2: [[[String]]],
                     current _: Ice.Current) throws -> (returnValue: [[[String]]], p3: [[[String]]]) {
        return (p2.reversed(), p1 + p2)
    }

    func opStringStringD(p1: [String: String],
                         p2: [String: String],
                         current _: Ice.Current) throws -> (returnValue: [String: String], p3: [String: String]) {
        return (p1.merging(p2) { _, new in new }, p1)
    }

    func opStruct(p1: Structure,
                  p2: Structure,
                  current _: Ice.Current) throws -> (returnValue: Structure, p3: Structure) {
        var p3 = p1
        p3.s.s = "a new string"
        return (p2, p3)
    }

    func opIdempotent(current: Ice.Current) throws {
        try _helper.test(current.mode == .Idempotent)
    }

    func opNonmutating(current: Ice.Current) throws {
        try _helper.test(current.mode == .Nonmutating)
    }

    func opDerived(current _: Ice.Current) throws {}

    func opByte1(opByte1: UInt8, current _: Ice.Current) throws -> UInt8 {
        return opByte1
    }

    func opShort1(opShort1: Int16, current _: Ice.Current) throws -> Int16 {
        return opShort1
    }

    func opInt1(opInt1: Int32, current _: Ice.Current) throws -> Int32 {
        return opInt1
    }

    func opLong1(opLong1: Int64, current _: Ice.Current) throws -> Int64 {
        return opLong1
    }

    func opFloat1(opFloat1: Float, current _: Ice.Current) throws -> Float {
        return opFloat1
    }

    func opDouble1(opDouble1: Double, current _: Ice.Current) throws -> Double {
        return opDouble1
    }

    func opString1(opString1: String, current _: Ice.Current) throws -> String {
        return opString1
    }

    func opStringS1(opStringS1: [String], current _: Ice.Current) throws -> [String] {
        return opStringS1
    }

    func opByteBoolD1(opByteBoolD1: [UInt8: Bool], current _: Ice.Current) throws -> [UInt8: Bool] {
        return opByteBoolD1
    }

    func opStringS2(stringS: [String], current _: Ice.Current) throws -> [String] {
        return stringS
    }

    func opByteBoolD2(byteBoolD: [UInt8: Bool], current _: Ice.Current) throws -> [UInt8: Bool] {
        return byteBoolD
    }

    func opMyClass1(opMyClass1: MyClass1?, current _: Ice.Current) throws -> MyClass1? {
        return opMyClass1
    }

    func opMyStruct1(opMyStruct1: MyStruct1, current _: Ice.Current) throws -> MyStruct1 {
        return opMyStruct1
    }

    func opStringLiterals(current _: Ice.Current) throws -> [String] {
        return [s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10,
                sw0, sw1, sw2, sw3, sw4, sw5, sw6, sw7, sw8, sw9, sw10,
                ss0, ss1, ss2, ss3, ss4, ss5,
                su0, su1, su2]
    }

    func opWStringLiterals(current: Ice.Current) throws -> [String] {
        return try opStringLiterals(current: current)
    }
}
