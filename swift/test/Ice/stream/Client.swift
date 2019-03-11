//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import Ice
import TestCommon

public class Client: TestHelperI {
    public override func run(args _: [String]) throws {
        var writer = getWriter()
        writer.write("testing primitive types... ")
        var communicator = try Ice.initialize()
        defer {
            communicator.destroy()
        }

        var inS: Ice.InputStream
        var outS: Ice.OutputStream

        do {
            let data = [UInt8]()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            outS.startEncapsulation()
            outS.write(true)
            outS.endEncapsulation()
            let data = outS.finished()

            inS = Ice.InputStream(communicator: communicator, bytes: data)
            try inS.startEncapsulation()
            var value: Bool = try inS.read()
            try test(value)

            inS = Ice.InputStream(communicator: communicator, bytes: data)
            try inS.startEncapsulation()
            value = try inS.read()
            try test(value)
        }

        do {
            inS = Ice.InputStream(communicator: communicator, bytes: [UInt8]())
            do {
                _ = try inS.read() as Bool
                try test(false)
            } catch {}
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(true)
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let value: Bool = try inS.read()
            try test(value)
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(UInt8(1))
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let value: UInt8 = try inS.read()
            try test(value == 1)
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(Int16(2))
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let value: Int16 = try inS.read()
            try test(value == 2)
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(Int32(3))
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let value: Int32 = try inS.read()
            try test(value == 3)
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(Int64(4))
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let value: Int64 = try inS.read()
            try test(value == 4)
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(Float(5.0))
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let value: Float = try inS.read()
            try test(value == 5.0)
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(Double(6.0))
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let value: Double = try inS.read()
            try test(value == 6.0)
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            outS.write("hello world")
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let value: String = try inS.read()
            try test(value == "hello world")
        }

        writer.writeLine("ok")

        writer.write("testing constructed types... ")
        do {
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(MyEnum.enum3)
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let e: MyEnum = try inS.read()
            try test(e == MyEnum.enum3)
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            var s = SmallStruct()
            s.bo = true
            s.by = 1
            s.sh = 2
            s.i = 3
            s.l = 4
            s.f = 5.0
            s.d = 6.0
            s.str = "7"
            s.e = MyEnum.enum2
            s.p = uncheckedCast(prx: try communicator.stringToProxy(str: "test:default")!,
                                type: MyInterfacePrx.self)
            outS.write(s)
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let s2: SmallStruct = try inS.read()
            try test(s2.bo == true)
            try test(s2.by == 1)
            try test(s2.sh == 2)
            try test(s2.i == 3)
            try test(s2.l == 4)
            try test(s2.f == 5.0)
            try test(s2.d == 6.0)
            try test(s2.str == "7")
            try test(s2.e == MyEnum.enum2)
            try test(Ice.proxyEquals(lhs: s2.p, rhs: s.p))
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            let o = OptionalClass()
            o.bo = true
            o.by = 5
            o.sh = 4
            o.i = 3
            outS.write(o)
            outS.writePendingValues()
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            var o2: OptionalClass?
            try inS.read(value: OptionalClass.self) {
                o2 = $0
            }
            try inS.readPendingValues()
            try test(o2!.bo == o.bo)
            try test(o2!.by == o.by)
            if communicator.getProperties().getProperty(key: "Ice.Default.EncodingVersion") == "1.0" {
                try test(o2!.sh != nil)
                try test(o2!.i != nil)
            } else {
                try test(o2!.sh == o.sh)
                try test(o2!.i == o.i)
            }
        }

        do {
            outS = Ice.OutputStream(communicator: communicator, encoding: Ice.Encoding_1_0)
            let o = OptionalClass()
            o.bo = true
            o.by = 5
            o.sh = 4
            o.i = 3
            outS.write(o)
            outS.writePendingValues()
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, encoding: Ice.Encoding_1_0, bytes: data)
            var o2: OptionalClass?
            try inS.read(value: OptionalClass.self) {
                o2 = $0
            }
            try inS.readPendingValues()
            try test(o2!.bo == o.bo)
            try test(o2!.by == o.by)
            try test(o2!.sh == nil)
            try test(o2!.i == nil)
        }

