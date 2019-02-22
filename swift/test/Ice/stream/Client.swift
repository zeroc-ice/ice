//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Ice
import TestCommon
import Foundation

public class Client : TestHelperI {

    public override func run(args: [String]) throws {

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
                let data = outS.finished()
                
                inS = Ice.InputStream(communicator: communicator, bytes: data)
                try inS.startEncapsulation()
                let b = try Bool(from: inS)
                assert(b)
            }
        }
        catch let err
        {
            print(err)
        }
        print("ok")
    }
}
