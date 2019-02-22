//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import Foundation

func main() throws {
    fputs("testing primitive types... ", stdout)
    do
    {
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
            var data = outS.finished()
            
            inS = Ice.InputStream(communicator: communicator, bytes: data)
            try inS.startEncapsulation()
            assert(inS.read(as: Bool.self))
            
            
            
        }
    }
    catch let err
    {
        print(err)
    }
    print("ok")
}