        do {
            let arr = [true, false, true, false]
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(arr)
            var data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2: [Bool] = try inS.read()
            try test(arr2 == arr)

            let arrS = [arr, [], arr]
            outS = Ice.OutputStream(communicator: communicator)
            _BoolSSHelper.write(to: outS, value: arrS)
            data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2S: [[Bool]] = try _BoolSSHelper.read(from: inS)
            try test(arr2S == arrS)
        }

        do {
            let arr: [UInt8] = [0x01, 0x11, 0x12, 0x22]
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(arr)
            var data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2: [UInt8] = try inS.read()
            try test(arr2 == arr)

            let arrS = [arr, [], arr]
            outS = Ice.OutputStream(communicator: communicator)
            _ByteSSHelper.write(to: outS, value: arrS)
            data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2S = try _ByteSSHelper.read(from: inS)
            try test(arr2S == arrS)
        }

        do {
            let arr: [Int16] = [0x01, 0x11, 0x12, 0x22]
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(arr)
            var data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2: [Int16] = try inS.read()
            try test(arr2 == arr)

            let arrS = [arr, [], arr]
            outS = Ice.OutputStream(communicator: communicator)
            _ShortSSHelper.write(to: outS, value: arrS)
            data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2S = try _ShortSSHelper.read(from: inS)
            try test(arr2S == arrS)
        }

        do {
            let arr: [Int32] = [0x01, 0x11, 0x12, 0x22]
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(arr)
            var data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2: [Int32] = try inS.read()
            try test(arr2 == arr)

            let arrS = [arr, [], arr]
            outS = Ice.OutputStream(communicator: communicator)
            _IntSSHelper.write(to: outS, value: arrS)
            data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2S = try _IntSSHelper.read(from: inS)
            try test(arr2S == arrS)
        }

        do {
            let arr: [Int64] = [0x01, 0x11, 0x12, 0x22]
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(arr)
            var data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2: [Int64] = try inS.read()
            try test(arr2 == arr)

            let arrS = [arr, [], arr]
            outS = Ice.OutputStream(communicator: communicator)
            _LongSSHelper.write(to: outS, value: arrS)
            data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2S = try _LongSSHelper.read(from: inS)
            try test(arr2S == arrS)
        }

        do {
            let arr: [Float] = [1, 2, 3, 4]
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(arr)
            var data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2: [Float] = try inS.read()
            try test(arr2 == arr)

            let arrS = [arr, [], arr]
            outS = Ice.OutputStream(communicator: communicator)
            _FloatSSHelper.write(to: outS, value: arrS)
            data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2S = try _FloatSSHelper.read(from: inS)
            try test(arr2S == arrS)
        }

        do {
            let arr: [Double] = [1, 2, 3, 4]
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(arr)
            var data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2: [Double] = try inS.read()
            try test(arr2 == arr)

            let arrS = [arr, [], arr]
            outS = Ice.OutputStream(communicator: communicator)
            _DoubleSSHelper.write(to: outS, value: arrS)
            data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2S = try _DoubleSSHelper.read(from: inS)
            try test(arr2S == arrS)
        }

        do {
            let arr: [String] = ["string1", "string2", "string3", "string4"]
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(arr)
            var data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2: [String] = try inS.read()
            try test(arr2 == arr)
            let arrS = [arr, [], arr]
            outS = Ice.OutputStream(communicator: communicator)
            _StringSSHelper.write(to: outS, value: arrS)
            data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2S = try _StringSSHelper.read(from: inS)
            try test(arr2S == arrS)
        }

