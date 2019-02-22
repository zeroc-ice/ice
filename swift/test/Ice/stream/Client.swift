//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import Foundation

public class Client : TestHelperI {

    public override func run(args: [String]) throws {

        var writer = getWriter()
        writer.write(data: "testing primitive types... ")
        var communicator = try Ice.initialize()
        defer {
            communicator.destroy()
        }

        var inS:Ice.InputStream
        var outS:Ice.OutputStream

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
            let b = try Bool(from: inS)
            try test(value: b)
            
            try test(value: false)
        }
        writer.writeLine(data: "ok")
    }
}
