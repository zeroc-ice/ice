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
            var s = SmallStruct();
            s.bo = true
            s.by = 1
            s.sh = 2
            s.i = 3
            s.l = 4
            s.f = 5.0
            s.d = 6.0
            s.str = "7"
            s.e = MyEnum.enum2;
            s.p = nil
            //s.p = MyInterfacePrx.uncheckedCast(communicator.stringToProxy("test:default"))
            outS.write(s)
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            let s2: SmallStruct = try inS.read()
            //test(s2 == s)
        }
        writer.writeLine("ok")
    }
}
