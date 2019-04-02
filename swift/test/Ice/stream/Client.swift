//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import Ice
import TestCommon

open class TestFactoryI: TestFactory {
    public class func create() -> TestHelper {
        return Client()
    }
}

public class Client: TestHelperI {
    public override func run(args: [String]) throws {
        var writer = getWriter()
        writer.write("testing primitive types... ")
        let (communicator, _) = try self.initialize(args: args)
        defer {
            communicator.destroy()
        }
        try communicator.getValueFactoryManager().add(
            factory: { Ice.InterfaceByValue(id: $0) }, id: "::Test::MyInterface")

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
            _ = try inS.startEncapsulation()
            var value: Bool = try inS.read()
            try test(value)

            inS = Ice.InputStream(communicator: communicator, bytes: data)
            _ = try inS.startEncapsulation()
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
            s.p = uncheckedCast(prx: try communicator.stringToProxy("test:default")!,
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
            try test(s2.p == s.p)
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
            try inS.read(OptionalClass.self) {
                o2 = $0
            }
            try inS.readPendingValues()
            try test(o2!.bo == o.bo)
            try test(o2!.by == o.by)
            if communicator.getProperties().getProperty("Ice.Default.EncodingVersion") == "1.0" {
                try test(o2!.sh == nil)
                try test(o2!.i == nil)
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
            try inS.read(OptionalClass.self) {
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
            BoolSSHelper.write(to: outS, value: arrS)
            data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2S: [[Bool]] = try BoolSSHelper.read(from: inS)
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
            ByteSSHelper.write(to: outS, value: arrS)
            data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2S = try ByteSSHelper.read(from: inS)
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
            ShortSSHelper.write(to: outS, value: arrS)
            data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2S = try ShortSSHelper.read(from: inS)
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
            IntSSHelper.write(to: outS, value: arrS)
            data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2S = try IntSSHelper.read(from: inS)
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
            LongSSHelper.write(to: outS, value: arrS)
            data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2S = try LongSSHelper.read(from: inS)
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
            FloatSSHelper.write(to: outS, value: arrS)
            data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2S = try FloatSSHelper.read(from: inS)
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
            DoubleSSHelper.write(to: outS, value: arrS)
            data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2S = try DoubleSSHelper.read(from: inS)
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
            StringSSHelper.write(to: outS, value: arrS)
            data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2S = try StringSSHelper.read(from: inS)
            try test(arr2S == arrS)
        }

        do {
            let arr: [MyEnum] = [MyEnum.enum3, MyEnum.enum2, MyEnum.enum1, MyEnum.enum2]
            outS = Ice.OutputStream(communicator: communicator)
            MyEnumSHelper.write(to: outS, value: arr)
            var data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2: [MyEnum] = try MyEnumSHelper.read(from: inS)
            try test(arr2 == arr)
            let arrS = [arr, [], arr]
            outS = Ice.OutputStream(communicator: communicator)
            MyEnumSSHelper.write(to: outS, value: arrS)
            data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2S = try MyEnumSSHelper.read(from: inS)
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
            smallStructArray[i].p = uncheckedCast(prx: try communicator.stringToProxy("test:default")!,
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

        var myInterfaceArray = [Ice.Value]()
        for _ in 0..<4 {
            myInterfaceArray.append(Ice.InterfaceByValue(id: "::Test::MyInterface"))
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            MyClassSHelper.write(to: outS, value: myClassArray)
            outS.writePendingValues()
            var data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2: [MyClass?] = try MyClassSHelper.read(from: inS)
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

            let arrS = [myClassArray, [], myClassArray]
            outS = Ice.OutputStream(communicator: communicator)
            MyClassSSHelper.write(to: outS, value: arrS)
            outS.writePendingValues()
            data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2S = try MyClassSSHelper.read(from: inS)
            try test(arr2S.count == arrS.count)
            try test(arr2S[0].count == arrS[0].count)
            try test(arr2S[1].count == arrS[1].count)
            try test(arr2S[2].count == arrS[2].count)
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            MyInterfaceSHelper.write(to: outS, value: myInterfaceArray)
            outS.writePendingValues()
            var data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2 = try MyInterfaceSHelper.read(from: inS)
            try inS.readPendingValues()
            try test(arr2.count == myInterfaceArray.count)

            let arrS = [myInterfaceArray, [], myInterfaceArray]
            outS = Ice.OutputStream(communicator: communicator)
            MyInterfaceSSHelper.write(to: outS, value: arrS)
            data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let arr2S = try MyInterfaceSSHelper.read(from: inS)
            try test(arr2S.count == arrS.count)
            try test(arr2S[0].count == arrS[0].count)
            try test(arr2S[1].count == arrS[1].count)
            try test(arr2S[2].count == arrS[2].count)
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            let ex = MyException()
            let c = MyClass()
            c.c = c
            c.o = c
            c.s.e = MyEnum.enum2
            c.seq1 = [true, false, true, false]
            c.seq2 = [1, 2, 3, 4]
            c.seq3 = [1, 2, 3, 4]
            c.seq4 = [1, 2, 3, 4]
            c.seq5 = [1, 2, 3, 4]
            c.seq6 = [1, 2, 3, 4]
            c.seq7 = [1, 2, 3, 4]
            c.seq8 = ["string1", "string2", "string3", "string4"]
            c.seq9 = [MyEnum.enum3, MyEnum.enum2, MyEnum.enum1]
            c.seq10 = [nil, nil, nil, nil]
            c.d = ["hi": c]

            ex.c = c

            outS.write(ex)
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            do {
                try inS.throwException()
            } catch let ex1 as MyException {
                try test(ex1.c!.s.e == c.s.e)
                try test(ex1.c!.seq1 == c.seq1)
                try test(ex1.c!.seq2 == c.seq2)
                try test(ex1.c!.seq3 == c.seq3)
                try test(ex1.c!.seq4 == c.seq4)
                try test(ex1.c!.seq5 == c.seq5)
                try test(ex1.c!.seq6 == c.seq6)
                try test(ex1.c!.seq7 == c.seq7)
                try test(ex1.c!.seq8 == c.seq8)
                try test(ex1.c!.seq9 == c.seq9)
            } catch is Ice.UserException {
                try test(false)
            }
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            let dict: ByteBoolD = [4: true, 1: false]
            ByteBoolDHelper.write(to: outS, value: dict)
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let dict2 = try ByteBoolDHelper.read(from: inS)
            try test(dict == dict2)
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            let dict: ShortIntD = [1: 9, 4: 8]
            ShortIntDHelper.write(to: outS, value: dict)
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let dict2 = try ShortIntDHelper.read(from: inS)
            try test(dict == dict2)
        }

        do {
            let dict: LongFloatD = [123809828: 0.5, 123809829: 0.6]
            outS = Ice.OutputStream(communicator: communicator)
            LongFloatDHelper.write(to: outS, value: dict)
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let dict2 = try LongFloatDHelper.read(from: inS)
            try test(dict == dict2)
        }

        do {
            let dict: StringStringD = ["key1": "value1", "key2": "value2"]
            outS = Ice.OutputStream(communicator: communicator)
            StringStringDHelper.write(to: outS, value: dict)
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let dict2 = try StringStringDHelper.read(from: inS)
            try test(dict2 == dict)
        }

        do {
            var dict = StringMyClassD()
            var c = MyClass()
            c.s = SmallStruct()
            c.s.e = MyEnum.enum2
            dict["key1"] = c
            c = MyClass()
            c.s = SmallStruct()
            c.s.e = MyEnum.enum3
            dict["key2"] = c
            outS = Ice.OutputStream(communicator: communicator)
            StringMyClassDHelper.write(to: outS, value: dict)
            outS.writePendingValues()
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let dict2: StringMyClassD = try StringMyClassDHelper.read(from: inS)
            try inS.readPendingValues()
            try test(dict2.count == dict.count)
            try test(dict2["key1"]!!.s.e == MyEnum.enum2)
            try test(dict2["key2"]!!.s.e == MyEnum.enum3)
        }
        writer.writeLine("ok")
    }
}
