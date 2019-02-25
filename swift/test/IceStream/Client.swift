//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import Ice
import TestCommon

public class Client: TestHelperI {
    public override func run(args _: [String]) throws {
        var writer = getWriter()
        writer.write(data: "testing primitive types... ")
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
            try test(value: try Bool(from: inS))

            inS = Ice.InputStream(communicator: communicator, bytes: data)
            try inS.startEncapsulation()
            try test(value: try Bool(from: inS))
        }

        do {
            inS = Ice.InputStream(communicator: communicator, bytes: [UInt8]())
            do {
                _ = try Bool(from: inS)
                try test(value: false)
            } catch {}
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(true)
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            try test(value: try Bool(from: inS))
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(UInt8(1))
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            try test(value: try UInt8(from: inS) == 1)
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(Int16(2))
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            try test(value: try Int16(from: inS) == 2)
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(Int32(3))
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            try test(value: try Int32(from: inS) == 3)
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(Int64(4))
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            try test(value: try Int64(from: inS) == 4)
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(Float(5.0))
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            try test(value: try Float(from: inS) == 5.0)
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(Double(6.0))
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            try test(value: try Double(from: inS) == 6.0)
        }

        do {
            outS = Ice.OutputStream(communicator: communicator)
            outS.write(utf8view: "hello world".utf8)
            let data = outS.finished()
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            try test(value: inS.readString() == "hello world")
        }

        writer.writeLine(data: "ok")

        writer.write(data: "testing constructed types... ")
        outS = Ice.OutputStream(communicator: communicator)
        writer.writeLine(data: "ok")
    }
}
