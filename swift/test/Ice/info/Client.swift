// Copyright (c) ZeroC, Inc.

import Ice
import PromiseKit
import TestCommon

public class Client: TestHelperI {
    override public func run(args: [String]) throws {
        let communicator = try initialize(args)
        defer {
            communicator.destroy()
        }
        try allTests(self)
    }
}
