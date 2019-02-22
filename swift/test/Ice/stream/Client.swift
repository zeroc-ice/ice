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

        do {
            var data = [UInt8]()
            var inS = Ice.InputStream(communicator, data)
        }

        var outS = Ice.OutputStream(communicator)
        outS.startEncapsulation()
        outS.write(true)
        outS.endEncapsulation()
    }
    catch let err
    {
        print(err)
    }
    print("ok")
}

do
{
    try main()
}
catch let err
{
    print(err)
}
