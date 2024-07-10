// Copyright (c) ZeroC, Inc.

import Ice
import TestCommon

public class Client: TestHelperI {
    override public func run(args: [String]) throws {
        let communicator = try initialize(args)
        defer {
            communicator.destroy()
        }

        // try communicator.getClientDispatchQueue().async {
        //     print("Hello from client dispatch queue")
        // }
    }
}