        do {
            let arr: [MyEnum] = [MyEnum.enum3, MyEnum.enum2, MyEnum.enum1, MyEnum.enum2]
            outS = Ice.OutputStream(communicator: communicator)
            _MyEnumSHelper.write(to: outS, value: arr)
            var data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2: [MyEnum] = try _MyEnumSHelper.read(from: inS)
            try test(arr2 == arr)
            let arrS = [arr, [], arr]
            outS = Ice.OutputStream(communicator: communicator)
            _MyEnumSSHelper.write(to: outS, value: arrS)
            data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2S = try _MyEnumSSHelper.read(from: inS)
            try test(arr2S == arrS)
        }

        var smallStructArray = [SmallStruct]()
        for i in 0..<3 {
            smallStructArray.append(SmallStruct())
            smallStructArray[i].bo = true
            smallStructArray[i].by = 1
            smallStructArray[i].sh = 2
            smallStructArray[i].i = 3
            smallStructArray[i].l = 4
            smallStructArray[i].f = 5.0
            smallStructArray[i].d = 6.0
            smallStructArray[i].str = "7"
            smallStructArray[i].e = MyEnum.enum2
            smallStructArray[i].p = uncheckedCast(prx: try communicator.stringToProxy(str: "test:default")!,
                                                  type: MyInterfacePrx.self)
        }

        var myClassArray = [MyClass]()
        for i in 0..<4 {
            myClassArray.append(MyClass())
            myClassArray[i].c = myClassArray[i]
            myClassArray[i].o = myClassArray[i]
            myClassArray[i].s = SmallStruct()
            myClassArray[i].s.e = MyEnum.enum2
            myClassArray[i].seq1 = [true, false, true, false]
            myClassArray[i].seq2 = [1, 2, 3, 4]
            myClassArray[i].seq3 = [1, 2, 3, 4]
            myClassArray[i].seq4 = [1, 2, 3, 4]
            myClassArray[i].seq5 = [1, 2, 3, 4]
            myClassArray[i].seq6 = [1, 2, 3, 4]
            myClassArray[i].seq7 = [1, 2, 3, 4]
            myClassArray[i].seq8 = ["string1", "string2", "string3", "string4"]
            myClassArray[i].seq9 = [MyEnum.enum3, MyEnum.enum2, MyEnum.enum1]
            myClassArray[i].seq10 = [nil, nil, nil, nil]
            myClassArray[i].d = ["hi": myClassArray[i]]
        }

        var myInterface = [Ice.Value]()
        for _ in 0..<4 {
            myInterface.append(Ice.InterfaceByValue(id: "::Test::MyInterface"))
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            _MyClassSHelper.write(to: outS, value: myClassArray)
            outS.writePendingValues()
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2: [MyClass?] = try _MyClassSHelper.read(from: inS)
            try inS.readPendingValues()
            try test(myClassArray.count == arr2.count)
            for i in 0..<myClassArray.count {
                try test(arr2[i] != nil)
                try test(arr2[i]!.c === arr2[i])
                try test(arr2[i]!.o === arr2[i])
                try test(arr2[i]!.s.e == MyEnum.enum2)
                try test(arr2[i]!.seq1 == myClassArray[i].seq1)
                try test(arr2[i]!.seq2 == myClassArray[i].seq2)
                try test(arr2[i]!.seq3 == myClassArray[i].seq3)
                try test(arr2[i]!.seq4 == myClassArray[i].seq4)
                try test(arr2[i]!.seq5 == myClassArray[i].seq5)
                try test(arr2[i]!.seq6 == myClassArray[i].seq6)
                try test(arr2[i]!.seq7 == myClassArray[i].seq7)
                try test(arr2[i]!.seq8 == myClassArray[i].seq8)
                try test(arr2[i]!.seq9 == myClassArray[i].seq9)
                try test(arr2[i]!.d["hi"]! === arr2[i])
            }
        }
        writer.writeLine("ok")
    }
}
