//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import Foundation
import Ice
import TestCommon

func main() throws {
    fputs("testing stringToProxy... ", stdout)
    do {
        var communicator = try Ice.initialize()
        defer {
            communicator.destroy()
        }

        //
        // Test nil proxies.
        //
        do {
            var p = try communicator.stringToProxy(str: "")
            assert(p == nil)
            p = try communicator.stringToProxy(str: "bogus")
            assert(p == nil)
        }
    } catch let err {
        print(err)
    }
    print("ok")
}

do {
    try main()
} catch let err {
    print(err)
}
